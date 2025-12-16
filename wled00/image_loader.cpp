#include "wled.h"

#ifdef WLED_ENABLE_GIF

#include "GifDecoder.h"


/*
 * Functions to render images from filesystem to segments, used by the "Image" effect
 */

static File file;
static char lastFilename[WLED_MAX_SEGNAME_LEN+2] = "/"; // enough space for "/" + seg.name + '\0'
static GifDecoder<320,320,12,true> decoder;  // this creates the basic object; parameter lzwMaxBits is not used; decoder.alloc() always allocates "everything else" = 24Kb 
static bool gifDecodeFailed = false;
static unsigned long lastFrameDisplayTime = 0, currentFrameDelay = 0;

bool fileSeekCallback(unsigned long position) {
  return file.seek(position);
}

unsigned long filePositionCallback(void) {
  return file.position();
}

int fileReadCallback(void) {
  return file.read();
}

int fileReadBlockCallback(void * buffer, int numberOfBytes) {
  return file.read((uint8_t*)buffer, numberOfBytes);
}

int fileSizeCallback(void) {
  return file.size();
}

bool openGif(const char *filename) {  // side-effect: updates "file"
  file = WLED_FS.open(filename, "r");
  USER_PRINTF("opening GIF file %s\n", filename);

  if (!file) return false;
  return true;
}

static Segment* activeSeg;
static uint16_t gifWidth = 0, gifHeight = 0;  // these two must stay uint16_t, because they are passed by reference
static unsigned segCols = 1;
static unsigned segRows = 1;
static unsigned segLen = 1; // for 1D and 1DExpand support
static int lastCoordinate = -1; // last coordinate (x+y) that was set, used to reduce redundant pixel writes
static unsigned perPixelX, perPixelY; // scaling factors when upscaling

void screenClearCallback(void) {
  activeSeg->fill(0);
}

// this callback runs when the decoder has finished painting all pixels
void updateScreenCallback(void) {
  // perfect time for adding blur
  if (activeSeg->intensity > 1) {
    uint8_t blurAmount = activeSeg->intensity >> 1;
    if ((blurAmount < 48) && (activeSeg->is2D())) activeSeg->blurRows(activeSeg->intensity);  // some blur - fast
    else activeSeg->blur(blurAmount);                                                         // more blur - slower
  }
  lastCoordinate = -1; // invalidate last position
}

// note: GifDecoder drawing is done top right to bottom left, line by line

// callbacks to draw a pixel at (x,y) without scaling: used if GIF size matches (virtual)segment size (faster) works for 1D and 2D segments
void drawPixelCallbackNoScale1D(int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue) {
  activeSeg->setPixelColor(y * gifWidth + x, gamma8(red), gamma8(green), gamma8(blue));
}
void drawPixelCallbackNoScale2D(int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue) { // WLEDMM setPixelColorXY is faster in our fork
  activeSeg->setPixelColorXY(x, y, gamma8(red), gamma8(green), gamma8(blue));
}

void drawPixelCallback1D(int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue) {
  // 1D strip: load pixel-by-pixel left to right, top to bottom (0/0 = top-left in gifs)
  int totalImgPix = (int)gifWidth * gifHeight;
  int start =  ((int)y * gifWidth + (int)x) * segLen / totalImgPix; // simple nearest-neighbor scaling
  if (start == lastCoordinate) return; // skip setting same coordinate again
  lastCoordinate = start;
  for (int i = 0; i < perPixelX; i++) {
    activeSeg->setPixelColor(start + i, gamma8(red), gamma8(green), gamma8(blue));
  }
}

void drawPixelCallback2D(int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue) {
  // simple nearest-neighbor scaling
  int outY = (int)y * segRows / gifHeight;
  int outX = (int)x * segCols  / gifWidth;
  // Pack coordinates uniquely: outY into upper 16 bits, outX into lower 16 bits
  if (((outY << 16) | outX) == lastCoordinate) return; // skip setting same coordinate again
  lastCoordinate = (outY << 16) | outX; // since input is a "scanline" this is sufficient to identify a "unique" coordinate
  // pre-calculate final pixel color - avoids repetition inside the loop
  red = gamma8(red); green=gamma8(green); blue=gamma8(blue);
  uint32_t pColor = RGBW32(red, green, blue, 0);
  // set multiple pixels if upscaling //  softhack007: changed loop x/y order -> minor speedup from better cache locality
  for (int j = 0; j < perPixelY; j++) {
    int pixelY = outY + j;
    for (int i = 0; i < perPixelX; i++) {
      int pixelX = outX + i;
      activeSeg->setPixelColorXY(pixelX, pixelY, pColor);
    }
  }
}
// WLEDMM 2D without upscaling loop - up to 10% faster
void drawPixelCallbackDownScale2D(int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue) {
  // simple nearest-neighbor downscaling
  int outX = (int)x * segCols  / gifWidth;
  int outY = (int)y * segRows / gifHeight;
  int32_t newCoordinate = ((outY << 16) | outX);
  if (newCoordinate == lastCoordinate) return; // skip setting same coordinate again
  lastCoordinate = newCoordinate;
  activeSeg->setPixelColorXY(outX, outY, gamma8(red), gamma8(green), gamma8(blue));
}

