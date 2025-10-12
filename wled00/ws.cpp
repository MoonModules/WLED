#include "wled.h"

/*
 * WebSockets server for bidirectional communication
 */
#ifdef WLED_ENABLE_WEBSOCKETS

static volatile uint16_t wsLiveClientId = 0;        // WLEDMM added "static"
static volatile unsigned long wsLastLiveTime = 0;   // WLEDMM
//uint8_t* wsFrameBuffer = nullptr;

#if !defined(ARDUINO_ARCH_ESP32) || defined(WLEDMM_FASTPATH)   // WLEDMM
#define WS_LIVE_INTERVAL_MAX 30
#define WS_LIVE_INTERVAL_MIN 10
#else
#define WS_LIVE_INTERVAL_MAX 80
#define WS_LIVE_INTERVAL_MIN 40
#endif

void wsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
  if(type == WS_EVT_CONNECT){
    //client connected
    DEBUG_PRINTLN(F("WS client connected."));
    sendDataWs(client);
  } else if(type == WS_EVT_DISCONNECT){
    //client disconnected
    if (client->id() == wsLiveClientId) wsLiveClientId = 0;
    DEBUG_PRINTLN(F("WS client disconnected."));
  } else if(type == WS_EVT_DATA){
    DEBUG_PRINTLN(F("WS event data."));
    // data packet
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    if(info->final && info->index == 0 && info->len == len){
      // the whole message is in a single frame and we got all of its data (max. 1450 bytes)
      if(info->opcode == WS_TEXT)
      {
        if (len > 0 && len < 10 && data[0] == 'p') {
          // application layer ping/pong heartbeat.
          // client-side socket layer ping packets are unanswered (investigate)
          client->text(F("pong"));
          return;
        }

        bool verboseResponse = false;
        if (!requestJSONBufferLock(11)) {
          client->text(F("{\"error\":3}")); // ERR_NOBUF
          return;
        }

        DeserializationError error = deserializeJson(doc, data, len);
        JsonObject root = doc.as<JsonObject>();
        if (error || root.isNull()) {
          releaseJSONBufferLock();
          return;
        }
        if (root["v"] && root.size() == 1) {
          //if the received value is just "{"v":true}", send only to this client
          verboseResponse = true;
        } else if (root.containsKey("lv")) {
          wsLiveClientId = root["lv"] ? client->id() : 0;
        } else {
          verboseResponse = deserializeState(root);
        }
        releaseJSONBufferLock(); // will clean fileDoc

        if (!interfaceUpdateCallMode) { // individual client response only needed if no WS broadcast soon
          if (verboseResponse) {
            sendDataWs(client);
          } else {
            // we have to send something back otherwise WS connection closes
            client->text(F("{\"success\":true}"));
          }
          // force broadcast in 500ms after updating client
          //lastInterfaceUpdate = millis() - (INTERFACE_UPDATE_COOLDOWN -500); // ESP8266 does not like this
        }
      }
    } else {
      //message is comprised of multiple frames or the frame is split into multiple packets
      //if(info->index == 0){
        //if (!wsFrameBuffer && len < 4096) wsFrameBuffer = new uint8_t[4096];
      //}

      //if (wsFrameBuffer && len < 4096 && info->index + info->)
      //{

      //}

      if((info->index + len) == info->len){
        if(info->final){
          if(info->message_opcode == WS_TEXT) {
            client->text(F("{\"error\":9}")); //we do not handle split packets right now
          }
        }
      }
      DEBUG_PRINTLN(F("WS multipart message."));
    }
  } else if(type == WS_EVT_ERROR){
    //error was received from the other end
    USER_PRINTLN(F("WS error."));

  } else if(type == WS_EVT_PONG){
    //pong message was received (in response to a ping request maybe)
    DEBUG_PRINTLN(F("WS pong."));

  }
}

