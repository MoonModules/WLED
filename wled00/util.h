#ifndef WLED_UTIL_H
#define WLED_UTIL_H

// Library inclusions
#include <Arduino.h>
#define ARDUINOJSON_DECODE_UNICODE 0
#include "src/dependencies/json/AsyncJson-v6.h"
#include "src/dependencies/json/ArduinoJson-v6.h"
#if !defined(FASTLED_VERSION) // pull in FastLED if we don't have it yet (we need the CRGB type)
  #define USE_GET_MILLISECOND_TIMER
  #define FASTLED_INTERNAL
  #include <FastLED.h>
#endif

/*
 * functions exported by util.cpp
 */

#define inoise8 perlin8   // fastled legacy alias
#define inoise16 perlin16 // fastled legacy alias
#define hex2int(a) (((a)>='0' && (a)<='9') ? (a)-'0' : ((a)>='A' && (a)<='F') ? (a)-'A'+10 : ((a)>='a' && (a)<='f') ? (a)-'a'+10 : 0)

int  __attribute__((pure)) getNumVal(const String* req, uint16_t pos);
void parseNumber(const char* str, byte* val, byte minv=0, byte maxv=255);
bool getVal(JsonVariant elem, byte* val, byte minv=0, byte maxv=255);
bool updateVal(const char* req, const char* key, byte* val, byte minv=0, byte maxv=255);
void oappendUseDeflate(bool OnOff); // enable / disable string squeezing
bool oappend(const char* txt); // append new c string to temp buffer efficiently
bool oappendi(int i);          // append new number to temp buffer efficiently
void sappend(char stype, const char* key, int val);
void sappends(char stype, const char* key, char* val);
void prepareHostname(char* hostname);
bool isAsterisksOnly(const char* str, byte maxLen)  __attribute__((pure));

bool requestJSONBufferLock(uint8_t module=255, unsigned timeoutMS = 1800);
void releaseJSONBufferLock();

uint8_t extractModeName(uint8_t mode, const char *src, char *dest, uint8_t maxLen);
uint8_t extractModeSlider(uint8_t mode, uint8_t slider, char *dest, uint8_t maxLen, uint8_t *var = nullptr);
int16_t extractModeDefaults(uint8_t mode, const char *segVar);

void checkSettingsPIN(const char *pin);
uint16_t __attribute__((pure)) crc16(const unsigned char* data_p, size_t length);   // WLEDMM: added attribute pure
String computeSHA1(const String& input);
String getDeviceId();

uint16_t beatsin88_t(accum88 beats_per_minute_88, uint16_t lowest = 0, uint16_t highest = 65535, uint32_t timebase = 0, uint16_t phase_offset = 0);
uint16_t beatsin16_t(accum88 beats_per_minute, uint16_t lowest = 0, uint16_t highest = 65535, uint32_t timebase = 0, uint16_t phase_offset = 0);
uint8_t beatsin8_t(accum88 beats_per_minute, uint8_t lowest = 0, uint8_t highest = 255, uint32_t timebase = 0, uint8_t phase_offset = 0);

uint8_t get_random_wheel_index(uint8_t pos);
CRGB getCRGBForBand(int x, uint8_t *fftResult, int pal); //WLEDMM netmindz ar palette
char *cleanUpName(char *in); // to clean up a name that was read from file

uint32_t hashInt(uint32_t s);
int32_t perlin1D_raw(uint32_t x, bool is16bit = false);
int32_t perlin2D_raw(uint32_t x, uint32_t y, bool is16bit = false);
int32_t perlin3D_raw(uint32_t x, uint32_t y, uint32_t z, bool is16bit = false);
uint16_t perlin16(uint32_t x);
uint16_t perlin16(uint32_t x, uint32_t y);
uint16_t perlin16(uint32_t x, uint32_t y, uint32_t z);
uint8_t perlin8(uint16_t x);
uint8_t perlin8(uint16_t x, uint16_t y);
uint8_t perlin8(uint16_t x, uint16_t y, uint16_t z);

