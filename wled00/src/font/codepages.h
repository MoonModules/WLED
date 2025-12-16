#ifndef WLED_CODEPAGES_H
#define WLED_CODEPAGES_H
#include <stdlib.h> // needed to get uint16_t definition
#include <stdint.h> // helps for code analysis with clang

// always disable unicode for 8266 builds - not enough program space
#if !defined(ARDUINO_ARCH_ESP32) && defined(WLED_ENABLE_FULL_FONTS)
#undef WLED_ENABLE_FULL_FONTS
#endif

// UTF-16 visual replacements when decoding fails
//constexpr uint16_t UNKNOWN_CODE = 0x2219;  //  ∙ multiplication dot (try this if you don't like the middle dot)
constexpr uint16_t UNKNOWN_CODE = 0x00B7;    //  · middle dot = unknown code    (generic error)
constexpr uint16_t BAD_CODE     = 0x2022;    //  • bigger dot = cannot decode   (unicode malformed)
constexpr uint16_t EXT_CODE     = 0x263B;    //  ☻ smiling face = extended code (unicode not supported)

// UTF‑8 → reduced UTF‑16 decoding
// translates the next unicode UTF-8 item into a 2-byte "code point"
// return "•" in case of input errors, and for unsupported/invalid UTF-8
uint16_t unicodeToWchar16(const unsigned char* utf8, size_t maxLen);           // unicodetool.cpp

#if defined(WLED_ENABLE_FULL_FONTS)
// translates unicode 2-byte (UTF-16) "code point" into corresponding character in codepage 437 (IBM PC aka PC-8)
uint16_t wchar16ToCodepage437(uint16_t wideChar);                              // codepage437.cpp
#endif

// returns a pointer to the next unicode item - can be used to "advance" conversion after unicodeToWchar16()
// return nullptr at end of input
const unsigned char* nextUnicode(const unsigned char* utf8, size_t maxLen);    // unicodetool.cpp

// unicode-aware string length
size_t strlenUC(const unsigned char* utf8);

// the next (lesser) string index that is safe for cutting an UTF-8 string
// Important: calling code is responsible to provide a string with at least _where_ chars
size_t cutUnicodeAt(const unsigned char* utf8, size_t where);


// special utility function for @troyhacks ;-)
// removes all unicode letter from a C style char[] - conversion is in-place, to avoid heap fragging
// doesn't work on PROGMEM strings, unless you strdup() them into RAM before calling this function
inline void killUnicode(unsigned char* utf8) {
  if (utf8 == nullptr) return;
  size_t clean_index = 0;
  for (size_t i=0; utf8[i] != '\0'; i++)
    if ((utf8[i] > 0) && (utf8[i] < 128)) utf8[clean_index++] = utf8[i]; // only keep pure ASCII; unicode extended chars start at 128
  utf8[clean_index] = '\0';   // ensure proper string termination
}

#endif
