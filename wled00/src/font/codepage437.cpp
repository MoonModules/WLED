/* 
   @title     WLED(-MM) - unicode to CP437 conversion
   @repo      https://github.com/MoonModules/WLED-MM, https://github.com/wled/WLED
   @Copyright © 2025 Github WLED and WLED-MM Commit Authors (see "git blame" for details)
   @license   Licensed under the EUPL-1.2 or later
*/

#if defined(WLED_ENABLE_FULL_FONTS)

#include "codepages.h"
#include <string.h>

constexpr uint8_t CP437_UNKNOWN = 250; // small middle dot · // not sure if we better return " " ?

// translates unicode 2-byte (UTF-16) "code points" into corresponding characters in codepage 437 (IBM PC aka PC-8)
// based on a table from https://en.wikipedia.org/wiki/Code_page_437#Character_set
uint16_t wchar16ToCodepage437(uint16_t wideChar) {

  // unicode codes 0 up to 127 are same as ASCII -> pass through
  if ((wideChar < 0x7F) && (wideChar != 0x08)) return wideChar; // excludes 127 = DEL and 8 = BS, so we can map them

  switch (wideChar) {
    // original IBM PC would interpret codes 0x07, 0x08, 0x0A, and 0x0D as BEL, BS, LF, and CR, respectively. 
    // we don't implement any special handling at the moment
    case 0x0008: return 0x08; break; // Backspace: pass through (could be handled differently in future)

    //  unicode codes mapped to characters 1 - 31
    // unicode 0 = C string terminator -> already passed through, never map it !!
    // 1 to 31 (0x01 to 0x1F) are "assorted dingbats" (complementary and decorative characters). 
    // The isolated character 127 (7Fhex) also belongs to this group.
    case 0x263A: return 0x01; break; // ☺︎
    case 0x263B: return 0x02; break; // ☻
    case 0x2665: return 0x03; break; // ♥︎
    case 0x2666: return 0x04; break; // ♦︎
    case 0x2663: return 0x05; break; // ♣︎
    case 0x2660: return 0x06; break; // ♠︎
    case 0x2022: return 0x07; break; // •
    case 0x25D8: return 0x08; break; // ◘
    case 0x25CB: return 0x09; break; // ○
    case 0x25D9: return 0x0A; break; // ◙
    case 0x2642: return 0x0B; break; // ♂︎
    case 0x2640: return 0x0C; break; // ♀︎
    case 0x266A: return 0x0D; break; // ♪
    case 0x266B: return 0x0E; break; // ♫
    case 0x263C: return 0x0F; break; // ☼
    case 0x25BA: return 0x10; break; // ►
    case 0x25C4: return 0x11; break; // ◄
    case 0x2195: return 0x12; break; // ↕︎
    case 0x203C: return 0x13; break; // ‼︎
    case 0x00B6: return 0x14; break; // ¶
    case 0x00A7: return 0x15; break; // §
    case 0x25AC: return 0x16; break; // ▬
    case 0x21A8: return 0x17; break; // ↨
    case 0x2191: return 0x18; break; // ↑
    case 0x2193: return 0x19; break; // ↓
    case 0x2192: return 0x1A; break; // →
    case 0x2190: return 0x1B; break; // ←
    case 0x221F: return 0x1C; break; // ∟
    case 0x2194: return 0x1D; break; // ↔︎
    case 0x25B2: return 0x1E; break; // ▲
    case 0x25BC: return 0x1F; break; // ▼

    //  unicode codes 32 to 126 (0x20 to 0x7E) are the standard ASCII printable characters -> already passed through

    //  unicode codes mapped to characters 127 - 254
    // code 127 DEL -> small arrow back. We don't implement legacy "rubout" or "backspace" for composing letters or for bold printing
    case 0x007F: return 0x1B; break; // ←
    case 0x2302: return 0x7F; break; // ⌂
    // 128 to 175 (0x80 to 0xAF) are a selection of international text characters
    case 0x00C7: return 0x80; break; // Ç
    case 0x00FC: return 0x81; break; // ü
    case 0x00E9: return 0x82; break; // é
    case 0x00E2: return 0x83; break; // â
    case 0x00E4: return 0x84; break; // ä
    case 0x00E0: return 0x85; break; // à
    case 0x00E5: return 0x86; break; // å
    case 0x00E7: return 0x87; break; // ç
    case 0x00EA: return 0x88; break; // ê
    case 0x00EB: return 0x89; break; // ë
    case 0x00E8: return 0x8A; break; // è
    case 0x00EF: return 0x8B; break; // ï
    case 0x00EE: return 0x8C; break; // î
    case 0x00EC: return 0x8D; break; // ì
    case 0x00C4: return 0x8E; break; // Ä
    case 0x00C5: return 0x8F; break; // Å
    case 0x00C9: return 0x90; break; // É
    case 0x00E6: return 0x91; break; // æ
    case 0x00C6: return 0x92; break; // Æ
    case 0x00F4: return 0x93; break; // ô
    case 0x00F6: return 0x94; break; // ö
    case 0x00F2: return 0x95; break; // ò
    case 0x00FB: return 0x96; break; // û
    case 0x00F9: return 0x97; break; // ù
    case 0x00FF: return 0x98; break; // ÿ
    case 0x00D6: return 0x99; break; // Ö
    case 0x00DC: return 0x9A; break; // Ü
    case 0x00A2: return 0x9B; break; // ¢
    case 0x00A3: return 0x9C; break; // £
    case 0x00A5: return 0x9D; break; // ¥
    case 0x20A7: return 0x9E; break; // ₧
    case 0x0192: return 0x9F; break; // ƒ
    case 0x00E1: return 0xA0; break; // á
    case 0x00ED: return 0xA1; break; // í
    case 0x00F3: return 0xA2; break; // ó
    case 0x00FA: return 0xA3; break; // ú
    case 0x00F1: return 0xA4; break; // ñ
    case 0x00D1: return 0xA5; break; // Ñ
    case 0x00AA: return 0xA6; break; // ª
    case 0x00BA: return 0xA7; break; // º
    case 0x00BF: return 0xA8; break; // ¿
    case 0x2310: return 0xA9; break; // ⌐
    case 0x00AC: return 0xAA; break; // ¬
    case 0x00BD: return 0xAB; break; // ½
    case 0x00BC: return 0xAC; break; // ¼
    case 0x00A1: return 0xAD; break; // ¡
    case 0x00AB: return 0xAE; break; // «
    case 0x00BB: return 0xAF; break; // »
    // 176 to 223 (0xB0 to 0xDF) are box drawing and block characters
    case 0x2591: return 0xB0; break; // ░
    case 0x2592: return 0xB1; break; // ▒
    case 0x2593: return 0xB2; break; // ▓
    case 0x2502: return 0xB3; break; // │
    case 0x2524: return 0xB4; break; // ┤
    case 0x2561: return 0xB5; break; // ╡
    case 0x2562: return 0xB6; break; // ╢
    case 0x2556: return 0xB7; break; // ╖
    case 0x2555: return 0xB8; break; // ╕
    case 0x2563: return 0xB9; break; // ╣
    case 0x2551: return 0xBA; break; // ║
    case 0x2557: return 0xBB; break; // ╗
    case 0x255D: return 0xBC; break; // ╝
    case 0x255C: return 0xBD; break; // ╜
    case 0x255B: return 0xBE; break; // ╛
    case 0x2510: return 0xBF; break; // ┐
    case 0x2514: return 0xC0; break; // └
    case 0x2534: return 0xC1; break; // ┴
    case 0x252C: return 0xC2; break; // ┬
    case 0x251C: return 0xC3; break; // ├
    case 0x2500: return 0xC4; break; // ─
    case 0x253C: return 0xC5; break; // ┼
    case 0x255E: return 0xC6; break; // ╞
    case 0x255F: return 0xC7; break; // ╟
    case 0x255A: return 0xC8; break; // ╚
    case 0x2554: return 0xC9; break; // ╔
    case 0x2569: return 0xCA; break; // ╩
    case 0x2566: return 0xCB; break; // ╦
    case 0x2560: return 0xCC; break; // ╠
    case 0x2550: return 0xCD; break; // ═
    case 0x256C: return 0xCE; break; // ╬
    case 0x2567: return 0xCF; break; // ╧
    case 0x2568: return 0xD0; break; // ╨
    case 0x2564: return 0xD1; break; // ╤
    case 0x2565: return 0xD2; break; // ╥
    case 0x2559: return 0xD3; break; // ╙
    case 0x2558: return 0xD4; break; // ╘
    case 0x2552: return 0xD5; break; // ╒
    case 0x2553: return 0xD6; break; // ╓
    case 0x256B: return 0xD7; break; // ╫
    case 0x256A: return 0xD8; break; // ╪
    case 0x2518: return 0xD9; break; // ┘
    case 0x250C: return 0xDA; break; // ┌
    case 0x2588: return 0xDB; break; // █
    case 0x2584: return 0xDC; break; // ▄
    case 0x258C: return 0xDD; break; // ▌
    case 0x2590: return 0xDE; break; // ▐
    case 0x2580: return 0xDF; break; // ▀
    // 224 to 235 (0xE0 to 0xEB) are math symbols part 1 - Greek letters commonly used in physics
    case 0x03B1: return 0xE0; break; // α
    case 0x00DF: return 0xE1; break; // ß
    case 0x0393: return 0xE2; break; // Γ
    case 0x03C0: return 0xE3; break; // π
    case 0x03A3: return 0xE4; break; // Σ
    case 0x03C3: return 0xE5; break; // σ
    case 0x00B5: return 0xE6; break; // µ
    case 0x03C4: return 0xE7; break; // τ
    case 0x03A6: return 0xE8; break; // Φ
    case 0x0398: return 0xE9; break; // Θ
    case 0x03A9: return 0xEA; break; // Ω
    case 0x03B4: return 0xEB; break; // δ
    // 236 to 254 (0xEC to 0xFE) are other common physics and math symbols
    case 0x221E: return 0xEC; break; // ∞
    case 0x03C6: return 0xED; break; // φ
    case 0x03B5: return 0xEE; break; // ε
    case 0x2229: return 0xEF; break; // ∩
    case 0x2261: return 0xF0; break; // ≡
    case 0x00B1: return 0xF1; break; // ±
    case 0x2265: return 0xF2; break; // ≥
    case 0x2264: return 0xF3; break; // ≤
    case 0x2320: return 0xF4; break; // ⌠
    case 0x2321: return 0xF5; break; // ⌡
    case 0x00F7: return 0xF6; break; // ÷
    case 0x2248: return 0xF7; break; // ≈
    case 0x00B0: return 0xF8; break; // ° degree
    case 0x2219: return 0xF9; break; // ∙ multiplication dot
    case 0x00B7: return 0xFA; break; // · middle dot
    case 0x221A: return 0xFB; break; // √
    case 0x207F: return 0xFC; break; // ⁿ
    case 0x00B2: return 0xFD; break; // ²
    case 0x25A0: return 0xFE; break; // ■ geometric shapes
    // 255 (0xFF) is "non breakable space" (NBSP)
    case 0x00A0: return 32;   break; // NBSP -> normal "space"

    // special mappings for very similar unicode characters
    case 0x00A6: return 0x7C; break; // ¦ broken bar -> | bar
    case 0x266C: return 14;   break; // musical note ♬ -> ♫
    case 0x0394: return 127;  break; // greek capital delta Δ -> ⌂
    case 0x23AE: return 179;  break; // integral extension ⎮
    case 0x03B2: return 225;  break; // greek small beta β -> sz umlaut ß
    case 0x03A0: return 227;  break; // greek capital PI Π
    case 0x220F: return 227;  break; // math product ∏
    case 0x2211: return 228;  break; // math sum ∑
    case 0x03BC: return 230;  break; // greek mu small μ
    case 0x2126: return 234;  break; // Ohm Ω
    case 0x00F0: return 235;  break; // small eth ð
    case 0x2202: return 235;  break; // math partial derivate ∂
    case 0x03D5: return 237;  break; // greek phi small ϕ
    case 0x2205: return 237;  break; // empty set ∅
    case 0x2300: return 237;  break; // diameter ⌀
    case 0x00D8: return 237;  break; // 0 strikethrough Ø
    case 0x00F8: return 237;  break; // 0 strikethrough small ø
    case 0x017F: return 244;  break; // long S ſ
    case 0x02DA: return 0xF8; break; // small circle (up) ˚

    case 0x2208: return 238;  break; // element-of ∈ -> ε
    case 0x20AC: return 238;  break; // Euro € -> ε
    case 0x2713: return 251;  break; // check mark ✓ -> √
    case 0x263E: return 0x01; break; // ☾ last quarter moon (Moonmodules) -> ☺︎ face

    // everything else: unknown
    //default: return 32; // blank
    default: return CP437_UNKNOWN; // small middle dot ·
  }

}

#endif
