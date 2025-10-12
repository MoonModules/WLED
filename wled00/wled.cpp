#define WLED_DEFINE_GLOBAL_VARS //only in one source file, wled.cpp!
static const char *TAG = "WLED";
#include "wled.h"
#include "wled_ethernet.h"
#include <Arduino.h>
#if defined(CONFIG_IDF_TARGET_ESP32P4)
  #include "esp_ldo_regulator.h" // ESP32-P4 for higher GPIOS.
  esp_ldo_channel_handle_t ldo2 = NULL;
  esp_ldo_channel_handle_t ldo3 = NULL;
  // ESP32-P4 Board Log
  // ESP-ROM:esp32p4-eco2-20240710 // WaveShare Nano
  // ESP-ROM:esp32p4-eco2-20240710 // WaveShare P4 Module Dev Kit (4 USB-A ports, custom module)
  // ESP-ROM:esp32p4-eco2-20240710 // WaveShare Big Round Display Thingy
  // ESP-ROM:esp32p4-eco2-20240710 // WaveShare ESP32-P4-86-Panel-ETH-PRO
  // ESP-ROM:esp32p4-eco1-20240205 // Espressif EV
  // ESP-ROM:esp32p4-eco2-20240710 // Wireless Tag Fancy C5 board that's weird.
  // 
#endif
#ifdef SOC_USB_OTG_SUPPORTED
  #ifndef CONFIG_USB_HOST_HW_BUFFER_BIAS_BALANCED
    #error "USB Hardware Buffer Bias must be set to 'Balanced' via USB-OTG or CONFIG_USB_HOST_HW_BUFFER_BIAS_BALANCED=y."
    // This is likely to be fixed later. 
    // You could also comment out the #error and try (untested):
    #undef CONFIG_USB_HOST_HW_BUFFER_BIAS_IN
    #undef CONFIG_USB_HOST_HW_BUFFER_BIAS_PERIODIC_OUT
    #define CONFIG_USB_HOST_HW_BUFFER_BIAS_BALANCED 1
  #endif
  #include <dirent.h>
  #include "usb/usb_host.h"
  #include "usb/msc_host_vfs.h"
  #include "ImageCacheManager.h"
  
  #define MNT_PATH "/usb"     // Base mount path prefix, devices will be mounted as /usb0, /usb1, /usb2...
  #define MAX_MSC_DEVICES  CONFIG_FATFS_VOLUME_COUNT 

  typedef struct {
    uint8_t usb_addr;                     /*!< USB device address */
    msc_host_device_handle_t msc_device;  /*!< Handle of the MSC device */
    msc_host_vfs_handle_t vfs_handle;     /*!< VFS handle assigned to the MSC device */
  } msc_dev_entry_t;

  static msc_dev_entry_t *msc_devices[MAX_MSC_DEVICES] = {0};

  static QueueHandle_t app_queue;

  typedef struct {
    enum {
      APP_QUIT,                // Signals request to exit the application
      APP_DEVICE_CONNECTED,    // USB device connect event
      APP_DEVICE_DISCONNECTED, // USB device disconnect event
    } id;
    union {
      uint8_t new_dev_address; // Address of new USB device for APP_DEVICE_CONNECTED event
      msc_host_device_handle_t device_handle; // Handle of removed USB device for APP_DEVICE_DISCONNECTED event
    } data;
  } app_message_t;

  static inline int find_free_slot(void)
  {
    for (int i = 0; i < MAX_MSC_DEVICES; i++) {
      if (msc_devices[i] == NULL) {
        return i;
      }
    }
    return -1;
  }

  static esp_err_t allocate_new_msc_device(const app_message_t *msg, int *out_slot)
  {
      int slot = find_free_slot();
      if (slot < 0) {
          ESP_LOGE(TAG, "No free slots for new MSC device (max %d)", MAX_MSC_DEVICES);
          return ESP_ERR_NOT_FOUND;
      }

      msc_devices[slot] = (msc_dev_entry_t *)calloc(1, sizeof(msc_dev_entry_t));

      if (!msc_devices[slot]) {
          ESP_LOGE(TAG, "Failed to allocate memory for new MSC device entry");
          return ESP_ERR_NO_MEM;
      }
      esp_err_t err = msc_host_install_device(msg->data.new_dev_address, &msc_devices[slot]->msc_device);
      if (err != ESP_OK) {
          ESP_LOGE(TAG, "msc_host_install_device failed: %s", esp_err_to_name(err));
          free(msc_devices[slot]);
          msc_devices[slot] = NULL;
          return err;
      }

      msc_devices[slot]->usb_addr = msg->data.new_dev_address;

      const esp_vfs_fat_mount_config_t mount_config = {
          .format_if_mount_failed = false,
          .max_files = 3,
          .allocation_unit_size = 8192,
      };

      char mount_path[16];
      snprintf(mount_path, sizeof(mount_path), MNT_PATH "%d", slot);

      err = msc_host_vfs_register(msc_devices[slot]->msc_device, mount_path, &mount_config, &msc_devices[slot]->vfs_handle);
      if (err != ESP_OK) {
        ESP_LOGE(TAG, "msc_host_vfs_register failed: %s", esp_err_to_name(err));
        // Just call the uninstall function. Don't check the error in a way that aborts.
        // You can log its return value if you want, but the program must continue.
        esp_err_t uninstall_err = msc_host_uninstall_device(msc_devices[slot]->msc_device);
        if (uninstall_err != ESP_OK) {
          ESP_LOGW(TAG, "msc_host_uninstall_device failed during cleanup: %s", esp_err_to_name(uninstall_err));
        }
        free(msc_devices[slot]);
        msc_devices[slot] = NULL;
        // Return the original error that started this cleanup process
        return err;
      }

      *out_slot = slot;
      return ESP_OK;
  }

  static int find_slot_by_handle(msc_host_device_handle_t handle)
  {
      for (int i = 0; i < MAX_MSC_DEVICES; i++) {
          if (msc_devices[i] && msc_devices[i]->msc_device == handle) {
              return i;
          }
      }
      return -1;
  }

  static void free_msc_device(int slot)
  {
      if (slot < 0 || slot >= MAX_MSC_DEVICES || !msc_devices[slot]) {
          ESP_LOGE(TAG, "Invalid slot index for MSC device deallocation");
          return;
      }

      if (msc_devices[slot]->vfs_handle) {
          ESP_ERROR_CHECK(msc_host_vfs_unregister(msc_devices[slot]->vfs_handle));
      }
      if (msc_devices[slot]->msc_device) {
          ESP_ERROR_CHECK(msc_host_uninstall_device(msc_devices[slot]->msc_device));
      }

      free(msc_devices[slot]);
      msc_devices[slot] = NULL;
  }

  static void free_all_msc_devices(void)
  {
      for (int i = 0; i < MAX_MSC_DEVICES; i++) {
          if (msc_devices[i]) {
              free_msc_device(i);
          }
      }
  }

  static void gpio_cb(void *arg)
  {
      BaseType_t xTaskWoken = pdFALSE;
      app_message_t message = {
          .id = app_message_t::APP_QUIT,
      };

      if (app_queue) {
          xQueueSendFromISR(app_queue, &message, &xTaskWoken);
      }

      if (xTaskWoken == pdTRUE) {
          portYIELD_FROM_ISR();
      }
  }

  static inline int8_t find_usb_addr_by_handle(msc_host_device_handle_t handle)
  {
      for (int8_t i = 0; i < MAX_MSC_DEVICES; i++) {
          if (msc_devices[i] && msc_devices[i]->msc_device == handle) {
              return msc_devices[i]->usb_addr;
          }
      }
      return -1;
  }

  static void msc_event_cb(const msc_host_event_t *event, void *arg)
  {
      if (event->event == event->MSC_DEVICE_CONNECTED) {
          DEBUG_PRINTF("MSC device connected (usb_addr=%d)\n", event->device.address);
          app_message_t message = {};
          message.id = app_message_t::APP_DEVICE_CONNECTED;
          message.data.new_dev_address = event->device.address;

          xQueueSend(app_queue, &message, portMAX_DELAY);
      } else if (event->event == event->MSC_DEVICE_DISCONNECTED) {
          int usb_addr = find_usb_addr_by_handle(event->device.handle);
          if (usb_addr >= 0) {
              DEBUG_PRINTF("MSC device disconnected (usb_addr=%d)\n", usb_addr);
          } else {
              DEBUG_PRINTLN("MSC device disconnected, but failed to retrieve USB address");
          }
          app_message_t message = {};
          message.id = app_message_t::APP_DEVICE_DISCONNECTED;
          message.data.device_handle = event->device.handle;
          xQueueSend(app_queue, &message, portMAX_DELAY);
      }
  }

  static void print_device_info(msc_host_device_info_t *info)
  {
      const size_t megabyte = 1024 * 1024;
      uint64_t capacity = ((uint64_t)info->sector_size * info->sector_count) / megabyte;
      USER_PRINTF("USB Disk Capacity: %llu MB\n", capacity);
      // ESP_LOGE(TAG, "\t Sector size: %" PRIu32, info->sector_size);
      // ESP_LOGE(TAG, "\t Sector count: %" PRIu32, info->sector_count);
      // ESP_LOGE(TAG, "\t PID: 0x%04X", info->idProduct);
      // ESP_LOGE(TAG, "\t VID: 0x%04X", info->idVendor);
  }

  static void usb_task(void *args)
  {
      usb_host_config_t host_config = {};
      host_config.intr_flags = ESP_INTR_FLAG_LEVEL1;
      host_config.peripheral_map = BIT(0); // <--- this may be a bug of the current IDFv5.5 with USB High-Speed devices.

      // Bias Mode	  nptx_fifo_lines	  ptx_fifo_lines	rx_fifo_lines
      // Balanced	    256	              128	            512 (896 - 256 - 128)
      // IN-Biased	  64	              128	            704 (896 - 64 - 128) <--- does not work, but 896 seenms to be the correct total, confirmed by reading out the register.

      // Balanced values work: (works! and seem to match the IDF built with balanced defaults)
      // host_config.fifo_settings_custom.nptx_fifo_lines = 256;
      // host_config.fifo_settings_custom.ptx_fifo_lines = 128;
      // host_config.fifo_settings_custom.rx_fifo_lines = 512;

      // Testing a mid point: (marginal winner!) - THIS MAY BE FLAKEY? 
      // (tried a bunch off other ones too, this was the best - they were worse than balanced)
      //
      // host_config.fifo_settings_custom.nptx_fifo_lines = 128;
      // host_config.fifo_settings_custom.ptx_fifo_lines = 128;
      // host_config.fifo_settings_custom.rx_fifo_lines = 640;

      // If you need to know your on-SOC FIFO numbers this is the code - just for checking the register on the P4.
      // (The answer is 896, at least on all the current P4 devices I have.)
      //
      // periph_module_enable(PERIPH_UHCI_MODULE);
      // uint16_t fifo_depth_value = USB_DWC_HS.ghwcfg3_reg.dfifodepth;
      // USER_PRINTF("*** Extracted dfifodepth (fifo_size_lines): %u\n", fifo_depth_value);
      // periph_module_disable(PERIPH_UHCI_MODULE);

      ESP_ERROR_CHECK(usb_host_install(&host_config));

      const msc_host_driver_config_t msc_config = {
          .create_backround_task = true,
          .task_priority = 1, // xtaskcreate TroyHacks for finding later.
          .stack_size = 4096,
          .core_id = 0,
          .callback = msc_event_cb,
      };
      ESP_ERROR_CHECK(msc_host_install(&msc_config));

      bool has_clients = true;
      while (true) {
          uint32_t event_flags;
          usb_host_lib_handle_events(portMAX_DELAY, &event_flags);

          // Release devices once all clients has deregistered
          if (event_flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS) {
              has_clients = false;
              if (usb_host_device_free_all() == ESP_OK) {
                  break;
              };
          }
          if (event_flags & USB_HOST_LIB_EVENT_FLAGS_ALL_FREE && !has_clients) {
              break;
          }
      }

      vTaskDelay(10); // Give clients some time to uninstall
      USER_PRINTLN("Deinitializing USB (This shouldn't happen)");
      ESP_ERROR_CHECK(usb_host_uninstall());
      vTaskDelete(NULL);
  }

  static inline void show_list_files_all_devices(void)
  {
      // USER_PRINTF("ls command output for all connected devices:\n");
      for (int i = 0; i < MAX_MSC_DEVICES; i++) {
          if (msc_devices[i]) {
              char mount_path[16];
              snprintf(mount_path, sizeof(mount_path), MNT_PATH "%d", i);

              USER_PRINTF("Listing contents of %s:\n", mount_path);
              struct dirent *d;
              DIR *dh = opendir(mount_path);
              if (!dh) {
                  USER_PRINTF("Failed to open directory: %s", mount_path);
                  continue;
              }

              while ((d = readdir(dh)) != NULL) {
                  USER_PRINTF("%s/%s\n", mount_path, d->d_name);
              }
              closedir(dh);
          }
      }
  }

  #define APP_QUEUE_SIZE 5

