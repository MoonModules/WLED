#pragma once
#ifndef WLED_FONTS_H
#define WLED_FONTS_H
#include <stdlib.h> // needed to get uint16_t definition
#include <stdint.h> // helps for code analysis with clang

// always disable unicode for 8266 builds - not enough program space
#if !defined(ARDUINO_ARCH_ESP32) && defined(WLED_ENABLE_FULL_FONTS)
#undef WLED_ENABLE_FULL_FONTS
#endif

// preliminary - for testing
#define WLED_ENABLE_TINY_FONT
#define WLED_ENABLE_LARGE_FONTS
#if defined(ARDUINO_ARCH_ESP32) && !defined(WLEDMM_SAVE_FLASH)
  #define WLED_ENABLE_XXXL_FONT
#endif

// tiny 3x5 font (reduced)
#ifdef WLED_ENABLE_TINY_FONT
#include "src/font/clear_font_3x5.h"
#endif

// standard fonts
#include "src/font/console_font_4x6.h"
#include "src/font/console_font_5x8.h"
#include "src/font/console_font_5x12.h"
#include "src/font/console_font_6x8.h"
#include "src/font/console_font_7x9.h"

// Large fonts
#ifdef WLED_ENABLE_LARGE_FONTS
#include "src/font/console_font_12x16.h" // 2 bytes per row
#include "src/font/console_font_12x24.h" // 2 bytes per row
#include "src/font/console_font_16x32.h" // 2 bytes per row
#endif
// Ultra-Large font
#ifdef WLED_ENABLE_XXXL_FONT
#include "src/font/console_font_25x57.h" // 4 bytes per row
#endif

// fontInfo struct returned by getFontInfo
typedef struct {
  unsigned firstChar;           // first supported glyph (32 for standard "reduced" fonts)
  unsigned lastChar;            // last supported glyph  (126 for standard "reduced" fonts)
  unsigned width_bytes;         // single letter width in bytes (default:1)
  // unsigned height_bytes;     // future support
  bool isProgMem;               // raw data points into ProgMem => 8266 needs pgm_read_byte_near()
  const unsigned char* raw;     // array of bytes with raw pixeldata (typicially lives in PROGMEM)
  // note: we only support monospaced fonts
} FontInfo_t;

// logic for font selection based on width and height
inline FontInfo_t getFontInfo(unsigned width, unsigned height) {
  FontInfo_t font = {0}; // will be returned by value

  unsigned pixels = width * height;
  switch (pixels) {
    // WLED standard fonts (PROGMEM)
    case 24: // 4x6 font
      font.raw        = console_font_4x6;
      font.isProgMem  = true;
      font.firstChar  = console_font_4x6_first;
      font.lastChar   = console_font_4x6_last;
      font.width_bytes= 1;
    break;
    case 40: // 5x8 font
      font.raw        = console_font_5x8; 
      font.isProgMem  = true;
      font.firstChar  = console_font_5x8_first;
      font.lastChar   = console_font_5x8_last;
      font.width_bytes= 1;
    break;
    case 48: // 6x8 font
      font.raw        = console_font_6x8; 
      font.isProgMem  = true;
      font.firstChar  = console_font_6x8_first;
      font.lastChar   = console_font_6x8_last;
      font.width_bytes= 1;
    break;
    case 63: // 7x9 font
      font.raw        = console_font_7x9; 
      font.isProgMem  = true;
      font.firstChar  = console_font_7x9_first;
      font.lastChar   = console_font_7x9_last;
      font.width_bytes= 1;
    break;
    case 60: // 5x12 font
      font.raw        = console_font_5x12;
      font.isProgMem  = true;
      font.firstChar  = console_font_5x12_first;
      font.lastChar   = console_font_5x12_last;
      font.width_bytes= 1;
    break;

    // you can add any custom fonts here
#ifdef WLED_ENABLE_TINY_FONT
    case 15: // tiny 3x5 font (reduced)
      font.raw        = clear_font_3x5;
      font.isProgMem  = true;
      font.firstChar  = clear_font_3x5_first;
      font.lastChar   = clear_font_3x5_last;
      font.width_bytes= 1;
    break;
#endif

#ifdef WLED_ENABLE_LARGE_FONTS
    case 192: // 12x16 font
      font.raw        = console_font_12x16;
      font.isProgMem  = true;
      font.firstChar  = console_font_12x16_first;
      font.lastChar   = console_font_12x16_last;
      font.width_bytes= 2;
    break;
    case 288: // 12x24 font
      font.raw        = console_font_12x24;
      font.isProgMem  = true;
      font.firstChar  = console_font_12x24_first;
      font.lastChar   = console_font_12x24_last;
      font.width_bytes= 2;
    break;
    case 512: // 16x32 font
      font.raw        = console_font_16x32;
      font.isProgMem  = true;
      font.firstChar  = console_font_16x32_first;
      font.lastChar   = console_font_16x32_last;
      font.width_bytes= 2;
    break;
#endif

#ifdef WLED_ENABLE_XXXL_FONT
    case 1425: // 25x57 font
      font.raw        = console_font_25x57;
      font.isProgMem  = true;
      font.firstChar  = console_font_25x57_first;
      font.lastChar   = console_font_25x57_last;
      font.width_bytes= 4;
    break;
#endif

    default: // no font
      font.raw        = nullptr;
      font.isProgMem  = false;
      font.firstChar  = 1;
      font.lastChar   = 1;
      font.width_bytes= 1;
  }
  return font;
}

#endif
