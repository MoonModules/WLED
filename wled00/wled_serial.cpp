#include "wled.h"
#ifdef ARDUINO_ARCH_ESP32
#include "esp_ota_ops.h"
#endif

#define SERIAL_MAXTIME_MILLIS 500 // to avoid blocking other activities, do not spend more than 100ms with continuous reading
// at 115200 baud, 100ms is enough to send/receive 1280 chars

uint16_t currentBaud = 1152; //default baudrate 115200 (divided by 100)
uint32_t lastUpdate = 0;

void updateBaudRate(uint32_t rate){
  uint16_t rate100 = rate/100;
  if (rate100 == currentBaud || rate100 < 96) return;
  currentBaud = rate100;

  if (!pinManager.isPinAllocated(hardwareTX) || pinManager.getPinOwner(hardwareTX) == PinOwner::DebugOut){
    if (Serial) { Serial.print(F("Baud is now ")); Serial.println(rate); }
  }

  if (Serial) Serial.flush();
  Serial.begin(rate);
}

// RGB LED data return as JSON array. Slow, but easy to use on the other end.
void sendJSON(){
  if (!pinManager.isPinAllocated(hardwareTX) || pinManager.getPinOwner(hardwareTX) == PinOwner::DebugOut) {
    if (!Serial) return; // WLEDMM avoid writing to unconnected USB-CDC
    uint16_t used = strip.getLengthTotal();
    Serial.write('[');
    for (uint16_t i=0; i<used; i++) {
      Serial.print(strip.getPixelColor(i));
      if (i != used-1) Serial.write(',');
    }
    Serial.println("]");
  }
}

bool canUseSerial(void) {   // WLEDMM returns true if Serial can be used for debug output (i.e. not configured for other purpose)
  #if ARDUINO_USB_CDC_ON_BOOT && !defined(WLED_DEBUG_HOST)
  //  USB CDC blocks if disconnected! so check if Serial is active before printing to it.
  if (!Serial) return false;
  #endif
  if (pinManager.isPinAllocated(hardwareTX) && (pinManager.getPinOwner(hardwareTX) != PinOwner::DebugOut)) 
    return false;  // TX allocated to LEDs or other functions
  if ((realtimeMode == REALTIME_MODE_GENERIC)) 
    return false;  // Serial in use for other serial communication
  return true;
} // WLEDMM end

static um_data_t* getAudioData() {
  um_data_t *um_data;
  if (!usermods.getUMData(&um_data, USERMOD_ID_AUDIOREACTIVE)) {
    // add support for no audio
    um_data = simulateSound(SEGMENT.soundSim);
  }
  return um_data;
}

static float mapf(float x, float in_min, float in_max, float out_min, float out_max){
  if (in_max == in_min) return (out_min);  // WLEDMM avoid div/0
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

#include <algorithm> 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void task_list() {

  #define MAX_TASKS 30 // if you see "zero tasks" raise this number. If there's more tasks than this, you get NO tasks back.

  TaskStatus_t taskStatusArray[MAX_TASKS];
  UBaseType_t taskCount;
  uint32_t totalRunTime;

  taskCount = uxTaskGetSystemState(taskStatusArray, MAX_TASKS, &totalRunTime);

  // Sort tasks first by Core ID, then by descending Run Time (CPU usage)
  std::sort(taskStatusArray, taskStatusArray + taskCount, [](const TaskStatus_t& a, const TaskStatus_t& b) {
    // Primary sort: Core ID (Core 0, then Core 1, then unassigned)
    if (a.xCoreID != b.xCoreID) {
      return a.xCoreID < b.xCoreID;
    }
    // Secondary sort: Run Time (higher usage first)
    return a.ulRunTimeCounter > b.ulRunTimeCounter;
  });

  printf("Found %d tasks\n", taskCount);
  printf("Name\t\tState\tPrio\tStack\tRun Time\tCPU %%\tCore\n");

  for (UBaseType_t i = 0; i < taskCount; i++) {
    TaskStatus_t* ts = &taskStatusArray[i];

    const char* state;
    switch (ts->eCurrentState) {
      case eRunning:   state = "Running"; break;
      case eReady:     state = "Ready"; break;
      case eBlocked:   state = "Blocked"; break;
      case eSuspended: state = "Suspended"; break;
      case eDeleted:   state = "Deleted"; break;
      default:         state = "Unknown"; break;
    }

    char cpu_percent[32];
    snprintf(cpu_percent, sizeof(cpu_percent), "%5.2f%%", totalRunTime > 0 ? (100.0f * ts->ulRunTimeCounter) / totalRunTime : 0.0f);

    printf("%-12s %-10s %4u\t%5u\t%10lu\t%s\t%2d\n",
      ts->pcTaskName,
      state,
      ts->uxCurrentPriority,
      ts->usStackHighWaterMark,
      ts->ulRunTimeCounter,
      cpu_percent,
      ts->xCoreID == tskNO_AFFINITY ? -1 : ts->xCoreID);
  }
}