#endif // SOC_USB_OTG_SUPPORTED

#ifdef ARDUINO_ARCH_ESP32
  #include "esp_ota_ops.h"
#endif
#warning WLED-MM is licensed under the EUPL-1.2. By installing WLED MM you implicitly accept the terms!

#if defined(ARDUINO_ARCH_ESP32) && defined(WLED_DISABLE_BROWNOUT_DET)
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#endif

#if defined(WLED_DEBUG) && defined(ARDUINO_ARCH_ESP32) // && !defined(CONFIG_IDF_TARGET_ESP32C6) && !defined(CONFIG_IDF_TARGET_ESP32P4)
#include "../tools/ESP32-Chip_info.hpp"
#endif

// #if defined(CONFIG_IDF_TARGET_ESP32P4)
// #include <WiFiGeneric.h>
// #endif

// WLEDMM some buildenv sanity checks

#ifdef ARDUINO_ARCH_ESP32 // ESP32
  #if !defined(ESP32)
    #error please fix your build environment. ESP32 is not defined.
  #endif
  #if defined(ESP8266) || defined(ARDUINO_ARCH_ESP8266)
    #error please fix your build environment. ESP32 and ESP8266 are both defined.
  #endif
  // only one of ARDUINO_ARCH_ESP32S2, ARDUINO_ARCH_ESP32S3, ARDUINO_ARCH_ESP32C3 allowed
  #if defined(ARDUINO_ARCH_ESP32S3) && ( defined(ARDUINO_ARCH_ESP32S2) || defined(ARDUINO_ARCH_ESP32C3) )
    #error please fix your build environment. only one of ARDUINO_ARCH_ESP32S3, ARDUINO_ARCH_ESP32S2, ARDUINO_ARCH_ESP32C3 may be defined
  #endif
  #if defined(ARDUINO_ARCH_ESP32S2) && ( defined(ARDUINO_ARCH_ESP32S3) || defined(ARDUINO_ARCH_ESP32C3) )
    #error please fix your build environment. only one of ARDUINO_ARCH_ESP32S3, ARDUINO_ARCH_ESP32S2, ARDUINO_ARCH_ESP32C3 may be defined
  #endif
  #if defined(CONFIG_IDF_TARGET_ESP32) && ( defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32C3))
    #error please fix your build environment. only one CONFIG_IDF_TARGET may be defined
  #endif
  // make sure we have a supported CONFIG_IDF_TARGET_
  #if !defined(CONFIG_IDF_TARGET_ESP32) && !defined(CONFIG_IDF_TARGET_ESP32S3) && !defined(CONFIG_IDF_TARGET_ESP32S2) && !defined(CONFIG_IDF_TARGET_ESP32C3)  && !defined(CONFIG_IDF_TARGET_ESP32C6) && !defined(CONFIG_IDF_TARGET_ESP32P4)
    #error please fix your build environment. No supported CONFIG_IDF_TARGET was defined
  #endif
  #if CONFIG_IDF_TARGET_ESP32_SOLO || CONFIG_IDF_TARGET_ESP32SOLO
    #warning ESP32 SOLO (single core) is not supported.
  #endif
  // only one of CONFIG_IDF_TARGET_ESP32, CONFIG_IDF_TARGET_ESP32S2, CONFIG_IDF_TARGET_ESP32S3, CONFIG_IDF_TARGET_ESP32C3 is allowed
  #if defined(CONFIG_IDF_TARGET_ESP32) && ( defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32C3))
    #error please fix your build environment. only one CONFIG_IDF_TARGET may be defined
  #endif
  #if defined(CONFIG_IDF_TARGET_ESP32S3) && ( defined(CONFIG_IDF_TARGET_ESP32) || defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32C3))
    #error please fix your build environment. only one CONFIG_IDF_TARGET may be defined
  #endif
  #if defined(CONFIG_IDF_TARGET_ESP32C3) && ( defined(CONFIG_IDF_TARGET_ESP32) || defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32S2))
    #error please fix your build environment. only one CONFIG_IDF_TARGET may be defined
  #endif

#else // 8266
  #if !defined(ARDUINO_ARCH_ESP8266) && !defined(ARDUINO_ARCH_ESP8265)
    #error please fix your build environment. Neither ARDUINO_ARCH_ESP8266 nor ARDUINO_ARCH_ESP32 are defined
  #else
    #if !defined(ESP8266) && !defined(ESP8265)
      #error please fix your build environment. ESP8266 is not defined.
    #endif
  #endif
#endif
// WLEDMM end


#if INCLUDE_xTaskGetHandle && defined(ARDUINO_ARCH_ESP32) && (defined(WLED_DEBUG) || defined(WLED_DEBUG_HEAP))
// WLEDMM stack debug tool - find async_tcp task, and queries it's free stack
static int wledmm_get_tcp_stacksize(void) {
  static TaskHandle_t tcp_taskHandle = NULL;                   // to store the task handle for later calls
  char * tcp_taskname = pcTaskGetTaskName(tcp_taskHandle);     // ask for name of the known task (to make sure we are still looking at the right one)

  if ((tcp_taskHandle == NULL) || (tcp_taskname == NULL) || (strncmp(tcp_taskname, "async_tcp", 9) != 0)) {
    tcp_taskHandle = xTaskGetHandle("async_tcp");              // need to look for the task by name. FreeRTOS docs say this is very slow, so we store the result for next time
    //DEBUG_PRINT(F("async_tcp task ")); DEBUG_PRINTLN( (tcp_taskHandle != NULL) ? F("found") : F("not found"));
  }

  if (tcp_taskHandle != NULL) return uxTaskGetStackHighWaterMark(tcp_taskHandle); // got it !!
  else return -1;
}
#endif

/*
 * Main WLED class implementation. Mostly initialization and connection logic
 */

WLED::WLED()
{
}

// turns all LEDs off and restarts ESP
void WLED::reset()
{
  briT = 0;
  #ifdef WLED_ENABLE_WEBSOCKETS
  ws.closeAll(1012);
#endif
  long dly = millis();
  while (millis() - dly < 450) {
    yield();        // enough time to send response to client
  }
  applyBri();
  USER_PRINTLN(F("\nWLED RESTART\n"));
  USER_FLUSH();   // WLEDMM: wait until Serial has completed sending buffered data
  ESP.restart();
}

#if defined(ARDUINO_ARCH_ESP32) && defined(WLEDMM_FASTPATH)
#define yield() {}  // WLEDMM yield() is completely unnecessary on esp32. See https://github.com/espressif/arduino-esp32/issues/1385
#endif

