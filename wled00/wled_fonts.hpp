#pragma once
#ifndef WLED_FONTS_H
#define WLED_FONTS_H
#include <stdlib.h> // needed to get uint16_t definition
#include <stdint.h> // helps for code analysis with clang

// always disable unicode for 8266 builds - not enough program space
#if !defined(ARDUINO_ARCH_ESP32) && defined(WLED_ENABLE_FULL_FONTS)
#undef WLED_ENABLE_FULL_FONTS
#endif

#if !defined(WLED_DISABLE_2D) // don't include any font in 1D-only builds - saves up to 12Kb of flash

// pull in all fonts
#include "src/font/console_font_4x6.h"
#include "src/font/console_font_5x8.h"
#include "src/font/console_font_5x12.h"
#include "src/font/console_font_6x8.h"
#include "src/font/console_font_7x9.h"

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
#if !defined(WLED_DISABLE_2D) // no fonts in 1D-only builds
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
