#pragma once

#include <vector>
#include <string>
#include <map>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "PSRAM_Allocator.h"
#ifndef IMAGECACHE_BG_PRIORITY
  #define IMAGECACHE_BG_PRIORITY 5
#endif

// The state of the caching process
enum class CacheStatus {
  IDLE,
  PRELOADING_BG,
  LOADING_DEMAND
};

// Stores image data and its modification time for smart sync
typedef struct {
  uint8_t* buffer;
  size_t size;
  time_t mtime;
} ImageData;

// Define C++ types that use the PSRAM allocator
using psram_string = std::basic_string<char, std::char_traits<char>, PSRAM_Allocator<char>>;
using psram_file_map = std::map<psram_string, ImageData, std::less<psram_string>,PSRAM_Allocator<std::pair<const psram_string, ImageData>>>;
using psram_image_map = std::map<psram_string, psram_file_map, std::less<psram_string>,PSRAM_Allocator<std::pair<const psram_string, psram_file_map>>>;

class ImageCacheManager {
  public:
    static ImageCacheManager& getInstance();

    void startPreload(const std::string& root_path);
    ImageData* getImage(const std::string& folder_path, size_t index);
    size_t getFolderSize(const std::string& folder_path);
    void clearCache();

    // "Peek" functions to get the current status
    CacheStatus getStatus();
    psram_string getCurrentFile();
    size_t getCacheUsedBytes();

  private:
    ImageCacheManager();
    ~ImageCacheManager();
    ImageCacheManager(const ImageCacheManager&) = delete;
    void operator=(const ImageCacheManager&) = delete;

    static void _preloadTask(void* params);
    void _synchronizeFolder(const psram_string& folder_path, bool is_on_demand);

    // Main cache is a PSRAM-based map of maps
    psram_image_map image_cache;
    // Helper to get image by index since map is not indexable
    ImageData* _getImageByIndex(const psram_string& folder_path, size_t index);

    // Synchronization and state
    SemaphoreHandle_t cache_mutex;
    SemaphoreHandle_t loader_mutex;
    TaskHandle_t preload_task_handle;
    volatile CacheStatus current_status;
    psram_string current_loading_file;

    // Resource management
    size_t psram_limit;
    size_t psram_used;
    psram_string preload_root_path;
};