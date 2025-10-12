#include "ImageCacheManager.h"
#include "esp_psram.h"
#include "esp_log.h"
#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>
#include <fstream>
#include <set>

static const char* TAG = "ImageCache";

ImageCacheManager& ImageCacheManager::getInstance() {
  static ImageCacheManager instance;
  return instance;
}

ImageCacheManager::ImageCacheManager() :
  preload_task_handle(NULL),
  current_status(CacheStatus::IDLE),
  psram_limit(0),
  psram_used(0) {
  cache_mutex = xSemaphoreCreateMutex();
  loader_mutex = xSemaphoreCreateMutex();
  size_t total_psram = esp_psram_get_size();
  psram_limit = static_cast<size_t>(total_psram * 0.8);
  ESP_LOGI(TAG, "Total PSRAM: %u bytes, Cache Limit (80%%): %u bytes", total_psram, psram_limit);
}

ImageCacheManager::~ImageCacheManager() {
  clearCache();
  vSemaphoreDelete(cache_mutex);
}

void ImageCacheManager::startPreload(const std::string& root_path) {
  if (preload_task_handle != NULL) {
    ESP_LOGW(TAG, "Preload task is already running.");
    return;
  }
  // Don't clear cache here to allow for intelligent sync
  preload_root_path = root_path.c_str();
  xTaskCreatePinnedToCore(_preloadTask, "preload_task", 4096, this, IMAGECACHE_BG_PRIORITY, &preload_task_handle, 0); // core 0, where FFT lives
}

ImageData* ImageCacheManager::getImage(const std::string& folder_path, size_t index) {
  psram_string ps_folder_path = folder_path.c_str();

  xSemaphoreTake(cache_mutex, portMAX_DELAY);
  ImageData* data = _getImageByIndex(ps_folder_path, index);
  xSemaphoreGive(cache_mutex);

  if (data) {
    return data;
  }

  ESP_LOGI(TAG, "On-demand loading folder: %s", folder_path.c_str());
  _synchronizeFolder(ps_folder_path, true);

  xSemaphoreTake(cache_mutex, portMAX_DELAY);
  data = _getImageByIndex(ps_folder_path, index);
  xSemaphoreGive(cache_mutex);

  return data;
}

size_t ImageCacheManager::getFolderSize(const std::string& folder_path) {
  size_t size = 0;
  psram_string ps_folder_path = folder_path.c_str();
  xSemaphoreTake(cache_mutex, portMAX_DELAY);
  auto it = image_cache.find(ps_folder_path);
  if (it != image_cache.end()) {
    size = it->second.size();
  }
  xSemaphoreGive(cache_mutex);
  return size;
}

CacheStatus ImageCacheManager::getStatus() {
  return current_status;
}

psram_string ImageCacheManager::getCurrentFile() {
  psram_string file;
  xSemaphoreTake(cache_mutex, portMAX_DELAY);
  file = current_loading_file;
  xSemaphoreGive(cache_mutex);
  return file;
}

size_t ImageCacheManager::getCacheUsedBytes() {
  size_t used_bytes = 0;
  xSemaphoreTake(cache_mutex, portMAX_DELAY);
  used_bytes = psram_used;
  xSemaphoreGive(cache_mutex);
  return used_bytes;
}

void ImageCacheManager::_preloadTask(void* params) {
  ImageCacheManager* manager = static_cast<ImageCacheManager*>(params);
  manager->current_status = CacheStatus::PRELOADING_BG;

  DIR* dir = opendir(manager->preload_root_path.c_str());
  if (!dir) {
    ESP_LOGE(TAG, "Failed to open root directory: %s", manager->preload_root_path.c_str());
    manager->preload_task_handle = NULL;
    vTaskDelete(NULL);
    return;
  }

  using psram_string_vector = std::vector<psram_string, PSRAM_Allocator<psram_string>>;
  psram_string_vector hot_folders;
  psram_string_vector cold_folders;

  struct dirent* entry;
  while ((entry = readdir(dir)) != nullptr) {
    psram_string name = entry->d_name;
    if (entry->d_type == DT_DIR && name.rfind("sequence", 0) == 0) {
      if (name.length() > 4 && name.substr(name.length() - 4) == "_hot") {
        hot_folders.push_back(name);
      }
      else {
        cold_folders.push_back(name);
      }
    }
  }
  closedir(dir);

  std::sort(hot_folders.begin(), hot_folders.end());
  std::sort(cold_folders.begin(), cold_folders.end());

  ESP_LOGI(TAG, "--- Processing %d high-priority (_hot) folders ---", hot_folders.size());
  for (const auto& folder_name : hot_folders) {
    psram_string full_path = manager->preload_root_path + "/" + folder_name;
    manager->_synchronizeFolder(full_path, false);
  }

  ESP_LOGI(TAG, "--- Processing %d standard-priority folders ---", cold_folders.size());
  for (const auto& folder_name : cold_folders) {
    psram_string full_path = manager->preload_root_path + "/" + folder_name;
    manager->_synchronizeFolder(full_path, false);
  }

  manager->current_status = CacheStatus::IDLE;
  xSemaphoreTake(manager->cache_mutex, portMAX_DELAY);
  manager->current_loading_file = "";
  xSemaphoreGive(manager->cache_mutex);

  ESP_LOGI(TAG, "Background synchronization finished.");
  manager->preload_task_handle = NULL;
  vTaskDelete(NULL);
}