void background_loop(void* pvParameters) {
  while (true) {
    #ifdef WLED_DEBUG
    // esp_log_level_set("*",ESP_LOG_VERBOSE);
    static unsigned long maxUsermodMillis = 0;
    static uint16_t avgUsermodMillis = 0;
    static unsigned long maxStripMillis = 0;
    static uint16_t avgStripMillis = 0;
    #endif
    handleTime();
    #ifndef WLED_DISABLE_INFRARED
    handleIR();        // 2nd call to function needed for ESP32 to return valid results -- should be good for ESP8266, too
    #endif
    WLED::handleConnection();
    #ifndef WLED_DISABLE_ESPNOW
    handleRemote();
    #endif
    handleSerial();
    #ifndef WLED_DISABLE_IMPROV_WIFISCAN
    handleImprovWifiScan();
    #endif

    handleNotifications();
    handleTransitions();

    #ifdef WLED_ENABLE_DMX
    handleDMXOutput();
    #endif
    #ifdef WLED_ENABLE_DMX_INPUT
    dmxInput.update();
    #endif
    userLoop();

    #ifdef WLED_DEBUG
    unsigned long usermodMillis = millis();
    #endif

    if (xSemaphoreTake(busMutex, portMAX_DELAY)) {
      usermods.loop();
      xSemaphoreGive(busMutex);
    }

    #ifdef WLED_DEBUG
    usermodMillis = millis() - usermodMillis;
    avgUsermodMillis += usermodMillis;
    if (usermodMillis > maxUsermodMillis) maxUsermodMillis = usermodMillis;
    #endif

    handleIO();

    if (doCloseFile) {
      if (xSemaphoreTake(busMutex, portMAX_DELAY)) {
        closeFile();
        xSemaphoreGive(busMutex);
      }
    }

    if (doSerializeConfig) {
      if (xSemaphoreTake(busMutex, portMAX_DELAY)) {
        serializeConfig();
        xSemaphoreGive(busMutex);
      }
    }

    //LED settings have been saved, re-init busses
    //This code block causes severe FPS drop on ESP32 with the original "if (busConfigs[0] != nullptr)" conditional. Investigate!
    if (doInitBusses) {
      if (xSemaphoreTake(busMutex, portMAX_DELAY)) {
        unsigned long waitStart = millis();                                             // WLEDMM: to avoid crash,
        // while (strip.isUpdating() && (millis() - waitStart < 250)) { yield(); delay(5); } // wait a bit until busses are idle (max 250ms)
        doInitBusses = false;
        DEBUG_PRINTLN(F("Re-init busses."));
        bool aligned = strip.checkSegmentAlignment(); //see if old segments match old bus(ses)
        busses.removeAll();
        uint32_t mem = 0;
        for (uint8_t i = 0; i < WLED_MAX_BUSSES + WLED_MIN_VIRTUAL_BUSSES; i++) {
          if (busConfigs[i] == nullptr) break;
          mem += BusManager::memUsage(*busConfigs[i]);
          if (mem <= MAX_LED_MEMORY) {
            busses.add(*busConfigs[i]);
          }
          delete busConfigs[i]; busConfigs[i] = nullptr;
        }
        strip.finalizeInit();
        busses.setBrightness(bri); // fix re-initialised bus' brightness
        loadLedmap = true;
        if (aligned) strip.makeAutoSegments();
        else strip.fixInvalidSegments();
        serializeConfig();
        xSemaphoreGive(busMutex);
      }
    }

    if (loadLedmap) {
      if (xSemaphoreTake(busMutex, portMAX_DELAY)) {
        if (!strip.deserializeMap(loadedLedmap) && strip.isMatrix) strip.setUpMatrix();
        strip.enumerateLedmaps(); //WLEDMM
        loadLedmap = false;
      }
      xSemaphoreGive(busMutex);
    }

    #ifndef WLED_DISABLE_INFRARED
    handleIR();
    #endif

    #ifndef WLED_DISABLE_ALEXA
    handleAlexa();
    #endif

    if (doReboot && !doInitBusses) { // if busses have to be inited & saved, wait until next iteration
      WLED::reset();
    }

    if (!realtimeMode || realtimeOverride || (realtimeMode && useMainSegmentOnly)) {

      if (apActive) dnsServer.processNextRequest();

      #ifndef WLED_DISABLE_OTA
      if (WLED_CONNECTED && aOtaEnabled && !otaLock && correctPIN) ArduinoOTA.handle();
      #endif

      handleNightlight();
      
      #ifndef WLED_DISABLE_HUESYNC
      handleHue();
      #endif

      if (xSemaphoreTake(busMutex, portMAX_DELAY)) {
        handlePlaylist();
        handlePresets();
        usermods.loop2();
        xSemaphoreGive(busMutex);
      }

    }

    if (lastMqttReconnectAttempt > millis()) { //millis() rolls over every 50 days
      rolloverMillis++;
      lastMqttReconnectAttempt = 0;
      ntpLastSyncTime = NTP_NEVER;  // force new NTP query
      strip.restartRuntime();
    }

    if (millis() - lastMqttReconnectAttempt > 30000 || lastMqttReconnectAttempt == 0) { // lastMqttReconnectAttempt==0 forces immediate broadcast
      lastMqttReconnectAttempt = millis();

      #ifndef WLED_DISABLE_MQTT
      initMqtt();
      #endif

      refreshNodeList(); // refresh WLED nodes list
      if (nodeBroadcastEnabled) sendSysInfoUDP();
    }

    // 15min PIN time-out
    if (strlen(settingsPIN) > 0 && millis() - lastEditTime > 900000) {
      correctPIN = false;
      createEditHandler(false);
    }

    handleWs();

    #ifdef STATUSLED
    WLED::handleStatusLED();
    #endif

    app_message_t msg;

    #ifdef SOC_USB_OTG_SUPPORTED
    // Poll for messages without blocking
    if (xQueueReceive(app_queue, &msg, 0)) {
      switch (msg.id) {
      case 1: {
        int slot;
        if (allocate_new_msc_device(&msg, &slot) == ESP_OK) {
          // USER_PRINTLN("USB Disk Connected");
          msc_host_device_info_t info;
          ESP_ERROR_CHECK_WITHOUT_ABORT(msc_host_get_device_info(msc_devices[slot]->msc_device, &info));
          print_device_info(&info);
          // show_list_files_all_devices();
          USER_PRINTLN("ImageCache started");
          ImageCacheManager::getInstance().startPreload("/usb0");
        } else {
          USER_PRINTLN("USB operation failed. Try replugging?");
        }
        break;
      }

      case 2: {
        USER_PRINTLN("USB Device Disconnected");

        int slot = find_slot_by_handle(msg.data.device_handle);
        if (slot >= 0) {
          free_msc_device(slot);
        }
        break;
      }

      default:
        USER_PRINTF("Unknown USB Error message ID: %d\n", msg.id);
        break;
      }
    }
    #endif // SOC_USB_OTG_SUPPORTED

    vTaskDelay(1);

  }
}

void WLED::loop() {
  static bool raised_priority = false;
  if (!raised_priority) {
    vTaskPrioritySet(NULL, configMAX_PRIORITIES - 1);
    raised_priority = true;
  }
  #ifdef WLED_DEBUG
  // esp_log_level_set("*",ESP_LOG_VERBOSE);
  static unsigned long maxUsermodMillis = 0;
  static uint16_t avgUsermodMillis = 0;
  static unsigned long maxStripMillis = 0;
  static uint16_t avgStripMillis = 0;
  #endif

  if (!interfacesInited || strip.getBrightness() == 0) delay(10); // TroyHacks: burn some loop in case there's nothing else to do.

  if (!realtimeMode || realtimeOverride || (realtimeMode && useMainSegmentOnly)) {

    #ifdef WLED_DEBUG
    unsigned long stripMillis = millis();
    #endif

    if (!offMode || strip.isOffRefreshRequired()) {
      if (xSemaphoreTake(busMutex, portMAX_DELAY)) {
        strip.service();
        xSemaphoreGive(busMutex);
      }
    }
    
    #ifdef WLED_DEBUG
    stripMillis = millis() - stripMillis;
    avgStripMillis += stripMillis;
    if (stripMillis > maxStripMillis) maxStripMillis = stripMillis;
    #endif
  }

  #if defined(WLED_DEBUG) && !defined(WLED_DEBUG_HEAP) // DEBUG serial logging (every 30s)
  if (millis() - debugTime > 29999) {
    DEBUG_PRINTLN(F("---DEBUG INFO---"));
    DEBUG_PRINT(F("Name: "));       DEBUG_PRINTLN(serverDescription);
    DEBUG_PRINT(F("Runtime: "));       DEBUG_PRINTLN(millis());
    DEBUG_PRINT(F("Unix time: "));     toki.printTime(toki.getTime());
    DEBUG_PRINT(F("Free heap : "));     DEBUG_PRINTLN(ESP.getFreeHeap());
    DEBUG_PRINT(F("Free heap: "));     DEBUG_PRINTLN(ESP.getFreeHeap());
    //WLEDMM
    #ifdef ARDUINO_ARCH_ESP32
    DEBUG_PRINT(F("Avail heap: "));     DEBUG_PRINTLN(ESP.getMaxAllocHeap());
    DEBUG_PRINTF("%s min free stack %d\n", pcTaskGetTaskName(NULL), uxTaskGetStackHighWaterMark(NULL)); //WLEDMM
    #endif
    #if defined(ARDUINO_ARCH_ESP32) && defined(BOARD_HAS_PSRAM) && !defined(CONFIG_IDF_TARGET_ESP32P4)
    if (psramFound()) {  // OK use
      //DEBUG_PRINT(F("Total PSRAM: "));    DEBUG_PRINT(ESP.getPsramSize()/1024); DEBUG_PRINTLN("kB");
      DEBUG_PRINT(F("Free PSRAM : "));     DEBUG_PRINT(ESP.getFreePsram() / 1024); DEBUG_PRINTLN("kB");
      DEBUG_PRINT(F("Avail PSRAM: "));     DEBUG_PRINT(ESP.getMaxAllocPsram() / 1024); DEBUG_PRINTLN("kB");
      DEBUG_PRINT(F("PSRAM in use:")); DEBUG_PRINT(int(ESP.getPsramSize() - ESP.getFreePsram())); DEBUG_PRINTLN(F(" Bytes"));
    } else {
      //DEBUG_PRINTLN(F("No PSRAM"));
    }
    #endif
    // DEBUG_PRINT(F("Wifi state: "));      DEBUG_PRINTLN(WiFi.status());

    // if (WiFi.status() != lastWifiState) {
    //   wifiStateChangedTime = millis();
    // }
    // lastWifiState = WiFi.status();
    DEBUG_PRINT(F("State time: "));      DEBUG_PRINTLN(wifiStateChangedTime);
    DEBUG_PRINT(F("NTP last sync: "));   DEBUG_PRINTLN(ntpLastSyncTime);
    DEBUG_PRINT(F("Client IP: "));       DEBUG_PRINTLN(Network.localIP());
    if (loops > 0) { // avoid division by zero
      DEBUG_PRINT(F("Loops/sec: "));       DEBUG_PRINTLN(loops / 30);
      DEBUG_PRINT(F("UM time[ms]: "));     DEBUG_PRINT(avgUsermodMillis / loops); DEBUG_PRINT("/");DEBUG_PRINTLN(maxUsermodMillis);
      DEBUG_PRINT(F("Strip time[ms]: "));  DEBUG_PRINT(avgStripMillis / loops); DEBUG_PRINT("/"); DEBUG_PRINTLN(maxStripMillis);
    }
    strip.printSize();
    loops = 0;
    maxUsermodMillis = 0;
    maxStripMillis = 0;
    avgUsermodMillis = 0;
    avgStripMillis = 0;
    debugTime = millis();
    DEBUG_PRINTLN(F("---END OF DEBUG INFO---"));
  }
  loops++;
  #endif
  #ifdef WLED_DEBUG_HEAP
  if (millis() - debugTime > 4999) { // WLEDMM: Special case for debugging heap faster
    DEBUG_PRINT(F("*** Free heap: "));     DEBUG_PRINT(heap_caps_get_free_size(0x1800));
    DEBUG_PRINT(F("\tLargest free block: "));     DEBUG_PRINT(heap_caps_get_largest_free_block(0x1800));
    DEBUG_PRINT(F(" *** \t\tArduino min free stack: ")); DEBUG_PRINT(uxTaskGetStackHighWaterMark(NULL));
    #if INCLUDE_xTaskGetHandle
    DEBUG_PRINT(F("   TCP min free stack: ")); DEBUG_PRINT(wledmm_get_tcp_stacksize());
    #endif
    DEBUG_PRINTLN(F(" ***"));
  }
  #endif        // WLED_DEBUG_HEAP

  toki.resetTick();

  #if WLED_WATCHDOG_TIMEOUT > 0
  esp_task_wdt_reset();
  #endif

  vTaskDelay(1);

} // end main loop
  
#if defined(ARDUINO_ARCH_ESP32) && defined(WLEDMM_FASTPATH)
#undef yield  // WLEDMM restore yield()
#endif

void WLED::enableWatchdog() {
#if WLED_WATCHDOG_TIMEOUT > 0
  #if ESP_IDF_VERSION_MAJOR >= 5
  esp_task_wdt_config_t wdtConfig;
  wdtConfig.timeout_ms = WLED_WATCHDOG_TIMEOUT * 1000;  // convert to milliseconds
  wdtConfig.idle_core_mask = (1 << CONFIG_FREERTOS_NUMBER_OF_CORES) - 1;
  wdtConfig.trigger_panic = false; // TroyHacks P4: Stop panics temporarily until we sort the WDT out.
  esp_err_t watchdog = esp_task_wdt_reconfigure(&wdtConfig);
  #else
  esp_err_t watchdog = esp_task_wdt_init(WLED_WATCHDOG_TIMEOUT, true);
  #endif
  DEBUG_PRINT(F("Watchdog enabled: "));
  if (watchdog == ESP_OK) {
    DEBUG_PRINTLN(F("OK"));
  } else {
    DEBUG_PRINTLN(watchdog);
    return;
  }
  esp_task_wdt_add(NULL);
#endif
}

void WLED::disableWatchdog() {
  #if WLED_WATCHDOG_TIMEOUT > 0
  DEBUG_PRINTLN(F("Watchdog: disabled"));
  esp_task_wdt_delete(NULL);
  #endif
}