//  calculate image scaling; updates scaling factors and sets the best pixel drawing callback
static void calculateScaling() {
  if (gifDecodeFailed) return;
  if ((gifWidth > 0) && (gifHeight > 0)) {
    if (activeSeg->is2D()) {
      perPixelX   = (segCols + gifWidth -1) / gifWidth;
      perPixelY   = (segRows + gifHeight-1) / gifHeight;
      if (segCols != gifWidth || segRows != gifHeight) {
        if ((perPixelX <2) && (perPixelY <2))
          decoder.setDrawPixelCallback(drawPixelCallbackDownScale2D);// use 2D callback with downscaling only
        else
          decoder.setDrawPixelCallback(drawPixelCallback2D);        // use 2D callback with full scaling
      } else {
        decoder.setDrawPixelCallback(drawPixelCallbackNoScale2D); // use 2D callback without scaling
      }
    } else {
      int totalImgPix = (int)gifWidth * gifHeight;
      if (totalImgPix - segLen == 1) totalImgPix--; // handle off-by-one: skip last pixel instead of first (gifs constructed from 1D input pad last pixel if length is odd)
      perPixelX   = (segLen + totalImgPix-1) / totalImgPix;
      if (totalImgPix != segLen) {
        decoder.setDrawPixelCallback(drawPixelCallback1D);        // use 1D callback with scaling
      }
    }    
  }
}

#define IMAGE_ERROR_NONE 0
#define IMAGE_ERROR_NO_NAME 1
#define IMAGE_ERROR_SEG_LIMIT 2
#define IMAGE_ERROR_UNSUPPORTED_FORMAT 3
#define IMAGE_ERROR_FILE_MISSING 4
#define IMAGE_ERROR_DECODER_ALLOC 5
#define IMAGE_ERROR_GIF_DECODE 6
#define IMAGE_ERROR_FRAME_DECODE 7
#define IMAGE_ERROR_WAITING 254
#define IMAGE_ERROR_PREV 255

