#pragma once
#ifndef WLED_COLORS_H
#define WLED_COLORS_H

/*
 * Color structs and color utility functions
 */

#include <vector>

#if !defined(FASTLED_VERSION) // pull in FastLED if we don't have it yet (we need the CRGB type)
  #define FASTLED_INTERNAL
  #define USE_GET_MILLISECOND_TIMER
  #include <FastLED.h>
#endif


#if 0 // WLEDMM not used yet
// CRGBW can be used to manipulate 32bit colors faster. However: if it is passed to functions, it adds overhead compared to a uint32_t color
// use with caution and pay attention to flash size. Usually converting a uint32_t to CRGBW to extract r, g, b, w values is slower than using bitshifts
// it can be useful to avoid back and forth conversions between uint32_t and fastled CRGB
struct CRGBW {
    union {
        uint32_t color32; // Access as a 32-bit value (0xWWRRGGBB)
        struct {
            uint8_t b;
            uint8_t g;
            uint8_t r;
            uint8_t w;
        };
        uint8_t raw[4];   // Access as an array in the order B, G, R, W
    };

    // Default constructor
    inline CRGBW() __attribute__((always_inline)) = default;

    // Constructor from a 32-bit color (0xWWRRGGBB)
    constexpr CRGBW(uint32_t color) __attribute__((always_inline)) : color32(color) {}

    // Constructor with r, g, b, w values
    constexpr CRGBW(uint8_t red, uint8_t green, uint8_t blue, uint8_t white = 0) __attribute__((always_inline)) : b(blue), g(green), r(red), w(white) {}

    // Constructor from CRGB
    constexpr CRGBW(CRGB rgb) __attribute__((always_inline)) : b(rgb.b), g(rgb.g), r(rgb.r), w(0) {}

    // Access as an array
    inline const uint8_t& operator[] (uint8_t x) const __attribute__((always_inline)) { return raw[x]; }

    // Assignment from 32-bit color
    inline CRGBW& operator=(uint32_t color) __attribute__((always_inline)) { color32 = color; return *this; }

    // Assignment from r, g, b, w
    inline CRGBW& operator=(const CRGB& rgb) __attribute__((always_inline)) { b = rgb.b; g = rgb.g; r = rgb.r; w = 0; return *this; }

    // Conversion operator to uint32_t
    inline operator uint32_t() const __attribute__((always_inline)) {
      return color32;
    }
    /*
    // Conversion operator to CRGB
    inline operator CRGB() const __attribute__((always_inline)) {
      return CRGB(r, g, b);
    }

    CRGBW& scale32 (uint8_t scaledown) // 32bit math
    {
      if (color32 == 0) return *this; // 2 extra instructions, worth it if called a lot on black (which probably is true) adding check if scaledown is zero adds much more overhead as its 8bit
      uint32_t scale = scaledown + 1;
      uint32_t rb = (((color32 & 0x00FF00FF) * scale) >> 8) & 0x00FF00FF; // scale red and blue
      uint32_t wg = (((color32 & 0xFF00FF00) >> 8) * scale) & 0xFF00FF00; // scale white and green
          color32 =  rb | wg;
      return *this;
    }*/

};

#endif


struct CHSV32 { // 32bit HSV color with 16bit hue for more accurate conversions - credits @dedehai
  union {
    struct {
        uint16_t h;  // hue
        uint8_t s;   // saturation
        uint8_t v;   // value
    };
    uint32_t raw;    // 32bit access
  };
  inline CHSV32() __attribute__((always_inline)) = default; // default constructor

    /// Allow construction from hue, saturation, and value
    /// @param ih input hue
    /// @param is input saturation
    /// @param iv input value
  inline CHSV32(uint16_t ih, uint8_t is, uint8_t iv) __attribute__((always_inline)) // constructor from 16bit h, s, v
        : h(ih), s(is), v(iv) {}
  inline CHSV32(uint8_t ih, uint8_t is, uint8_t iv) __attribute__((always_inline)) // constructor from 8bit h, s, v
        : h((uint16_t)ih << 8), s(is), v(iv) {}
  inline CHSV32(const CHSV& chsv) __attribute__((always_inline))  // constructor from CHSV
    : h((uint16_t)chsv.h << 8), s(chsv.s), v(chsv.v) {}
  inline operator CHSV() const { return CHSV((uint8_t)(h >> 8), s, v); } // typecast to CHSV
};

static inline void hsv2rgb(const CHSV32& hsv, uint32_t& rgb) // convert HSV (16bit hue) to RGB (32bit with white = 0)
{
  unsigned int remainder, region, p, q, t;
  unsigned int h = hsv.h;
  unsigned int s = hsv.s;
  unsigned int v = hsv.v;
  if (s == 0) {
      rgb = v << 16 | v << 8 | v;
      return;
  }
  region = h / 10923;  // 65536 / 6 = 10923
  remainder = (h - (region * 10923)) * 6;
  p = (v * (255 - s)) >> 8;
  q = (v * (255 - ((s * remainder) >> 16))) >> 8;
  t = (v * (255 - ((s * (65535 - remainder)) >> 16))) >> 8;
  switch (region) {
    case 0:
      rgb = v << 16 | t << 8 | p; break;
    case 1:
      rgb = q << 16 | v << 8 | p; break;
    case 2:
      rgb = p << 16 | v << 8 | t; break;
    case 3:
      rgb = p << 16 | q << 8 | v; break;
    case 4:
      rgb = t << 16 | p << 8 | v; break;
    default:
      rgb = v << 16 | p << 8 | q; break;
  }
}

static inline void rgb2hsv(const uint32_t rgb, CHSV32& hsv) // convert RGB to HSV (16bit hue), much more accurate and faster than fastled version
{
    hsv.raw = 0;
    int32_t r = (rgb>>16)&0xFF;
    int32_t g = (rgb>>8)&0xFF;
    int32_t b = rgb&0xFF;
    int32_t minval, maxval, delta;
    minval = min(r, g);
    minval = min(minval, b);
    maxval = max(r, g);
    maxval = max(maxval, b);
    if (maxval == 0)  return; // black
    hsv.v = maxval;
    delta = maxval - minval;
    hsv.s = (255 * delta) / maxval;
    if (hsv.s == 0)  return; // gray value
    if (maxval == r) hsv.h = (10923 * (g - b)) / delta;
    else if (maxval == g)  hsv.h = 21845 + (10923 * (b - r)) / delta;
    else hsv.h = 43690 + (10923 * (r - g)) / delta;
}

#if 0 // WLEDMM not used yet
static inline void colorHStoRGB(uint16_t hue, byte sat, byte* rgb) { //hue, sat to rgb
  uint32_t crgb;
  hsv2rgb(CHSV32(hue, sat, 255), crgb);
  rgb[0] = byte((crgb) >> 16);
  rgb[1] = byte((crgb) >> 8);
  rgb[2] = byte(crgb);
}

// fast scaling function for colors, performs color*scale/256 for all four channels, speed over accuracy
// note: inlining uses less code than actual function calls
static inline uint32_t fast_color_scale(const uint32_t c, const uint8_t scale) {
  uint32_t rb = (((c     & 0x00FF00FF) * scale) >> 8) &  0x00FF00FF;
  uint32_t wg = (((c>>8) & 0x00FF00FF) * scale)       & ~0x00FF00FF;
  return rb | wg;
}
#endif

#endif // WLED_COLORS_H