int retry_num=0;
static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id,void *event_data){
  if(event_id == WIFI_EVENT_STA_START) {
    USER_PRINTLN("WiFi Started");
    interfacesInited = false;
    wifi_is_connected = false;
  } else if (event_id == WIFI_EVENT_STA_CONNECTED) {
    USER_PRINTLN("WiFi Connected");
    interfacesInited = false;
    wifi_is_connected = false;
  } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
    USER_PRINTLN("WiFi Lost Connection");
    interfacesInited = false;
    wifi_is_connected = false;
    if(retry_num<5){esp_wifi_connect();retry_num++;USER_PRINTLN("Retrying to Connect...\n");}
  } else if (event_id == WIFI_EVENT_HOME_CHANNEL_CHANGE){
    // USER_PRINTLN("WiFi HOME CHANNEL CHAANGED");
  } else if (event_id == WIFI_EVENT_STA_STOP){
    USER_PRINTLN("WiFi Stopped");
    interfacesInited = false;
    wifi_is_connected = false;
  } else if (event_id == IP_EVENT_STA_GOT_IP) {
    USER_PRINTLN("WiFi Got IP");
    interfacesInited = false;
    wifi_is_connected = true;
  } else if (event_id == WIFI_EVENT_AP_START) {
    USER_PRINTLN("SoftAP Started");
    interfacesInited = false;
    wifi_is_connected = false;
  } else if (event_id == WIFI_EVENT_AP_STOP) {
    USER_PRINTLN("SoftAP Stopped");
    interfacesInited = false;
    wifi_is_connected = false;
  } else {
    USER_PRINTF("WiFi threw unidentified code %d\n",event_id);
  }
}

#ifdef WLED_USE_ETHERNET

static void eth_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
  if (event_id == ETHERNET_EVENT_CONNECTED) {
    USER_PRINTLN("Ethernet Link Up");
    eth_is_connected = false;
  } else if (event_id == ETHERNET_EVENT_DISCONNECTED) {
    USER_PRINTLN("Ethernet Link Down");
    eth_is_connected = false;
  } else if (event_id == ETHERNET_EVENT_START) {
    eth_is_connected = false;
    // USER_PRINTLN("Ethernet Started");
  } else {
    USER_PRINTF("Ethernet Undeclared Error %d\n", event_id);
  }
}

static void got_ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
  eth_is_connected = true;
  interfacesInited = false;
}
#endif

void WLED::setup() {

  #ifdef WLED_DEBUG
  // esp_log_level_set("*",ESP_LOG_VERBOSE);
  #endif 

  #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5,0,0)
    #if !defined(WLED_USE_ETHERNET_ONLY)
      #if defined(CONFIG_IDF_TARGET_ESP32P4)
        esp_hosted_init();
      #endif
      esp_netif_init();
      esp_event_loop_create_default();
      // esp_netif_create_default_wifi_sta();
      wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
      esp_wifi_init(&wifi_initiation); 
      esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
      esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
      uint8_t wifi_protocols;
      if (CONFIG_SLAVE_SOC_WIFI_HE_SUPPORT) {
        wifi_protocols = (WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_11AX);
      } else {
        wifi_protocols = (WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N);
      }
      ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_set_protocol((wifi_interface_t)ESP_IF_WIFI_STA, wifi_protocols));
      ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_start());
    #endif

    #ifdef WLED_USE_ETHERNET
      // Initialize TCP/IP network interface

      #if defined(WLED_USE_ETHERNET_ONLY)
      // With coexistence you don't need these again (can crash!)
      ESP_ERROR_CHECK(esp_netif_init());
      ESP_ERROR_CHECK(esp_event_loop_create_default());
      #endif

      // Create default Ethernet interface
      esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
      esp_netif_t *eth_netif = esp_netif_new(&cfg);
      assert(eth_netif);

      // Initialize Ethernet driver
      eth_esp32_emac_config_t esp32_emac_config = ETH_ESP32_EMAC_DEFAULT_CONFIG();
      esp32_emac_config.smi_gpio.mdc_num = 31;
      esp32_emac_config.smi_gpio.mdio_num = 52;
      eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
      eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
      phy_config.phy_addr = 1; // Set PHY address
      phy_config.reset_gpio_num = 51; // Set PHY reset GPIO number

      esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&esp32_emac_config, &mac_config);
      esp_eth_phy_t *phy = esp_eth_phy_new_ip101(&phy_config);

      esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
      
      ESP_ERROR_CHECK(esp_eth_driver_install(&config, &eth_handle));
      ESP_ERROR_CHECK(esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));

      // Start Ethernet driver
      ESP_ERROR_CHECK(esp_eth_start(eth_handle));

      // Register event handler for Ethernet events
      ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));
      ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event_handler, NULL));
    #endif
  #endif

  #if defined(ARDUINO_ARCH_ESP32) && defined(WLED_DISABLE_BROWNOUT_DET)
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detection
  #endif

  pinMode(hardwareRX, INPUT_PULLDOWN); delay(1);        // suppress noise in case RX pin is floating (at low noise energy) - see issue #3128

  #ifdef WLED_BOOTUPDELAY
  delay(WLED_BOOTUPDELAY); // delay to let voltage stabilize, helps with boot issues on some setups
  #endif
  Serial.begin(115200, SERIAL_8N1, SOC_RX0, SOC_TX0, false, 20000UL, 120U);

#if !defined(WLEDMM_NO_SERIAL_WAIT) || defined(WLED_DEBUG)
  if (!Serial) delay(1000); // WLEDMM make sure that Serial has initalized
#else
  if (!Serial) delay(300);  // just a tiny wait to avoid problems later when acessing serial
#endif
  Serial.flush();
#ifdef ARDUINO_ARCH_ESP32
  #if defined(WLED_DEBUG) && (defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32C3) || ARDUINO_USB_CDC_ON_BOOT)
  if (!Serial) delay(2500);  // WLEDMM allow CDC USB serial to initialise (WLED_DEBUG only)
  #endif
  #if ARDUINO_USB_CDC_ON_BOOT || ARDUINO_USB_MODE
    #if ARDUINO_USB_CDC_ON_BOOT && (defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C6) || defined(CONFIG_IDF_TARGET_ESP32P4))
    //  WLEDMM avoid "hung devices" when USB_CDC is enabled; see https://github.com/espressif/arduino-esp32/issues/9043
    Serial.setTxTimeoutMs(0);    // potential side-effect: incomplete debug output, with missing characters whenever TX buffer is full.
    #endif
#if !defined(WLEDMM_NO_SERIAL_WAIT) || defined(WLED_DEBUG)
  if (!Serial) delay(2500);  // WLEDMM: always allow CDC USB serial to initialise
  if (Serial) Serial.println("wait 1");  // waiting a bit longer ensures that a  debug messages are shown in serial monitor
  if (!Serial) delay(2500);
  if (Serial) Serial.println("wait 2");
  if (!Serial) delay(2500);

  if (Serial) Serial.flush(); // WLEDMM
#endif

  //Serial.setTimeout(350); // WLEDMM: don't change timeout, as it causes crashes later
  // WLEDMM: redirect debug output to HWCDC
  #if ARDUINO_USB_CDC_ON_BOOT && (defined(WLED_DEBUG) || defined(SR_DEBUG))
  Serial0.setDebugOutput(false);
  Serial.setDebugOutput(true);
  #endif
  // WLEDMM don't touch serial timeout when we use CDC USB or tinyUSB
  #else // "standard" serial-to-USB chip
  if (Serial) Serial.setTimeout(50);  // WLEDMM - only when serial is initialized
  #endif
  #else  // 8266
  if (Serial) Serial.setTimeout(50);  // WLEDMM - only when serial is initialized
  #endif

  //Serial0.setDebugOutput(false);
  #if CORE_DEBUG_LEVEL || defined(WLED_DEBUG_HEAP) || defined(WLED_DEBUG)
  Serial.setDebugOutput(true);  // enables kernel debug messages on Serial
  #endif
  USER_FLUSH(); delay(100);
  USER_PRINTLN();
  USER_PRINT(F("---WLED "));
  #ifdef WLEDMM_FASTPATH
  USER_PRINT("=FASTPATH= ");
  #endif
  USER_PRINT(versionString);
  USER_PRINT(" ");
  USER_PRINT(VERSION);
  USER_PRINTLN(F(" INIT---"));
  #ifdef WLED_RELEASE_NAME
  USER_PRINTF(" WLEDMM_%s %s, build %s.\n", versionString, releaseString, TOSTRING(VERSION)); // WLEDMM specific
  #endif

  const esp_partition_t *running_partition = esp_ota_get_running_partition();
  USER_PRINTF("Running from: %s which is %u bytes and type %u subtype %u at address %x\n",running_partition->label,running_partition->size,running_partition->type,running_partition->subtype,running_partition->address);

  DEBUG_PRINT(F("esp32 "));
  DEBUG_PRINTLN(ESP.getSdkVersion());
  #if defined(ESP_ARDUINO_VERSION)
    //DEBUG_PRINTF(F("arduino-esp32  0x%06x\n"), ESP_ARDUINO_VERSION);
    DEBUG_PRINTF("arduino-esp32 v%d.%d.%d\n", int(ESP_ARDUINO_VERSION_MAJOR), int(ESP_ARDUINO_VERSION_MINOR), int(ESP_ARDUINO_VERSION_PATCH));  // availeable since v2.0.0
  #else
    DEBUG_PRINTLN(F("arduino-esp32 v1.0.x\n"));  // we can't say in more detail.
  #endif

  USER_PRINT(F("CPU:   ")); USER_PRINT(ESP.getChipModel());
  USER_PRINT(F(" rev.")); USER_PRINT(ESP.getChipRevision());
  USER_PRINT(F(", ")); USER_PRINT(ESP.getChipCores()); USER_PRINT(F(" core(s)"));
  USER_PRINT(F(", ")); USER_PRINT(ESP.getCpuFreqMHz()); USER_PRINTLN(F("MHz."));

  // WLEDMM begin
  delay(20); USER_FLUSH(); // drain serial output buffers
  USER_PRINT(F("CPU    "));
  esp_reset_reason_t resetReason = getRestartReason();
  USER_PRINT(restartCode2InfoLong(resetReason));
  USER_PRINT(F(" (code "));
  USER_PRINT((int)resetReason);
  USER_PRINT(F("). "));
  int core0code = getCoreResetReason(0);
  int core1code = getCoreResetReason(1);
  USER_PRINTF("Core#0 %s (%d)", resetCode2Info(core0code).c_str(), core0code);
  if (core1code > 0) {USER_PRINTF("; Core#1 %s (%d)", resetCode2Info(core1code).c_str(), core1code);}
  USER_PRINTLN(F("."));
  if ((core0code > 1) && (core0code <= 20) && (core0code != 3) && (core0code != 12) && (core0code != 14)) errorFlag = ERR_SYS_REBOOT; // abnormal reboot
  if ((resetReason >= 4) && (resetReason < 10)) errorFlag = ERR_SYS_REBOOT; // abnormal reboot (crash, brownout, watchdog, etc)
  if ((resetReason == ESP_RST_BROWNOUT) || (core0code == 15)) errorFlag = ERR_SYS_BROWNOUT; // brownout detected
  // WLEDMM end

  USER_PRINT(F("FLASH: ")); USER_PRINT((ESP.getFlashChipSize()/1024)/1024);
  // USER_PRINT(F("MB, Mode ")); USER_PRINT(ESP.getFlashChipMode());
  #ifdef WLED_DEBUG
  // switch (ESP.getFlashChipMode()) {
  //   // missing: Octal modes
  //   case FM_QIO:  DEBUG_PRINT(F(" (QIO)")); break;
  //   case FM_QOUT: DEBUG_PRINT(F(" (QOUT)"));break;
  //   case FM_DIO:  DEBUG_PRINT(F(" (DIO)")); break;
  //   case FM_DOUT: DEBUG_PRINT(F(" (DOUT)"));break;
  //   #if defined(CONFIG_IDF_TARGET_ESP32S3) && CONFIG_ESPTOOLPY_FLASHMODE_OPI
  //     case FM_FAST_READ: DEBUG_PRINT(F(" (OPI)"));break;
  //   #else
  //     case FM_FAST_READ: DEBUG_PRINT(F(" (fast_read)"));break;
  //   #endif
  //   case FM_SLOW_READ: DEBUG_PRINT(F(" (slow_read)"));break;
  //   default: break;
  // }
  #endif
  USER_PRINT(F(", speed ")); USER_PRINT(ESP.getFlashChipSpeed()/1000000);USER_PRINTLN(F("MHz."));
  
  #if defined(WLED_DEBUG) && defined(ARDUINO_ARCH_ESP32) && !defined(CONFIG_IDF_TARGET_ESP32C6) && !defined(CONFIG_IDF_TARGET_ESP32P4)
  showRealSpeed();
  #endif

  DEBUG_PRINT(F("heap ")); DEBUG_PRINTLN(ESP.getFreeHeap());

  #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 4, 0)  // unfortunately not available in older framework versions
  DEBUG_PRINT(F("\nArduino  max stack  ")); DEBUG_PRINTLN(getArduinoLoopTaskStackSize());
  #endif
  DEBUG_PRINTF("%s min free stack %d\n", pcTaskGetTaskName(NULL), uxTaskGetStackHighWaterMark(NULL)); //WLEDMM