// renders an image (.gif only; .bmp and .fseq to be added soon) from FS to a segment
byte renderImageToSegment(Segment &seg) {
  if (!seg.name) return IMAGE_ERROR_NO_NAME;
  // disable during effect transition, causes flickering, multiple allocations and depending on image, part of old FX remaining
  //if (seg.mode != seg.currentMode()) return IMAGE_ERROR_WAITING;
  if (activeSeg && activeSeg != &seg) {            // only one segment at a time
    if (!seg.isActive()) return IMAGE_ERROR_SEG_LIMIT; // sanity check: calling segment must be active
    if (gifDecodeFailed || !activeSeg->isActive())     // decoder failed, or last segment became inactive
      endImagePlayback(activeSeg);                     // => allow takeover but clean up first
    else
      return IMAGE_ERROR_SEG_LIMIT;                
  }

  activeSeg = &seg;
  segCols = activeSeg->virtualWidth();
  segRows = activeSeg->virtualHeight();
  segLen = activeSeg->calc_virtualLength();

 if (strncmp(lastFilename +1, seg.name, WLED_MAX_SEGNAME_LEN) != 0) { // segment name changed, load new image
    strcpy(lastFilename, "/");  // filename always starts with '/'
    strncpy(lastFilename +1, seg.name, WLED_MAX_SEGNAME_LEN);
    lastFilename[WLED_MAX_SEGNAME_LEN+1] ='\0';     // ensure proper string termination when segment name was truncated
    gifDecodeFailed = false;
    size_t fnameLen = strlen(lastFilename);
    if ((fnameLen < 4) || strcmp(lastFilename + fnameLen - 4, ".gif") != 0) { // empty segment name, name too short, or name not ending in .gif
      gifDecodeFailed = true;
      DEBUG_PRINTF("GIF decoder unsupported file: %s\n", lastFilename); // can cause message flood if two segments compete over one decoder instance
      return IMAGE_ERROR_UNSUPPORTED_FORMAT;
    }

    // flicker fixer: if we have ws2812b leds, lets wait a moment until RMT output is done.
    unsigned t0 = millis();
    while (strip.isUpdating() && (millis() - t0 < 100)) delay(1); // up to 100 ms = good enough for up to 1800 LEDs per output

    if (file) file.close();
    if (!openGif(lastFilename)) {
      gifDecodeFailed = true;
      USER_PRINTF("GIF file not found: %s\n", lastFilename);
      return IMAGE_ERROR_FILE_MISSING;
    }
    lastCoordinate = -1;
    decoder.setScreenClearCallback(screenClearCallback);
    decoder.setUpdateScreenCallback(updateScreenCallback);
    decoder.setDrawPixelCallback(drawPixelCallbackNoScale1D); //  default: use "fast path" callback without scaling
    decoder.setFileSeekCallback(fileSeekCallback);
    decoder.setFilePositionCallback(filePositionCallback);
    decoder.setFileReadCallback(fileReadCallback);
    decoder.setFileReadBlockCallback(fileReadBlockCallback);
    decoder.setFileSizeCallback(fileSizeCallback);
#if __cpp_exceptions // use exception handler if we can (some targets don't support exceptions)
    try {
#endif
    decoder.alloc(); // this function may throw out-of memory and cause a crash
#if __cpp_exceptions
    } catch (...) {  // if we arrive here, the decoder has thrown an OOM exception
      gifDecodeFailed = true;
      errorFlag = ERR_NORAM_PX;
      USER_PRINTLN(F("\nGIF decoder out of memory. Please try a smaller image file.\n"));
      //USER_PRINTLN("I'm going to shoot myself now.");
      return IMAGE_ERROR_DECODER_ALLOC;
      // decoder cleanup (hi @coderabbitai): No additonal cleanup necessary - decoder.alloc() ultimately uses "new AnimatedGIF". 
      // If new throws, no pointer is assigned, previous decoder state (if any) has already been deleted inside alloc(), so calling decoder.dealloc() here is unnecessary.
    }
#endif
    DEBUG_PRINTLN(F("Starting decoding"));
    int decoderError = decoder.startDecoding();
    if(decoderError < 0) {
      USER_PRINTF("GIF Decoding error %d in startDecoding().\n", decoderError);
      errorFlag = ERR_NORAM_PX;
      gifDecodeFailed = true;
      return IMAGE_ERROR_GIF_DECODE;
    }
    DEBUG_PRINTLN(F("Decoding started"));
    // after startDecoding, we can get GIF size, update static variables and callbacks
    decoder.getSize(&gifWidth, &gifHeight);
    if (gifWidth == 0 || gifHeight == 0) {  // bad gif size: prevent division by zero
      gifDecodeFailed = true;
      USER_PRINTF("Invalid GIF dimensions: %dx%d\n", gifWidth, gifHeight);
      return IMAGE_ERROR_GIF_DECODE;
    }
    USER_PRINTF("Started GIF decoding   %dx%d pixels -> %dx%d Segment\n", gifWidth, gifHeight, segCols, segRows);
    calculateScaling();
  }

  if (gifDecodeFailed) return IMAGE_ERROR_PREV;
  if (!file) { gifDecodeFailed = true; return IMAGE_ERROR_FILE_MISSING; }
  //if (!decoder) { gifDecodeFailed = true; return IMAGE_ERROR_DECODER_ALLOC; }

  // speed 0 = half speed, 128 = normal, 255 = full FX FPS
  // TODO: 0 = 4x slow, 64 = 2x slow, 128 = normal, 192 = 2x fast, 255 = 4x fast
  uint32_t wait = currentFrameDelay * 2 - seg.speed * currentFrameDelay / 128;

  // TODO consider handling this on FX level with a different frametime, but that would cause slow gifs to speed up during transitions
  if (millis() - lastFrameDisplayTime < wait) return IMAGE_ERROR_WAITING;

  // flicker fixer: if we have ws2812b leds, lets wait a moment until RMT output is done.
  unsigned td0 = millis();
  while (strip.isUpdating() && (millis() - td0 < 20)) delay(1); // wait up to 20ms for LED output to finish

  // WLEDMM experimental: segment options might change (mirror, transpose, grouping, spacing) at any time
  calculateScaling(); // --> re-calculate scaling for each frame

  int result = decoder.decodeFrame(false);
  if (result < 0) {
    USER_PRINTF("GIF Decoding error %d in decodeFrame().\n", result);
    gifDecodeFailed = true;
    return IMAGE_ERROR_FRAME_DECODE;
  }

  currentFrameDelay = decoder.getFrameDelay_ms();
  unsigned long tooSlowBy = (millis() - lastFrameDisplayTime) - wait; // if last frame was longer than intended, compensate
  currentFrameDelay = tooSlowBy > currentFrameDelay ? 0 : currentFrameDelay - tooSlowBy;
  lastFrameDisplayTime = millis();

  return IMAGE_ERROR_NONE;
}

void endImagePlayback(Segment *seg) {
  DEBUG_PRINTLN(F("Image playback end called"));
  if (!activeSeg || activeSeg != seg) return;
  if (file) file.close();
  decoder.dealloc();
  gifWidth = 0; gifHeight = 0; // clear cached image dimensions
  gifDecodeFailed = false;
  activeSeg = nullptr;
  strcpy(lastFilename, "/");  // reset filename
  DEBUG_PRINTLN(F("Image playback ended"));
}

#endif