void serial_drain() {
  if (Serial.available()) {
    byte* garbage = (byte*)heap_caps_malloc_prefer(Serial.available() + 100, 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_INTERNAL);
    USER_PRINTF("Throwing away %u bag%s of serial garbage.\n", Serial.available(), Serial.available() > 1 ? "s" : "");
    Serial.read(garbage, Serial.available());
    heap_caps_free(garbage);
  }
}

void handleSerial() {

  if (!Serial) return;  // arduino docs: `if (Serial)` indicates whether or not the USB CDC serial connection is open. For all non-USB CDC ports, this will always return true

  PinOwner srxo = pinManager.getPinOwner(hardwareRX);
  if (srxo != PinOwner::DebugOut && srxo != PinOwner::None) return;
  
  if (Serial.available() > 1) { serial_drain(); return; } // I dunno, something was really gicving me serial garbage.

  unsigned long startTime = millis();
  while ((Serial.available() > 0) && (millis() - startTime < SERIAL_MAXTIME_MILLIS)) {
    byte next = Serial.peek();
    if (next == 'v') {
      Serial.print("WLED"); Serial.write(' '); Serial.println(VERSION);
    } else if (next == 'F') {
      ArtNetSkipFrame = !ArtNetSkipFrame;
      USER_PRINTF("Art-Net Skip-Frame is now %s.\n", ArtNetSkipFrame ? "ON" : "OFF");
    } else if (next == '^') {
      esp_err_t err;
      const esp_partition_t* boot_partition = esp_ota_get_boot_partition();
      const esp_partition_t* running_partition = esp_ota_get_running_partition();
      USER_PRINTF("Running on %s and we should have booted from %s. This %s\n", running_partition->label, boot_partition->label, (String(running_partition->label) == String(boot_partition->label)) ? "is what we expect." : "means OTA messed up!");
      if (String(running_partition->label) == String(boot_partition->label)) {
        esp_partition_iterator_t new_boot_partition_iterator = NULL;
        if (boot_partition->subtype == ESP_PARTITION_SUBTYPE_APP_OTA_0) {
          new_boot_partition_iterator = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, "app1");
        } else {
          new_boot_partition_iterator = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, "app0");
        }
        const esp_partition_t* new_boot_partition = esp_partition_get(new_boot_partition_iterator);
        err = esp_ota_set_boot_partition(new_boot_partition);
        if (err == ESP_OK) {
          USER_PRINTF("Switching boot partitions from %s to %s in 3 seconds!\n", boot_partition->label, new_boot_partition->label);
          delay(3000);
          esp_restart();
        } else {
          USER_PRINTF("Looks like the other app partition (%s) is invalid. Ignoring.\n", new_boot_partition->label);
        }
      } else {
        USER_PRINTF("Looks like the other partion is invalid as we exepected %s but we booted failsafe to %s. Ignoring boot change.\n", boot_partition->label, running_partition->label);
      }
    } else if (next == 'X') { // WLEDMM - force reconnect via Serial
      USER_PRINTLN("handleSerial forcing reconnect");
      forceReconnect = true;
    } else if (next == 'C') { // WLEDMM - force reconnect via Serial
      task_list();
    } else if (next == 'R') { // WLEDMM - force reboot via Serial
      Serial.print("Rebooting ");
      for (int i = 0;i < 5;i++) {
        Serial.print(".");
        delay(200);
      }
      Serial.println(" now!");
      Serial.flush();
      Serial.end();
      ESP.restart();
    } else if (next == 'G') { // WLEDMM - "G"EQ via Serial
      um_data_t* um_data = getAudioData();
      uint8_t fftResult[16] = { 0 };
      if (um_data && um_data->u_data) {
        memcpy(fftResult, um_data->u_data[2], sizeof(fftResult));
      }
      const uint8_t scale = 16;
      uint8_t heights[16];
      for (int i = 0; i < 16; i++) {
        heights[i] = map8(fftResult[i], 0, scale);
      }
      USER_PRINTLN();
      for (int row = scale; row > 0; row--) {
        for (int col = 0; col < 16; col++) {
          if (heights[col] >= row) {
            USER_PRINT("████");
          } else {
            USER_PRINT("    ");
          }
        }
        USER_PRINTLN();
      }
      for (int i = 0; i < 16; i++) {
        USER_PRINTF("%4d", fftResult[i]);
      }
      USER_PRINTLN();
    } else if (next == 'l') {
      TROYHACKS_LPF = !TROYHACKS_LPF;
      USER_PRINTF("LP (highs) filter is now %s\n", TROYHACKS_LPF ? "On" : "Off");
      USER_PRINTF("HP (bass)  filter is now %s\n", TROYHACKS_HPF ? "On" : "Off");
      USER_PRINTF("Notch      filter is now %s\n", TROYHACKS_NOTCH ? "On" : "Off");
    } else if (next == 'h') {
      TROYHACKS_HPF = !TROYHACKS_HPF;
      USER_PRINTF("LP (highs) filter is now %s\n", TROYHACKS_LPF ? "On" : "Off");
      USER_PRINTF("HP (bass)  filter is now %s\n", TROYHACKS_HPF ? "On" : "Off");
      USER_PRINTF("Notch      filter is now %s\n", TROYHACKS_NOTCH ? "On" : "Off");
    } else if (next == 'n') {
      TROYHACKS_NOTCH = !TROYHACKS_NOTCH;
      USER_PRINTF("LP (highs) filter is now %s\n", TROYHACKS_LPF ? "On" : "Off");
      USER_PRINTF("HP (bass)  filter is now %s\n", TROYHACKS_HPF ? "On" : "Off");
      USER_PRINTF("Notch      filter is now %s\n", TROYHACKS_NOTCH ? "On" : "Off");
    } else if (next == 'p') {
      USER_PRINTLN("White Noise Calibration Cleared!");
      float max = 0;
      for (int i = 0; i < 16; i++) {
        fftBinAverage[i] = 0.0f;
      }
    } else if (next == 'P') {
      TROYHACKS_PINKY = !TROYHACKS_PINKY;
      USER_PRINTF("White Noise Calibration %s\n", TROYHACKS_PINKY ? "Started" : "Finished");
      if (TROYHACKS_PINKY) {
        float max = 0;
        for (int i = 0; i < 16; i++) {
          fftBinAverage[i] = 1.0f;
        }
      }
      if (!TROYHACKS_PINKY) {
        float max = 0;
        float min = 100000000;
        for (int i = 0; i < 16; i++) {
          if (fftBinAverage[i] > max) {
            max = fftBinAverage[i];
          }
          if (fftBinAverage[i] < min) {
            min = fftBinAverage[i];
          }
        }
        //  5.53, 8.10 = 1.70, 1.83, 1.82, 1.85, 1.78, 1.82, 1.76, 1.79, 1.79, 1.79, 1.79, 1.79, 1.79, 1.79, 1.89, 2.17
        // 10.05,95.22 = 7.78, 6.95, 5.15, 3.73, 1.70, 2.20, 4.86, 3.95, 5.92, 7.45, 7.64, 7.52, 6.84, 7.14, 6.90, 6.06,
        // 2.11, 16.77 = 9.67, 8.97, 6.87, 4.89, 3.36, 1.70, 4.69, 2.71, 6.73, 8.85, 9.01, 8.53, 7.10, 7.86, 5.23, 4.26,
        //  2.62,19.27 = 7.34, 6.64, 4.99, 3.76, 1.39, 1.00, 3.20, 1.23, 5.07, 6.63, 6.75, 6.37, 5.26, 5.72, 2.73, 2.74, 
        USER_PRINT(min);
        USER_PRINT(",");
        USER_PRINT(max);
        USER_PRINT(" = ");
        for (int i = 0; i < 16; i++) {
          fftBinAverage[i] = mapf(fftBinAverage[i], min, max, (max / min), 1.00f);
          USER_PRINTF("%1.2f, ", fftBinAverage[i]);
        }
        USER_PRINTLN();
      }
    } else if (next == 0xB0) {
      updateBaudRate(115200);
    } else if (next == 0xB1) {
      updateBaudRate(230400);
    } else if (next == 0xB2) {
      updateBaudRate(460800);
    } else if (next == 0xB3) {
      updateBaudRate(500000);
    } else if (next == 0xB4) {
      updateBaudRate(576000);
    } else if (next == 0xB5) {
      updateBaudRate(921600);
    } else if (next == 0xB6) {
      updateBaudRate(1000000);
    } else if (next == 0xB7) {
      updateBaudRate(1500000);
    } else if (next == 'l') {
      sendJSON(); // Send LED data as JSON Array
    } else if (next == '{') { //JSON API
      bool verboseResponse = false;
      if (!requestJSONBufferLock(16)) {
        if (Serial) Serial.println(F("{\"error\":3}")); // ERR_NOBUF
        return;
      }
      Serial.setTimeout(100);
      DeserializationError error = deserializeJson(doc, Serial);
      if (error) {
        releaseJSONBufferLock();
        return;
      }
      verboseResponse = deserializeState(doc.as<JsonObject>());
      //only send response if TX pin is unused for other purposes
      if (verboseResponse && (!pinManager.isPinAllocated(hardwareTX) || pinManager.getPinOwner(hardwareTX) == PinOwner::DebugOut)) {
        doc.clear();
        JsonObject state = doc.createNestedObject("state");
        serializeState(state);
        JsonObject info = doc.createNestedObject("info");
        serializeInfo(info);
        serializeJson(doc, Serial);
        Serial.println();
      }
      releaseJSONBufferLock();
    }
    Serial.read(); //discard the byte
  }

  #ifdef WLED_DEBUG
  if ((millis() - startTime) > SERIAL_MAXTIME_MILLIS) { USER_PRINTLN(F("handleSerial(): need a break after >100ms of activity.")); }
  #endif

}