#if defined(BOARD_HAS_PSRAM) || defined(CONFIG_ESPTOOLPY_FLASHMODE_OPI)
  //psramInit(); //WLEDMM?? softhack007: not sure if explicit init is really needed ... lets disable it here and see if that works
  #if defined(CONFIG_IDF_TARGET_ESP32S3)
    #if CONFIG_ESPTOOLPY_FLASHMODE_OPI || (CONFIG_SPIRAM_MODE_OCT && defined(BOARD_HAS_PSRAM))
      // S3: reserve GPIO 33-37 for "octal" PSRAM
      managed_pin_type pins[] = { {33, true}, {34, true}, {35, true}, {36, true}, {37, true} };
      pinManager.allocateMultiplePins(pins, sizeof(pins)/sizeof(managed_pin_type), PinOwner::SPI_RAM);
    #endif
  #elif defined(CONFIG_IDF_TARGET_ESP32S2)
  // S2: reserve GPIO 26-32 for PSRAM (may fail due to isPinOk() but that will also prevent other allocation)
  //managed_pin_type pins[] = { {26, true}, {27, true}, {28, true}, {29, true}, {30, true}, {31, true}, {32, true} };
  //pinManager.allocateMultiplePins(pins, sizeof(pins)/sizeof(managed_pin_type), PinOwner::SPI_RAM);
  #elif defined(CONFIG_IDF_TARGET_ESP32C3)
  // C3: reserve GPIO 12-17 for PSRAM (may fail due to isPinOk() but that will also prevent other allocation)
  //managed_pin_type pins[] = { {12, true}, {13, true}, {14, true}, {15, true}, {16, true}, {17, true} };
  //pinManager.allocateMultiplePins(pins, sizeof(pins)/sizeof(managed_pin_type), PinOwner::SPI_RAM);
  #elif defined(CONFIG_IDF_TARGET_ESP32P4)
  // ESP32-P4 peripherals are hidden from GPIO map, including PSRAM - so we don't need to further hide them.

  // GPIO > 36 are not powered/configured for Arduino-ESP32 by default.
  // fix from https://esp32.com/viewtopic.php?t=45334 thanks to microfoundry

  esp_ldo_channel_config_t config2 = {
    .chan_id = 3,  // discovered by trial and error
    .voltage_mv = 3300,
    .flags = {
      .adjustable = 1,
      .owned_by_hw = 0,
      .bypass = 0
    }
  };

  // Create configuration for LDO index 3
  esp_ldo_channel_config_t config3 = {
    .chan_id = 4,  // discovered by trial and error
    .voltage_mv = 3300,
    .flags = {
      .adjustable = 1,
      .owned_by_hw = 0,
      .bypass = 0
    }
  };

  // Try to acquire both channels
  if (esp_ldo_acquire_channel(&config2, &ldo2) == ESP_OK) {
    DEBUG_PRINTLN("LDO index 2 acquired");
  } else {
    USER_PRINTLN("Failed to acquire LDO index 2");
  }

  if (esp_ldo_acquire_channel(&config3, &ldo3) == ESP_OK) {
    DEBUG_PRINTLN("LDO index 3 acquired");
  } else {
    USER_PRINTLN("Failed to acquire LDO index 3 - higher GPOIOs may be unavailable.");
  }
  #else
  // GPIO16/GPIO17 reserved for SPI RAM
  managed_pin_type pins[] = { {16, true}, {17, true} };
  pinManager.allocateMultiplePins(pins, sizeof(pins)/sizeof(managed_pin_type), PinOwner::SPI_RAM);
  #endif
  
  #ifdef SOC_RX0
  pinManager.allocatePin(SOC_RX0, false, PinOwner::DebugOut);
  #endif
  #ifdef SOC_TX0
  pinManager.allocatePin(SOC_TX0, true, PinOwner::DebugOut);
  #endif

  // #if defined(SOC_PARLIO_SUPPORTED) && defined(PARLIO) 
  //   #ifndef PARLIO_PINS
  //     #define PARLIO_PINS -1
  //   #endif
  //   constexpr int8_t tempPins[] = { PARLIO_PINS };  // You can define more than 16 here
  //   constexpr int totalDefined = sizeof(tempPins) / sizeof(tempPins[0]);

  //   managed_pin_type parlio_pins[SOC_PARLIO_TX_UNIT_MAX_DATA_WIDTH];
  //   int allocatedCount = 0;

  //   for (int i = 0; i < totalDefined && allocatedCount < SOC_PARLIO_TX_UNIT_MAX_DATA_WIDTH; ++i) {
  //       byte gpio = tempPins[i];

  //       // Try to allocate the pin
  //       if (pinManager.allocatePin(gpio, true, PinOwner::Parallel_IO)) {
  //           parlio_pins[allocatedCount++] = { static_cast<int8_t>(gpio), true };
  //       }
  //   }

  //   // Fill remaining slots with -1 to mark unused
  //   for (int i = allocatedCount; i < SOC_PARLIO_TX_UNIT_MAX_DATA_WIDTH; ++i) {
  //       parlio_pins[i] = { -1, false };
  //   }
  // #endif
  #if defined(BOARD_HAS_PSRAM) && (defined(WLED_USE_PSRAM) || defined(WLED_USE_PSRAM_JSON))       // WLEDMM
  if (psramFound()) {  // OK use
    DEBUG_PRINT(F("Total PSRAM: ")); DEBUG_PRINT(ESP.getPsramSize()/1024); DEBUG_PRINTLN("kB");
    DEBUG_PRINT(F("Free PSRAM : ")); DEBUG_PRINT(ESP.getFreePsram()/1024); DEBUG_PRINTLN("kB");
  }
  #else
    DEBUG_PRINTLN(F("PSRAM not used."));
  #endif
#endif
#ifdef CONFIG_SOC_PPA_SUPPORTED
  ESP_ERROR_CHECK(ppa_register_client(&ppa_blend_config, &ppa_blend_handle));
  ESP_ERROR_CHECK(ppa_register_client(&ppa_fill_config, &ppa_fill_handle));
  ESP_ERROR_CHECK(ppa_register_client(&ppa_srm_config, &ppa_srm_handle));
  ESP_ERROR_CHECK(jpeg_new_decoder_engine(&decode_eng_cfg, &jpgd_handle));
#endif
#if defined(CONFIG_IDF_TARGET_ESP32P4) && defined(SOC_USB_OTG_SUPPORTED)
  DEBUG_PRINTLN("Initializing USB Host...");
  app_queue = xQueueCreate(APP_QUEUE_SIZE, sizeof(app_message_t));
  if (!app_queue) {
    DEBUG_PRINTLN( "Failed to create USB Host app_queue");
    return;
  }
  xTaskCreatePinnedToCore(usb_task, "usb_task", 4096, NULL, 2, NULL, 0);
  DEBUG_PRINTLN("Setup complete. Waiting for USB Host events.");
#endif

  if ((strncmp("ESP32-PICO", ESP.getChipModel(), 10) == 0) || (strncmp("ESP32-U4WDH", ESP.getChipModel(), 11) == 0))
  { // WLEDMM detect pico board and esp32-mini1 board at runtime
    // special handling for PICO-D4: gpio16+17 are in use for onboard SPI FLASH (not PSRAM)
    managed_pin_type pins[] = { {16, true}, {17, true} };
    pinManager.allocateMultiplePins(pins, sizeof(pins)/sizeof(managed_pin_type), PinOwner::SPI_RAM);
  }

  //DEBUG_PRINT(F("LEDs inited. heap usage ~"));
  //DEBUG_PRINTLN(heapPreAlloc - ESP.getFreeHeap());
  USER_FLUSH();  // WLEDMM flush buffer now, before anything time-critical is started.

  pinManager.manageDebugTXPin();

#ifdef WLED_ENABLE_DMX //reserve GPIO2 as hardcoded DMX pin
  pinManager.allocatePin(2, true, PinOwner::DMX);
#endif

#if defined(BOARD_HAS_PSRAM)
  if (psramFound()) {  // OK use
    DEBUG_PRINT(F("\nfree heap ")); DEBUG_PRINTLN(ESP.getFreeHeap());
    USER_PRINTLN(F("JSON gabage collection (initial)."));
    // doc.garbageCollect();   // WLEDMM experimental - this seems to move the complete doc[] into PSRAM TroyHacks FIXME this is just to make IDF 5.5 work
	  USER_PRINT(F("PSRAM in use:")); USER_PRINT(int(ESP.getPsramSize() - ESP.getFreePsram())); USER_PRINTLN(F(" Bytes."));
    DEBUG_PRINT(F("free heap ")); DEBUG_PRINTLN(ESP.getFreeHeap());
  }
#endif

// WLEDMM experimental: support for single neoPixel on Adafruit boards
#if 0
  //#ifdef PIN_NEOPIXEL
  //pinManager.allocatePin(PIN_NEOPIXEL, true, PinOwner::BusDigital);
  //#endif
  #ifdef NEOPIXEL_POWER
    pinManager.allocatePin(NEOPIXEL_POWER, true, PinOwner::Relay);  // just to ensure this GPIO will not get used for other purposes
    pinMode(NEOPIXEL_POWER, OUTPUT);
    digitalWrite(NEOPIXEL_POWER, HIGH);
  #endif
  #ifdef NEOPIXEL_I2C_POWER
    pinManager.allocatePin(NEOPIXEL_I2C_POWER, true, PinOwner::Relay);  // just to ensure this GPIO will not get used for other purposes
    pinMode(NEOPIXEL_I2C_POWER, OUTPUT);
    digitalWrite(NEOPIXEL_I2C_POWER, HIGH);
  #endif