void sendDataWs(AsyncWebSocketClient * client)
{
  DEBUG_PRINTF("sendDataWs\n");
  if (!ws.count()) return;

  if (!requestJSONBufferLock(12)) {
    if (client) {
      client->text(F("{\"error\":3}")); // ERR_NOBUF
    } else {
      ws.textAll(F("{\"error\":3}")); // ERR_NOBUF
    }
    return;
  }

  JsonObject state = doc.createNestedObject("state");
  serializeState(state);
  JsonObject info  = doc.createNestedObject("info");
  serializeInfo(info);

  size_t len = measureJson(doc);
  DEBUG_PRINTF("JSON buffer size: %u for WS request (%u).\n", doc.memoryUsage(), len);

  #ifdef ESP8266
  size_t heap1 = ESP.getFreeHeap();  // WLEDMM moved into 8266 specific section
  DEBUG_PRINT(F("heap ")); DEBUG_PRINTLN(ESP.getFreeHeap());
  if (len>heap1) {
    DEBUG_PRINTLN(F("Out of memory (WS)!"));
    return;
  }
  #else
    // DEBUG_PRINTF("%s min free stack %d\n", pcTaskGetTaskName(NULL), uxTaskGetStackHighWaterMark(NULL)); //WLEDMM
  #endif
  if (len < 1) return; // WLEDMM do not allocate 0 size buffer
  AsyncWebSocketBuffer buffer(len);
  #ifdef ESP8266
  size_t heap2 = ESP.getFreeHeap();
  DEBUG_PRINT(F("heap ")); DEBUG_PRINTLN(ESP.getFreeHeap());
  #else
  size_t heap1 = len+16; // WLEDMM
  size_t heap2 = 0; // ESP32 variants do not have the same issue and will work without checking heap allocation
  #endif
  if (!buffer || heap1-heap2<len) {
    releaseJSONBufferLock();
    USER_PRINTLN(F("WS buffer allocation failed."));
    ws.closeAll(1013); //code 1013 = temporary overload, try again later
    ws.cleanupClients(0); //disconnect all clients to release memory
    errorFlag = ERR_LOW_WS_MEM;
    return; //out of memory
  }
  serializeJson(doc, (char *)buffer.data(), len);

  DEBUG_PRINT(F("Sending WS data "));
  if (client) {
    client->text(std::move(buffer));
    DEBUG_PRINTLN(F("to a single client."));
  } else {
    ws.textAll(std::move(buffer));
    DEBUG_PRINTLN(F("to multiple clients."));
  }

  releaseJSONBufferLock();
}

// WLEDMM function to recover full-bright pixel (based on code from upstream alt-buffer, which is based on code from NeoPixelBrightnessBus)
static uint32_t restoreColorLossy(uint32_t c, uint_fast8_t _restaurationBri) {
  if (_restaurationBri == 255) return c;
  if (_restaurationBri == 0) return 0;
  uint8_t* chan = (uint8_t*) &c;
  for (uint_fast8_t i=0; i<4; i++) {
    uint_fast16_t val = chan[i];
    chan[i] = ((val << 8) + _restaurationBri) / (_restaurationBri + 1); //adding _bri slightly improves recovery / stops degradation on re-scale
  }
  return c;
}

// --- Constants for clarity and easy modification ---
namespace LiveLedsWS {
  constexpr uint32_t MEMORY_BACKOFF_MS = 6000;
  constexpr size_t   MAX_PREVIEW_LEDS = 4096;
  constexpr uint8_t  MESSAGE_ID = 'L';
  constexpr uint8_t  VERSION_1D = 1;
  constexpr uint8_t  VERSION_2D = 2;
  constexpr size_t   HEADER_SIZE_1D = 2;
  constexpr size_t   HEADER_SIZE_2D = 4;
}

// --- Helper function to calculate the pixel sampling rate ---
static size_t calculateSamplingFactor() {
#ifndef WLED_DISABLE_2D
  if (strip.isMatrix) {
    const size_t totalMatrixLeds = Segment::maxWidth * Segment::maxHeight;
    if (totalMatrixLeds > LiveLedsWS::MAX_PREVIEW_LEDS * 4) {
      // Pick a scaling factor that aligns well with the matrix width for a better preview
      if (Segment::maxWidth % 4 == 0) return 4;
      if (Segment::maxWidth % 3 == 0) return 3;
      if (Segment::maxWidth % 2 == 0) return 2;
    } else if (totalMatrixLeds > LiveLedsWS::MAX_PREVIEW_LEDS) {
      if (Segment::maxWidth % 2 == 0) return 2;
    }
    return 1; // No sampling needed
  }
#endif
  // Logic for 1D strips
  const size_t totalLeds = strip.getLengthTotal();
  return (totalLeds > 0) ? ((totalLeds - 1) / LiveLedsWS::MAX_PREVIEW_LEDS) + 1 : 1;
}