void ImageCacheManager::_synchronizeFolder(const psram_string& folder_path, bool is_on_demand) {

  xSemaphoreTake(loader_mutex, portMAX_DELAY);

  CacheStatus old_status = current_status; // Save the current state
  if (is_on_demand) {
    current_status = CacheStatus::LOADING_DEMAND;
  }

  if (xTaskGetCurrentTaskHandle() != preload_task_handle) {
    current_status = CacheStatus::LOADING_DEMAND;
  }

  if (is_on_demand) {
    current_status = CacheStatus::LOADING_DEMAND;
  }

  DIR* dir = opendir(folder_path.c_str());
  if (!dir) {
    xSemaphoreGive(loader_mutex);
    return;
  }

  std::map<psram_string, time_t, std::less<psram_string>, PSRAM_Allocator<std::pair<const psram_string, time_t>>> files_on_disk;
  struct dirent* entry;
  struct stat st;
  while ((entry = readdir(dir)) != nullptr) {
    psram_string name = entry->d_name;
    if (name.find(".jpg") != psram_string::npos || name.find(".jpeg") != psram_string::npos) {
      psram_string full_path = folder_path + "/" + name;
      if (stat(full_path.c_str(), &st) == 0) {
        files_on_disk[name] = st.st_mtime;
      }
    }
  }
  closedir(dir);

  xSemaphoreTake(cache_mutex, portMAX_DELAY);

  psram_file_map& cached_files = image_cache[folder_path];

  // Remove files from cache that are no longer on disk
  for (auto it = cached_files.begin(); it != cached_files.end(); ) {
    if (files_on_disk.find(it->first) == files_on_disk.end()) {
      ESP_LOGI(TAG, "Removing deleted file: %s", it->first.c_str());
      psram_used -= it->second.size;
      free(it->second.buffer);
      it = cached_files.erase(it);
    }
    else {
      ++it;
    }
  }

  for (const auto& disk_file_pair : files_on_disk) {
    const psram_string& filename = disk_file_pair.first;
    const time_t& disk_mtime = disk_file_pair.second;
    current_loading_file = folder_path + "/" + filename;

    auto cache_it = cached_files.find(filename);
    bool needs_load = false;
    if (cache_it == cached_files.end()) { needs_load = true; }
    else if (disk_mtime > cache_it->second.mtime) {
      ESP_LOGI(TAG, "Updating modified file: %s", filename.c_str());
      psram_used -= cache_it->second.size;
      free(cache_it->second.buffer);
      cached_files.erase(cache_it);
      needs_load = true;
    }

    if (needs_load) {

      xSemaphoreGive(cache_mutex);

      psram_string full_path = folder_path + "/" + filename;
      stat(full_path.c_str(), &st);
      size_t size = st.st_size;

      if (psram_used + size > psram_limit) {
        ESP_LOGW(TAG, "PSRAM limit reached, cannot load %s", filename.c_str());
        xSemaphoreTake(cache_mutex, portMAX_DELAY);
        continue;
      }

      FILE* file = fopen(full_path.c_str(), "rb");
      uint8_t* buffer = nullptr;
      if (file) {
        buffer = (uint8_t*)heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_CACHE_ALIGNED);
        if (buffer) {
          fread(buffer, 1, size, file);
        }
        fclose(file);
      }

      xSemaphoreTake(cache_mutex, portMAX_DELAY);
      if (buffer) {
        cached_files[filename] = { buffer, size, disk_mtime };
        psram_used += size;
        ESP_LOGI(TAG, "Loaded new/updated file: %s", filename.c_str());
      }
      else {
        ESP_LOGE(TAG, "Failed to load file: %s", filename.c_str());
      }
    }
  }

  xSemaphoreGive(cache_mutex);

  if (is_on_demand) {
    current_status = old_status;
    xSemaphoreTake(cache_mutex, portMAX_DELAY);
    current_loading_file = "";
    xSemaphoreGive(cache_mutex);
  }

  xSemaphoreGive(loader_mutex);

}
  

ImageData* ImageCacheManager::_getImageByIndex(const psram_string& folder_path, size_t index) {
  // This helper must be called from within a mutex lock
  auto it = image_cache.find(folder_path);
  if (it != image_cache.end()) {
    if (index < it->second.size()) {
      // std::map iterators are not random access, so we advance
      auto map_it = it->second.begin();
      std::advance(map_it, index);
      return &map_it->second;
    }
  }
  return nullptr;
}

void ImageCacheManager::clearCache() {
  xSemaphoreTake(cache_mutex, portMAX_DELAY);
  for (auto& pair : image_cache) {
    for (auto& file_pair : pair.second) {
      free(file_pair.second.buffer);
    }
  }
  image_cache.clear();
  psram_used = 0;
  xSemaphoreGive(cache_mutex);
  ESP_LOGI(TAG, "Image cache cleared.");
}