#endif

  USER_PRINTLN();
  DEBUG_PRINTLN(F("Registering usermods ..."));
  registerUsermods();

  DEBUG_PRINT(F("heap ")); DEBUG_PRINTLN(ESP.getFreeHeap());
  #ifdef ARDUINO_ARCH_ESP32
    DEBUG_PRINTF("%s min free stack %d\n", pcTaskGetTaskName(NULL), uxTaskGetStackHighWaterMark(NULL)); //WLEDMM
  #endif

  for (uint8_t i=1; i<WLED_MAX_BUTTONS; i++) btnPin[i] = -1;

  bool fsinit = false;
  USER_PRINTLN(F("Mounting FS ..."));
#ifdef ARDUINO_ARCH_ESP32
  fsinit = WLED_FS.begin(true);
#else
  fsinit = WLED_FS.begin();
#endif
  if (!fsinit) {
    USER_PRINTLN(F("Mount FS failed!"));  // WLEDMM
    errorFlag = ERR_FS_BEGIN;
  } else {
      USER_PRINTLN(F("Mount FS succeeded.")); // WLEDMM
  }
#ifdef WLED_ADD_EEPROM_SUPPORT
  else deEEP();
#else
  initPresetsFile();
#endif
  updateFSInfo();

  USER_PRINT(F("done Mounting FS; "));
  USER_PRINT(((fsBytesTotal-fsBytesUsed)/1024)); USER_PRINTLN(F(" kB free.\n"));

  escapedMac = Network.getEscapedMac();

  WLED_SET_AP_SSID(); // otherwise it is empty on first boot until config is saved

  DEBUG_PRINTLN(F("Reading config"));
  deserializeConfigFromFS();

#if defined(STATUSLED) && STATUSLED>=0
  if (!pinManager.isPinAllocated(STATUSLED)) {
    // NOTE: Special case: The status LED should *NOT* be allocated.
    //       See comments in handleStatusLed().
    pinMode(STATUSLED, OUTPUT);
  }
#endif
  esp_ldo_dump(stdout);
  DEBUG_PRINTLN(F("Initializing strip"));
  beginStrip();
  DEBUG_PRINT(F("heap ")); DEBUG_PRINTLN(ESP.getFreeHeap());

  USER_PRINTLN(F("\nUsermods setup ..."));
  userSetup();
  usermods.setup();
  DEBUG_PRINT(F("heap ")); DEBUG_PRINTLN(ESP.getFreeHeap());

  if (strcmp(clientSSID, DEFAULT_CLIENT_SSID) == 0) {
    showWelcomePage = true;
    // WiFi.persistent(false);
    // #ifdef WLED_USE_ETHERNET
    // WiFi.onEvent(WiFiEvent);
    // #endif
  }

  // fill in unique mdns default
  sprintf_P(cmDNS, PSTR("wled-%*s"), 6, escapedMac.c_str() + 6);

#ifndef WLED_DISABLE_MQTT
  if (mqttDeviceTopic[0] == 0) sprintf_P(mqttDeviceTopic, PSTR("wled/%*s"), 6, escapedMac.c_str() + 6);
  if (mqttClientID[0] == 0)    sprintf_P(mqttClientID, PSTR("WLED-%*s"), 6, escapedMac.c_str() + 6);
#endif

#ifndef WLED_DISABLE_OTA
  if (aOtaEnabled) {
    ArduinoOTA.onStart([]() {
      WLED::instance().disableWatchdog();
      DEBUG_PRINTLN(F("Start ArduinoOTA"));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      // reenable watchdog on failed update
      WLED::instance().enableWatchdog();
    });
    if (strlen(cmDNS) > 0)
      ArduinoOTA.setHostname(cmDNS);
  }
#endif
#ifdef WLED_ENABLE_DMX
  initDMXOutput();
#endif
#ifdef WLED_ENABLE_DMX_INPUT
  dmxInput.init(dmxInputReceivePin, dmxInputTransmitPin, dmxInputEnablePin, dmxInputPort);
#endif

  // HTTP server page init
  DEBUG_PRINTLN(F("initServer"));
  initServer();
  DEBUG_PRINT(F("heap ")); DEBUG_PRINTLN(ESP.getFreeHeap());
  #ifdef ARDUINO_ARCH_ESP32
  DEBUG_PRINT(pcTaskGetTaskName(NULL)); DEBUG_PRINT(F(" free stack ")); DEBUG_PRINTLN(uxTaskGetStackHighWaterMark(NULL));
  #endif

  // Seed FastLED random functions with an esp random value, which already works properly at this point.
  uint32_t seed32 = esp_random();
  seed32 ^= random(0, INT32_MAX);  // WLEDMM some extra entropy (for older frameworks where esp_ramdom alone might be too predictable after startup)

  random16_set_seed((uint16_t)((seed32 & 0xFFFF) ^ (seed32 >> 16)));

  #if WLED_WATCHDOG_TIMEOUT > 0
  enableWatchdog();
  #endif

  #if defined(WLED_DISABLE_BROWNOUT_DET)
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 1); //enable brownout detector
  #endif
  
  #ifdef ARDUINO_RUNNING_CORE
    DEBUG_PRINTF("Arduino core=%d (loop is now on core #%d)\n", int(ARDUINO_RUNNING_CORE), int(xPortGetCoreID()));
  #endif
  #ifdef ARDUINO_EVENT_RUNNING_CORE
    DEBUG_PRINTF("Arduino Event core=%d\n", int(ARDUINO_EVENT_RUNNING_CORE));
  #endif

  // WLEDMM : dump GPIO infos (experimental, UI integration pending)
  //#ifdef WLED_DEBUG
  USER_PRINTLN(F("\nGPIO\t| Assigned to\t\t| Info"));
  USER_PRINTLN(F("--------|-----------------------|------------"));
  for(int pinNr = 0; pinNr < WLED_NUM_PINS; pinNr++) { // 49 = highest PIN on ESP32-S3
#if defined(CONFIG_IDF_TARGET_ESP32S3)
    if((pinManager.isPinOk(pinNr, false)) || (pinNr > 18 && pinNr < 21)) {  // softhack007: list USB pins
#else
    if(pinManager.isPinOk(pinNr, false)) {
#endif
      //if ((!pinManager.isPinAllocated(pinNr)) && (pinManager.getPinSpecialText(pinNr).length() == 0)) continue;      // un-comment to hide no-name,unused GPIO pins
      bool is_inOut = pinManager.isPinOk(pinNr, true);
#if 0 // for testing
      USER_PRINT(pinManager.isPinAnalog(pinNr) ? "A": " ");
      USER_PRINT(pinManager.isPinADC1(pinNr) ? "1": " ");
      USER_PRINT(pinManager.isPinADC2(pinNr) ? "2": " ");
      USER_PRINT(pinManager.isPinTouch(pinNr) ? "T": " ");
      USER_PRINT(pinManager.isPinPWM(pinNr) ? " P": "  ");
      USER_PRINT(pinManager.isPinINT(pinNr) ? "I ": "  ");
#endif
      USER_PRINTF("%s  %2d\t  %-17s %s\t  %s\n", 
          (is_inOut?"i/o":"in "), 
          pinNr, 
          pinManager.getPinOwnerText(pinNr).c_str(),
          pinManager.getPinConflicts(pinNr).c_str(),
          pinManager.getPinSpecialText(pinNr).c_str()
      );
      USER_FLUSH();  // avoid lost lines (Serial buffer overflow)
    }
  }

#if 0 // for testing
  USER_PRINTLN(F("\n"));
  USER_PRINTF("ADC1-0 = %d, ADC1-3 = %d, ADC1-7 = %d, ADC2-0 = %d, ADC2-1 = %d, ADC2-8 = %d, ADC2-10 = %d\n",
    pinManager.getADCPin(PM_ADC1, 0), pinManager.getADCPin(PM_ADC1, 3), pinManager.getADCPin(PM_ADC1, 7), 
    pinManager.getADCPin(PM_ADC2, 0), pinManager.getADCPin(PM_ADC2, 1), pinManager.getADCPin(PM_ADC2, 8),
    pinManager.getADCPin(PM_ADC2, 10)
  );
  USER_PRINTLN();
  for(int p=0; p<11; p++) {
    if(pinManager.getADCPin(PinManagerClass::ADC1, p) < 255)
      USER_PRINTF("ADC1-%d = %d, ", p, pinManager.getADCPin(PinManagerClass::ADC1, p));
  }
  USER_PRINTLN();
  for(int p=0; p<11; p++) {
    if(pinManager.getADCPin(PinManagerClass::ADC2, p) < 255)
      USER_PRINTF("ADC2-%d = %d, ", p, pinManager.getADCPin(PinManagerClass::ADC2, p));
  }
  USER_PRINTLN(F("\n"));
#endif

  USER_PRINT(F("Free heap ")); USER_PRINTLN(ESP.getFreeHeap());USER_PRINTLN();
  USER_PRINTLN(F("WLED initialization done.\n"));
  
  serial_drain();
  Serial.flush();

  delay(50);
  
  xSemaphoreGive(busMutex);

  xTaskCreatePinnedToCore(
    background_loop,  // Task function
    "Background",     // Name
    24000,            // Stack size in words
    NULL,             // Parameters
    1,                // Priority
    NULL,             // Task handle (optional)
    0                 // Core ID (0 or 1)
  );

  //#endif
  // WLEDMM end
}

void WLED::beginStrip() {
  // Initialize NeoPixel Strip and button
  strip.fill(BLACK);    // WLEDMM avoids random colors at power-on
  strip.finalizeInit(); // busses created during deserializeConfig()
  strip.makeAutoSegments();
  strip.setBrightness(0, true); // WLEDMM directly apply BLACK (no transition time)
  strip.setShowCallback(handleOverlayDraw);

  if (turnOnAtBoot) {
    if (briS > 0) bri = briS;
    else if (bri == 0) bri = 128;
  } else {
    // fix for #3196
    briLast = briS; bri = 0;
    strip.fill(BLACK);
    strip.show();
  }
  if (bootPreset > 0) {
    applyPreset(bootPreset, CALL_MODE_INIT);
  }
  colorUpdated(CALL_MODE_INIT);

  // init relay pin
  if (rlyPin>=0) {
    // if (strip.isUpdating()) delay(FRAMETIME_FIXED); // WLEDMM ensure that no background led communication is happening while powering on the strip
    digitalWrite(rlyPin, (rlyMde ? bri : !bri));
    delay(75); // wait for relay to switch and power to stabilize
    strip.show(); // update LEDs
    delay(5);
  }
}