// fast (true) random numbers using hardware RNG, all functions return values in the range lowerlimit to upperlimit-1
// note: for true random numbers with high entropy, do not call faster than every 200ns (5MHz)
// tests show it is still highly random reading it quickly in a loop (better than fastled PRNG)
// for 8bit and 16bit random functions: no limit check is done for best speed
// 32bit inputs are used for speed and code size, limits don't work if inverted or out of range
// inlining does save code size except for random(a,b) and 32bit random with limits
#ifdef ESP8266
#define HW_RND_REGISTER RANDOM_REG32
#else // ESP32 family
#include "soc/wdev_reg.h"
#define HW_RND_REGISTER REG_READ(WDEV_RND_REG)
#endif
#define random hw_random // replace arduino random()
inline uint32_t hw_random() { return HW_RND_REGISTER; };
uint32_t hw_random(uint32_t upperlimit); // not inlined for code size
int32_t hw_random(int32_t lowerlimit, int32_t upperlimit);
inline uint16_t hw_random16() { return HW_RND_REGISTER; };
inline uint16_t hw_random16(uint32_t upperlimit) { return (hw_random16() * upperlimit) >> 16; }; // input range 0-65535 (uint16_t)
inline int16_t hw_random16(int32_t lowerlimit, int32_t upperlimit) { int32_t range = upperlimit - lowerlimit; return lowerlimit + hw_random16(range); }; // signed limits, use int16_t ranges
inline uint8_t hw_random8() { return HW_RND_REGISTER; };
inline uint8_t hw_random8(uint32_t upperlimit) { return (hw_random8() * upperlimit) >> 8; }; // input range 0-255
inline uint8_t hw_random8(uint32_t lowerlimit, uint32_t upperlimit) { uint32_t range = upperlimit - lowerlimit; return lowerlimit + hw_random8(range); }; // input range 0-255

// memory allocation wrappers (util.cpp)
extern "C" {
  // prefer DRAM in d_xalloc functions, PSRAM as fallback
  void *d_malloc(size_t);
  void *d_calloc(size_t, size_t);
  void *d_realloc_malloc(void *ptr, size_t size);  // implements reallocf(): realloc with malloc fallback, original pointer is free'd if reallocation fails
  void d_free(void *ptr);
  // prefer PSRAM in p_xalloc functions, DRAM as fallback
  void *p_malloc(size_t);
  void *p_calloc(size_t, size_t);
  void *p_realloc_malloc(void *ptr, size_t size);  // implements reallocf(): realloc with malloc fallback, original pointer is free'd if reallocation fails
  void p_free(void *ptr);
  // realloc_malloc_nofree() implements the original realloc semantics: 
  //   function returns a pointer to the newly allocated memory. This may be different from ptr, or NULL if the request fails.
  //   The contents will be unchanged in the range from the start of the region up to the minimum of the old and new sizes.
  //   If the new size is larger than the old size, the added memory will not be initialized.
  //   If realloc() fails the original block is left untouched; it is not freed or moved.
  void *d_realloc_malloc_nofree(void *ptr, size_t size);
  void *p_realloc_malloc_nofree(void *ptr, size_t size);
}
#ifndef ESP8266
//inline size_t getFreeHeapSize() { return heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT); } // returns free heap (ESP.getFreeHeap() can include other memory types) // WLEDMM can cause LED glitches
//inline size_t getContiguousFreeHeap() { return heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT); } // returns largest contiguous free block // WLEDMM may glitch, too

extern size_t d_measureFreeHeap(void);
extern size_t d_measureContiguousFreeHeap(void);
inline size_t getFreeHeapSize() { return d_measureFreeHeap();}  // total free heap - with flicker protection
inline size_t getContiguousFreeHeap() { return d_measureContiguousFreeHeap();}  // largest free block - with flicker protection

#else
inline size_t getFreeHeapSize() { return ESP.getFreeHeap(); } // returns free heap
inline size_t getContiguousFreeHeap() { return ESP.getMaxFreeBlockSize(); } // returns largest contiguous free block
#endif
#if 0 // WLEDMM not used yet
#define BFRALLOC_NOBYTEACCESS    (1 << 0) // ESP32 has 32bit accessible DRAM (usually ~50kB free) that must not be byte-accessed, and cannot store float
#define BFRALLOC_PREFER_DRAM     (1 << 1) // prefer DRAM over PSRAM
#define BFRALLOC_ENFORCE_DRAM    (1 << 2) // use DRAM only, no PSRAM
#define BFRALLOC_PREFER_PSRAM    (1 << 3) // prefer PSRAM over DRAM
#define BFRALLOC_ENFORCE_PSRAM   (1 << 4) // use PSRAM only (if available)
#define BFRALLOC_CLEAR           (1 << 5) // clear allocated buffer after allocation
void *allocate_buffer(size_t size, uint32_t type);
#endif

#endif