// --- Helper function to populate the pixel data into the buffer ---
static void populatePixelData(uint8_t* buffer, size_t bufferSize, size_t headerSize, size_t samplingFactor) {
  size_t bufferIndex = headerSize;
  const size_t totalLeds = strip.getLengthTotal();

  for (size_t i = 0; i < totalLeds && bufferIndex < bufferSize - 2; i += samplingFactor) {
  #ifndef WLED_DISABLE_2D
    // For 2D matrices, skip entire rows to maintain aspect ratio
    if (strip.isMatrix && samplingFactor > 1) {
      if ((i / Segment::maxWidth) % samplingFactor != 0) {
        i += Segment::maxWidth * (samplingFactor - 1); // Jump to the start of the next row to sample
        if (i >= totalLeds) break;
      }
    }
  #endif
    uint32_t pixelColor = strip.getPixelColorRestored(i);
    uint8_t w = W(pixelColor);

    if (gammaCorrectPreview) {
      if (w > 0) pixelColor = color_add(pixelColor, RGBW32(w, w, w, 0), false);
      buffer[bufferIndex++] = unGamma8(R(pixelColor));
      buffer[bufferIndex++] = unGamma8(G(pixelColor));
      buffer[bufferIndex++] = unGamma8(B(pixelColor));
    } else {
      buffer[bufferIndex++] = qadd8(w, R(pixelColor));
      buffer[bufferIndex++] = qadd8(w, G(pixelColor));
      buffer[bufferIndex++] = qadd8(w, B(pixelColor));
    }
  }
}

// --- Main function, now much cleaner and acting as a controller ---
static bool sendLiveLedsWs(uint32_t wsClient) {
  AsyncWebSocketClient* wsc = ws.client(wsClient);
  if (!wsc || wsc->queueLength() > 0) return false; // Client invalid or busy

  // Check for memory backoff period
  static unsigned long memory_backoff_ts = 0;
  if (memory_backoff_ts > 0 && millis() - memory_backoff_ts < LiveLedsWS::MEMORY_BACKOFF_MS) {
    return false;
  }
  memory_backoff_ts = 0;

  const size_t totalLeds = strip.getLengthTotal();
  if (totalLeds == 0) return false;

  const size_t samplingFactor = calculateSamplingFactor();
  const size_t ledsToSend = totalLeds / samplingFactor;

  // Determine header size and version based on strip type
  const bool isMatrix =
  #ifndef WLED_DISABLE_2D
    strip.isMatrix;
#else
    false;
#endif
  const size_t headerSize = isMatrix ? LiveLedsWS::HEADER_SIZE_2D : LiveLedsWS::HEADER_SIZE_1D;

  // Allocate buffer
  const size_t bufSize = headerSize + ledsToSend * 3;
  AsyncWebSocketBuffer wsBuf(bufSize);
  if (!wsBuf) {
    USER_PRINTF("WS buffer allocation failed (%u bytes).\n", bufSize);
    errorFlag = ERR_LOW_WS_MEM;
  #ifdef ARDUINO_ARCH_ESP32
    memory_backoff_ts = millis(); // Suspend live preview
  #endif
    return false;
  }

  uint8_t* buffer = reinterpret_cast<uint8_t*>(wsBuf.data());

  // Populate header
  buffer[0] = LiveLedsWS::MESSAGE_ID;
  if (isMatrix) {
    buffer[1] = LiveLedsWS::VERSION_2D;
    buffer[2] = MIN(Segment::maxWidth / samplingFactor, 255);
    buffer[3] = MIN(Segment::maxHeight / samplingFactor, 255);
  } else {
    buffer[1] = LiveLedsWS::VERSION_1D;
  }

  // Populate pixel data
  populatePixelData(buffer, bufSize, headerSize, samplingFactor);

  wsc->binary(std::move(wsBuf));
  return true;
}

void handleWs()
{
  if ((millis() - wsLastLiveTime) > (unsigned long)(max(WS_LIVE_INTERVAL_MIN, min((strip.getLengthTotal()/80), WS_LIVE_INTERVAL_MAX)))) //WLEDMM dynamic nr of peek frames per second
  {
    ws.cleanupClients();
    bool success = true;
    if (wsLiveClientId) success = sendLiveLedsWs(wsLiveClientId);
    wsLastLiveTime = millis();
    if (!success) wsLastLiveTime -= 20; //try again in 20ms if failed due to non-empty WS queue
  }
}

#else
void handleWs() {}
void sendDataWs(AsyncWebSocketClient * client) {}
#pragma message "WebSockets disabled - no live preview."
#endif