void WLED::initAP(bool resetAP)
{
  if (apBehavior == AP_BEHAVIOR_BUTTON_ONLY && !resetAP)
    return;

  if (resetAP) {
    WLED_SET_AP_SSID();
    strcpy_P(apPass, PSTR(WLED_AP_PASS));
  }
  USER_PRINT(F("Opening access point "));  // WLEDMM
  USER_PRINTLN(apSSID);                    // WLEDMM

  ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_stop());

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_set_mode(WIFI_MODE_APSTA));

  wifi_config_t wifi_ap_config = {};
  strncpy(reinterpret_cast<char*>(wifi_ap_config.ap.ssid), apSSID, sizeof(wifi_ap_config.ap.ssid));
  strncpy(reinterpret_cast<char*>(wifi_ap_config.ap.password), apPass, sizeof(wifi_ap_config.sta.password));
  wifi_ap_config.ap.ssid_len = strlen(apSSID);
  wifi_ap_config.ap.channel = apChannel;
  wifi_ap_config.ap.max_connection = 255;
  wifi_ap_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
  wifi_ap_config.ap.pmf_cfg.required = false;

  ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config));

  wifi_config_t wifi_sta_config = {};
  strncpy(reinterpret_cast<char*>(wifi_sta_config.sta.ssid), clientSSID, sizeof(wifi_sta_config.sta.ssid));
  strncpy(reinterpret_cast<char*>(wifi_sta_config.sta.password), clientPass, sizeof(wifi_sta_config.sta.password));
  wifi_sta_config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
  wifi_sta_config.sta.failure_retry_cnt = 5;
  wifi_sta_config.sta.threshold.authmode = WIFI_AUTH_WPA_PSK;
  wifi_sta_config.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;

  ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_config));

  esp_netif_t* esp_netif_ap = esp_netif_create_default_wifi_ap();
  esp_netif_t* esp_netif_sta = esp_netif_create_default_wifi_sta();

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
  
  ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_start());

  // esp_netif_dns_info_t dns;
  // esp_netif_get_dns_info(esp_netif_sta, ESP_NETIF_DNS_MAIN, &dns);
  // uint8_t dhcps_offer_option = 0x02;
  // ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcps_stop(esp_netif_ap));
  // ESP_ERROR_CHECK(esp_netif_dhcps_option(esp_netif_ap, ESP_NETIF_OP_SET, ESP_NETIF_DOMAIN_NAME_SERVER, &dhcps_offer_option, sizeof(dhcps_offer_option)));
  // ESP_ERROR_CHECK(esp_netif_set_dns_info(esp_netif_ap, ESP_NETIF_DNS_MAIN, &dns));
  // ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcps_start(esp_netif_ap));

  // WiFi.softAPConfig(IPAddress(4, 3, 2, 1), IPAddress(4, 3, 2, 1), IPAddress(255, 255, 255, 0));
  // WiFi.softAP(apSSID, apPass, apChannel, apHide, 8); // WLED-MM allow up to 8 clients for ad-hoc "in the field" syncing.
#if defined(LOLIN_WIFI_FIX) && (defined(ARDUINO_ARCH_ESP32C3) || defined(ARDUINO_ARCH_ESP32C6) || defined(ARDUINO_ARCH_ESP32S2) || defined(ARDUINO_ARCH_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32P4))
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  #endif

  if (!apActive) // start captive portal if AP active
  {
    DEBUG_PRINTLN(F("Init AP interfaces"));
    server.begin();
    if (udpPort > 0 && udpPort != ntpLocalPort) {
      udpConnected = notifierUdp.begin(udpPort);
    }
    if (udpRgbPort > 0 && udpRgbPort != ntpLocalPort && udpRgbPort != udpPort) {
      udpRgbConnected = rgbUdp.begin(udpRgbPort);
    }
    if (udpPort2 > 0 && udpPort2 != ntpLocalPort && udpPort2 != udpPort && udpPort2 != udpRgbPort) {
      udp2Connected = notifier2Udp.begin(udpPort2);
    }
    e131.begin(false, e131Port, e131Universe, E131_MAX_UNIVERSE_COUNT);
    ddp.begin(false, DDP_DEFAULT_PORT);

    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, "*", WiFi.softAPIP());
  }
  apActive = true;
}

bool WLED::initEthernet() {
#if defined(WLED_USE_ETHERNET)
  static bool successfullyConfiguredEthernet = false;

  if (successfullyConfiguredEthernet) {
    // DEBUG_PRINTLN(F("initE: ETH already successfully configured, ignoring"));
    return false;
  }
  if (ethernetType == WLED_ETH_NONE) {
    return false;
  }
  if (ethernetType >= WLED_NUM_ETH_TYPES) {
    DEBUG_PRINT(F("initE: Ignoring attempt for invalid ethernetType ")); DEBUG_PRINTLN(ethernetType);
    return false;
  }

  DEBUG_PRINT(F("initE: Attempting ETH config: ")); DEBUG_PRINTLN(ethernetType);

  // Ethernet initialization should only succeed once -- else reboot required
  ethernet_settings es = ethernetBoards[ethernetType];
  managed_pin_type pinsToAllocate[10] = {
    // first six pins are non-configurable
    esp32_nonconfigurable_ethernet_pins[0],
    esp32_nonconfigurable_ethernet_pins[1],
    esp32_nonconfigurable_ethernet_pins[2],
    esp32_nonconfigurable_ethernet_pins[3],
    esp32_nonconfigurable_ethernet_pins[4],
    esp32_nonconfigurable_ethernet_pins[5],
    { (int8_t)es.eth_mdc,   true },  // [6] = MDC  is output and mandatory
    { (int8_t)es.eth_mdio,  true },  // [7] = MDIO is bidirectional and mandatory
    { (int8_t)es.eth_power, true },  // [8] = optional pin, not all boards use
    { ((int8_t)0xFE),       false }, // [9] = replaced with eth_clk_mode, mandatory
  };
#if defined(WLED_USE_ETHERNET) && !defined(CONFIG_IDF_TARGET_ESP32P4)
  // update the clock pin....
  if (es.eth_clk_mode == ETH_CLOCK_GPIO0_IN) {
    pinsToAllocate[9].pin = 0;
    pinsToAllocate[9].isOutput = false;
  } else if (es.eth_clk_mode == ETH_CLOCK_GPIO0_OUT) {
    pinsToAllocate[9].pin = 0;
    pinsToAllocate[9].isOutput = true;
  } else if (es.eth_clk_mode == ETH_CLOCK_GPIO16_OUT) {
    pinsToAllocate[9].pin = 16;
    pinsToAllocate[9].isOutput = true;
  } else if (es.eth_clk_mode == ETH_CLOCK_GPIO17_OUT) {
    pinsToAllocate[9].pin = 17;
    pinsToAllocate[9].isOutput = true;
  } else {
    DEBUG_PRINT(F("initE: Failing due to invalid eth_clk_mode ("));
    DEBUG_PRINT(es.eth_clk_mode);
    DEBUG_PRINTLN(")");
    return false;
  }

  if (!pinManager.allocateMultiplePins(pinsToAllocate, 10, PinOwner::Ethernet)) {
    DEBUG_PRINTLN(F("initE: Failed to allocate ethernet pins"));
    return false;
  }
#endif

  /*
  For LAN8720 the most correct way is to perform clean reset each time before init
  applying LOW to power or nRST pin for at least 100 us (please refer to datasheet, page 59)
  ESP_IDF > V4 implements it (150 us, lan87xx_reset_hw(esp_eth_phy_t *phy) function in
  /components/esp_eth/src/esp_eth_phy_lan87xx.c, line 280)
  but ESP_IDF < V4 does not. Lets do it:
  [not always needed, might be relevant in some EMI situations at startup and for hot resets]
  */
#if ESP_IDF_VERSION_MAJOR==3
  if (es.eth_power > 0 && es.eth_type == ETH_PHY_LAN8720) {
    pinMode(es.eth_power, OUTPUT);
    digitalWrite(es.eth_power, 0);
    delayMicroseconds(150);
    digitalWrite(es.eth_power, 1);
    delayMicroseconds(10);
  }
#endif

  // if (!ETH.begin(
  //               (uint8_t) es.eth_address,
  //               (int)     es.eth_power,
  //               (int)     es.eth_mdc,
  //               (int)     es.eth_mdio,
  //               (eth_phy_type_t)   es.eth_type,
  //               (eth_clock_mode_t) es.eth_clk_mode
  //               )) {
  if (!ETH.begin()) {
    DEBUG_PRINTLN(F("initC: ETH.begin() failed"));
    // de-allocate the allocated pins
    for (managed_pin_type mpt : pinsToAllocate) {
      pinManager.deallocatePin(mpt.pin, PinOwner::Ethernet);
    }
    return false;
  }

  successfullyConfiguredEthernet = true;
  USER_PRINTLN(F("initC: *** Ethernet successfully configured! ***"));  // WLEDMM
  return true;
#else
  return false; // Ethernet not enabled for build
#endif

}

void WLED::initConnection() {
  USER_PRINTLN("initConnection");

  unsigned long t_wait = millis();
  // while(strip.isUpdating() && (millis() - t_wait < 86)) delay(1); // WLEDMM try to catch a moment when strip is idle
  //if (strip.isUpdating()) USER_PRINTLN("WLED::initConnection: strip still updating.");

#ifdef WLED_ENABLE_WEBSOCKETS
  ws.onEvent(wsEvent);
#endif

  if (staticIP[0] != 0 && staticGateway[0] != 0) {
    // WiFi.config(staticIP, staticGateway, staticSubnet, IPAddress(1, 1, 1, 1));
  } else {
    // WiFi.config(IPAddress((uint32_t)0), IPAddress((uint32_t)0), IPAddress((uint32_t)0));
  }

  lastReconnectAttempt = millis();

#ifdef TROYHACKS_FAILSAFE_BUSSES
  busses.removeAll(); // TROYHACKS FAILSAFE IN CASE BUSSES ARE CAUSING CRASHES
#endif

#ifndef WLED_USE_ETHERNET_ONLY 
  if (!WLED_WIFI_CONFIGURED) {
    USER_PRINTLN(F("No WiFi connection configured."));  // WLEDMM
    if (!apActive) initAP();        // instantly go to ap mode
    return;
  } else if (!apActive) {
    if (apBehavior == AP_BEHAVIOR_ALWAYS) {
      DEBUG_PRINTLN(F("Access point ALWAYS enabled."));
      initAP();
    } else {
      DEBUG_PRINTLN(F("Access point disabled (init)."));
      wifi_mode_t mode;
      esp_wifi_get_mode(&mode);
      if (mode == WIFI_MODE_APSTA) {
        esp_wifi_stop();
        USER_PRINTLN("initConnection WIFI_MODE_APSTA forcing reconnect");
        forceReconnect = true;
      }
    }
  }
  showWelcomePage = false;
#endif
  // convert the "serverDescription" into a valid DNS hostname (alphanumeric)
  char hostname[25];
  prepareHostname(hostname);

#ifndef WLED_USE_ETHERNET_ONLY
  USER_PRINT("Connecting to WiFi: ");
  USER_PRINTLN(clientSSID);
  wifi_config_t wifi_configuration = {};
  strncpy(reinterpret_cast<char*>(wifi_configuration.sta.ssid), clientSSID, sizeof(wifi_configuration.sta.ssid));
  strncpy(reinterpret_cast<char*>(wifi_configuration.sta.password), clientPass, sizeof(wifi_configuration.sta.password));
  wifi_configuration.sta.ssid[sizeof(wifi_configuration.sta.ssid) - 1] = '\0';
  wifi_configuration.sta.password[sizeof(wifi_configuration.sta.password) - 1] = '\0';
  ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_set_config((wifi_interface_t)ESP_IF_WIFI_STA, &wifi_configuration));
  ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_start());
#endif

#ifdef WLED_USE_ETHERNET
  USER_PRINTLN(F("Connecting to Ethernet"));
  USER_PRINTF("Network.isConnected = %d\n", Network.isConnected());
  USER_PRINTF("Network.isEthernet = %d\n", Network.isEthernet());
#endif

  // ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
  // wifi_init_sta();

#if defined(LOLIN_WIFI_FIX) && (defined(ARDUINO_ARCH_ESP32C3) || defined(ARDUINO_ARCH_ESP32C6) || defined(ARDUINO_ARCH_ESP32S2) || defined(ARDUINO_ARCH_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32P4))
// WiFi.setTxPower(WIFI_POWER_8_5dBm);
#endif
// WiFi.setSleep(!noWifiSleep);
  Network.setHostname(hostname);
  USER_PRINTLN("end initConnection");
}

void WLED::initInterfaces()
{
  DEBUG_PRINTLN(F("Init STA interfaces"));

#ifndef WLED_DISABLE_HUESYNC
  IPAddress ipAddress = Network.localIP();
  if (hueIP[0] == 0) {
    hueIP[0] = ipAddress[0];
    hueIP[1] = ipAddress[1];
    hueIP[2] = ipAddress[2];
  }
#endif

// aOtaEnabled=false; strcpy(cmDNS, ""); // WLEDMM use this to disable OTA and mDNS

//WLEDMM: add netdebug variables
#ifdef WLED_DEBUG_HOST
  if (netDebugPrintIP[0] == 0) {
    //WLEDMM: this code moved from net_debug.cpp as we store IP as IPAddress type
    if (!netDebugPrintIP && !netDebugPrintIP.fromString(WLED_DEBUG_HOST)) {
      Network.hostByName(WLED_DEBUG_HOST, netDebugPrintIP);
    } else {
      IPAddress ndIpAddress = Network.localIP();
      netDebugPrintIP[0] = ndIpAddress[0];
      netDebugPrintIP[1] = ndIpAddress[1];
      netDebugPrintIP[2] = ndIpAddress[2];
    }
  }
  if (netDebugPrintPort == 0) 
    #ifdef WLED_DEBUG_PORT
      netDebugPrintPort = WLED_DEBUG_PORT;
    #else
      netDebugPrintPort = 7868; //Default value
    #endif
#endif

#ifndef WLED_DISABLE_ALEXA
  // init Alexa hue emulation
  if (alexaEnabled)
    alexaInit();
#endif

#ifndef WLED_DISABLE_OTA
  if (aOtaEnabled)
    ArduinoOTA.begin();
#endif

  #ifndef WLED_DISABLE_OTA   // WLEDMM
  if (aOtaEnabled) {
    USER_PRINT(F("           ArduinoOTA: "));
    USER_PRINTLN(ArduinoOTA.getHostname());
  }
  #endif                     // WLEDMM end

  // Set up mDNS responder:
  if (strlen(cmDNS) > 0) {
    // "end" must be called before "begin" is called a 2nd time
    // see https://github.com/esp8266/Arduino/issues/7213
    MDNS.end();
    MDNS.begin(cmDNS);

    USER_PRINTF("mDNS started: http://%s.local\n", cmDNS); // WLEDMM
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("wled", "tcp", 80);
    MDNS.addServiceTxt("wled", "tcp", "mac", escapedMac.c_str());
  }
  server.begin();

  if (udpPort > 0 && udpPort != ntpLocalPort) {
    udpConnected = notifierUdp.begin(udpPort);
    if (udpConnected && udpRgbPort != udpPort)
      udpRgbConnected = rgbUdp.begin(udpRgbPort);
    if (udpConnected && udpPort2 != udpPort && udpPort2 != udpRgbPort)
      udp2Connected = notifier2Udp.begin(udpPort2);
  }
  if (ntpEnabled)
    ntpConnected = ntpUdp.begin(ntpLocalPort);

  e131.begin(e131Multicast, e131Port, e131Universe, E131_MAX_UNIVERSE_COUNT);
  ddp.begin(false, DDP_DEFAULT_PORT);
#ifndef WLED_DISABLE_HUESYNC
  reconnectHue();
#endif
#ifndef WLED_DISABLE_MQTT
  initMqtt();
#endif
  interfacesInited = true;
  wasConnected = true;
}

void WLED::handleConnection()
{
  static byte stacO = 0;
  static uint32_t lastHeap = UINT32_MAX;
  static unsigned long heapTime = 0;
  unsigned long now = millis();

  if (now < 2000 && (!WLED_WIFI_CONFIGURED || apBehavior == AP_BEHAVIOR_ALWAYS))
    return;

  if (lastReconnectAttempt == 0) {
    DEBUG_PRINTLN(F("lastReconnectAttempt == 0"));
    initConnection();
    return;
  }

  static unsigned retryCount = 0;  // WLEDMM
  // reconnect WiFi to clear stale allocations if heap gets too low
//   if ((!strip.isUpdating()) && (now - heapTime > 5000)) { // WLEDMM: updated with better logic for small heap available by block, not total. // WLEDMM trying to use a moment when the strip is idle
// #if defined(ARDUINO_ARCH_ESP32S2) || defined(WLED_ENABLE_HUB75MATRIX) // || defined(CONFIG_IDF_TARGET_ESP32P4)
//     uint32_t heap = ESP.getFreeHeap(); // WLEDMM works better on -S2
// #else
//     uint32_t heap = heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL); // was (MALLOC_CAP_INTERNAL|MALLOC_CAP_DEFAULT) WLEDMM: This is a better metric for free heap.
// #endif
//     if (heap < MIN_HEAP_SIZE && lastHeap < MIN_HEAP_SIZE) {
//       if (retryCount < 5) {  // WLEDMM avoid repeated disconnects
//         USER_PRINT(F("Heap too low! (step 2, force reconnect): "));
//         USER_PRINTLN(heap);
//         forceReconnect = true;
//         strip.purgeSegments(true); // remove all but one segments from memory
//         // WLEDMM
//         errorFlag = ERR_LOW_MEM;
//         retryCount ++;
//       }
//       errorFlag = ERR_LOW_MEM;
//     } else if (heap < MIN_HEAP_SIZE) {
//       USER_PRINT(F("Heap too low! (step 1, flush unread UDP): "));
//       USER_PRINTLN(heap);      
//       strip.purgeSegments();
//       notifierUdp.flush();
//       rgbUdp.flush();
//       notifier2Udp.flush();
//       ntpUdp.flush();
//       // WLEDMM
//       errorFlag = ERR_LOW_MEM;
//       retryCount = 1;
//     } else retryCount = 0;  // WLEDMM memory OK - reset counter
//     lastHeap = heap;
//     heapTime = now;
//   }

  byte stac = 0;
  if (apActive) {
    wifi_sta_list_t stationList;
    esp_wifi_ap_get_sta_list(&stationList);
    stac = stationList.num;
    if (stac != stacO) {
      stacO = stac;
      DEBUG_PRINT(F("Connected AP clients: "));
      DEBUG_PRINTLN(stac);
      if (!WLED_CONNECTED && WLED_WIFI_CONFIGURED) {        // trying to connect, but not connected
        if (stac) {
          // WiFi.disconnect();        // disable search so that AP can work
        } else {
          initConnection();         // restart search
        }
      }
    }
  }
  if (forceReconnect) {
    USER_PRINTLN(F("Forcing reconnect."));
    initConnection();
    interfacesInited = false;
    forceReconnect = false;
    wasConnected = false;
    return;
  }
  if (!Network.isConnected()) {
    if (interfacesInited) {
      USER_PRINTLN(F("Disconnected!"));
      interfacesInited = false;
      initConnection();
    }
    //send improv failed 6 seconds after second init attempt (24 sec. after provisioning)
    if (improvActive > 2 && now - lastReconnectAttempt > 6000) {
      sendImprovStateResponse(0x03, true);
      improvActive = 2;
    }
    if (now - lastReconnectAttempt > ((stac) ? 300000 : 18000) && WLED_WIFI_CONFIGURED) {
      if (improvActive == 2) improvActive = 3;
      DEBUG_PRINTLN(F("Last reconnect too old."));
      initConnection();
    }
    if (!apActive && now - lastReconnectAttempt > 12000 && (!wasConnected || apBehavior == AP_BEHAVIOR_NO_CONN)) {
      DEBUG_PRINTLN(F("Not connected AP."));
      initAP();
    }
  }

  if ((eth_is_connected || wifi_is_connected) && !interfacesInited) { //newly connected
    USER_PRINTLN();
    USER_PRINT(F("Connected! IP address: http://"));
    USER_PRINT(Network.localIP());
    if (Network.isEthernet()) {
      USER_PRINTLN(" via Ethernet");
    } else {
      USER_PRINTLN(" via WiFi");
    }

    if (improvActive) {
      if (improvError == 3) sendImprovStateResponse(0x00, true);
      sendImprovStateResponse(0x04);
      if (improvActive > 1) sendImprovIPRPCResult(ImprovRPCType::Command_Wifi);
    }
    initInterfaces();
    userConnected();
    usermods.connected();
    lastMqttReconnectAttempt = 0; // force immediate update

  } else {
  #ifndef WLED_USE_ETHERNET_ONLY
    wifi_mode_t mode;
    esp_wifi_get_mode(&mode);
    if (mode == WIFI_MODE_APSTA) {
      esp_wifi_stop();
      forceReconnect = true;
    }
  #endif
  }
}

// If status LED pin is allocated for other uses, does nothing
// else blink at 1Hz when WLED_CONNECTED is false (no WiFi, ?? no Ethernet ??)
// else blink at 2Hz when MQTT is enabled but not connected
// else turn the status LED off
void WLED::handleStatusLED()
{
  #if defined(STATUSLED)
  uint32_t c = 0;

  #if STATUSLED>=0
  if (pinManager.isPinAllocated(STATUSLED)) {
    return; //lower priority if something else uses the same pin
  }
  #endif

  if (WLED_CONNECTED) {
    c = RGBW32(0,255,0,0);
    ledStatusType = 2;
  } else if (WLED_MQTT_CONNECTED) {
    c = RGBW32(0,128,0,0);
    ledStatusType = 4;
  } else if (apActive) {
    c = RGBW32(0,0,255,0);
    ledStatusType = 1;
  }
  if (ledStatusType) {
    if (millis() - ledStatusLastMillis >= (1000/ledStatusType)) {
      ledStatusLastMillis = millis();
#if 0
      // WLEDMM un-comment this to stop the blinking
      if ((ledStatusType != 2) && (ledStatusType != 4))
        ledStatusState = !ledStatusState;
      else
        ledStatusState = HIGH;
#else
        ledStatusState = !ledStatusState;
#endif
      #if STATUSLED>=0
      digitalWrite(STATUSLED, ledStatusState);
      #else
      busses.setStatusPixel(ledStatusState ? c : 0);
      #endif
    }
  } else {
    #if STATUSLED>=0
      #ifdef STATUSLEDINVERTED
      digitalWrite(STATUSLED, HIGH);
      #else
      digitalWrite(STATUSLED, LOW);
      #endif
    #else
      busses.setStatusPixel(0);
    #endif
  }
  #endif
}
