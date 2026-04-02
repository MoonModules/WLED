---
applyTo: "**/*.cpp,**/*.h,**/*.ino"
---
# C++ Coding Conventions

See also: [CONTRIBUTING.md](../CONTRIBUTING.md) for general style guidelines that apply to all contributors.

## Formatting

- Indent with **2 spaces** (no tabs in C++ files)
- Opening braces on the same line is preferred (K&R style). Brace on a separate line (Allman style) is acceptable
- Single-statement `if` bodies may omit braces: `if (a == b) doStuff(a);`
- Space between keyword and parenthesis: `if (...)`, `for (...)`. No space between function name and parenthesis: `doStuff(a)`
- No enforced line-length limit; wrap when a line exceeds your editor width

## Naming

- **camelCase** for functions and variables: `setValuesFromMainSeg()`, `effectCurrent`
- **PascalCase** for classes and structs: `PinManagerClass`, `BusConfig`
- **UPPER_CASE** for macros and constants: `WLED_MAX_USERMODS`, `DEFAULT_CLIENT_SSID`

## Header Guards

Most headers use `#ifndef` / `#define` guards. Some newer headers add `#pragma once` before the guard:

```cpp
#ifndef WLED_EXAMPLE_H
#define WLED_EXAMPLE_H
// ...
#endif // WLED_EXAMPLE_H
```

## Comments

- `//` for inline comments, `/* ... */` for block comments. Always put a space after `//`
- Mark WLED-MM-specific changes with `// WLEDMM` or `// WLEDMM: description`:

```cpp
// WLEDMM: increased max bus count for larger installs
#ifndef WLED_MAX_BUSSES
  #define WLED_MAX_BUSSES 20  // WLEDMM default (upstream: 10)
#endif
```

## Preprocessor & Feature Flags

- Prefer compile-time feature flags (`#ifdef` / `#ifndef`) over runtime checks where possible
- Platform differentiation: `ARDUINO_ARCH_ESP32` vs `ESP8266`
- WLED-MM fork detection: `_MoonModules_WLED_` (defined in `wled.h`)
- PSRAM availability: `BOARD_HAS_PSRAM`, `WLED_USE_PSRAM`
- Flash-saving mode: `WLEDMM_SAVE_FLASH` (disables aggressive inlining)

## Error Handling

- No C++ exceptions — use return codes (`-1` / `false` for errors) and global flags (e.g. `errorFlag = ERR_LOW_MEM`)
- `DEBUG_PRINTF()` / `DEBUG_PRINTLN()` for developer diagnostics (compiled out unless `WLED_DEBUG`)
- `USER_PRINTF()` / `USER_PRINTLN()` for user-visible messages (always compiled in)

## Strings

- Use `F("string")` for string constants (stores in PROGMEM, saves RAM)
- Use `const char*` for temporary/parsed strings
- Avoid `String` (Arduino heap-allocated string) in hot paths; acceptable in config/setup code

## Memory

- PSRAM-aware allocation: use `d_malloc()` (prefer DRAM), `p_malloc()` (prefer PSRAM) from `util.h`
- Larger buffers (LED data, JSON documents) should use PSRAM when available
- Memory efficiency matters, but is less critical on boards with PSRAM

---

## Hot-Path Optimization

The hot path is the per-frame pixel pipeline: **Segment → Strip → BusManager → BusDigital → PolyBus → LED driver**. Speed is the top priority here. The patterns below are taken from existing hot-path code (`FX_fcn.cpp`, `FX_2Dfcn.cpp`, `bus_manager.cpp`, `colorTools.hpp`) and should be followed when modifying these files.

Note: `FX.cpp` (effect functions) is written by many contributors and has diverse styles — that is acceptable. The guidelines below apply starting from pixel set/get operations and below.

### Function Attributes

Stack the appropriate attributes on hot-path functions. Defined in `const.h`:

| Attribute | Meaning | When to use |
|---|---|---|
| `__attribute__((hot))` | Branch-prediction hint | All hot-path functions |
| `IRAM_ATTR` | Place in fast IRAM (ESP32) | Critical per-pixel functions (e.g. `BusDigital::setPixelColor`) |
| `IRAM_ATTR_YN` | IRAM on ESP32, no-op on ESP8266 | Hot functions that ESP8266 can't fit in IRAM |
| `WLED_O2_ATTR` | Force `-O2` optimization | Most hot-path functions |
| `WLED_O3_ATTR` | Force `-O3,fast-math` | Innermost color math (e.g. `color_blend`) |
| `[[gnu::hot]] inline` | Modern C++ attribute + inline | Header-defined accessors (e.g. `progress()`, `currentBri()`) |

Example signature:

```cpp
void IRAM_ATTR_YN WLED_O2_ATTR __attribute__((hot)) Segment::setPixelColor(int i, uint32_t col)
```

### Use `uint_fast` Types for Locals

Use `uint_fast8_t` and `uint_fast16_t` for loop counters, indices, and temporary calculations in hot paths. These let the compiler pick the CPU's native word size (32-bit on ESP32), avoiding unnecessary narrow-type masking:

```cpp
uint_fast8_t count = numBusses;
for (uint_fast8_t i = 0; i < count; i++) { ... }
```

Keep `uint8_t` / `uint16_t` for struct fields and stored data where memory layout matters.

### Cache Members to Locals Before Loops

Copy class members and virtual-call results to local variables before entering a loop:

```cpp
uint_fast8_t count = numBusses;          // avoid repeated member access
for (uint_fast8_t i = 0; i < count; i++) {
  Bus* const b = busses[i];              // const pointer hints to compiler
  uint_fast16_t bstart = b->getStart();
  uint_fast16_t blen = b->getLength();
  ...
}
```

### Unsigned Range Check

Replace two-comparison range tests with a single unsigned subtraction:

```cpp
// Instead of: if (pix >= bstart && pix < bstart + blen)
if ((uint_fast16_t)(pix - bstart) < blen)  // also catches negative pix via unsigned underflow
```

### Early Returns

Guard every hot-path function with the cheapest checks first:

```cpp
if (!isActive()) return;                    // inactive segment
if (unsigned(i) >= virtualLength()) return; // bounds check (catches negative i too)
```

### Avoid Nested Calls — Fast Path / Complex Path

Avoid calling non-inline functions or making complex decisions inside per-pixel hot-path code. When a function has both a common simple case and a rare complex case, split it into two variants and choose once per frame rather than per pixel:

```cpp
// Decision made once per frame in startFrame(), stored in a bool
bool simpleSegment = _isSuperSimpleSegment;

// Per-pixel loop — no complex branching inside
if (simpleSegment)
  setPixelColorXY_fast(x, y, col, scaled_col, cols, rows);  // inline, no bounds checks
else
  setPixelColorXY_slow(x, y, col);  // full validation, grouping, mirroring
```

The same principle applies to color utilities — `color_add()` accepts a `fast` flag so callers can choose saturating adds (no branches) vs. ratio-preserving adds (with division) without an inner-loop decision:

```cpp
uint32_t color_add(uint32_t c1, uint32_t c2, bool fast=false);
```

General rules:
- Keep the per-pixel fast path free of non-inline function calls and multi-way branches
- Hoist the "which path?" decision out of the inner loop (once per frame or per segment)
- It is acceptable to duplicate some code between fast and complex variants to keep the fast path lean

### Pre-Compute Outside Loops

Move invariant calculations before the loop. Pre-compute reciprocals to replace division with multiplication:

```cpp
const uint_fast16_t cols = virtualWidth();
const uint_fast16_t rows = virtualHeight();
uint_fast8_t fadeRate = (255 - rate) >> 1;
float mappedRate_r = 1.0f / (float(fadeRate) + 1.1f);  // reciprocal — avoid division inside loop
```

### Parallel Channel Processing

Process R+B and W+G channels simultaneously using the two-channel mask pattern:

```cpp
constexpr uint32_t TWO_CHANNEL_MASK = 0x00FF00FF;
uint32_t rb = (((c1 & TWO_CHANNEL_MASK) * amount) >> 8) & TWO_CHANNEL_MASK;
uint32_t wg = (((c1 >> 8) & TWO_CHANNEL_MASK) * amount) & ~TWO_CHANNEL_MASK;
return rb | wg;
```

### Bit Shifts Over Division

Prefer bit shifts for power-of-two operations:

```cpp
position >> 3     // instead of position / 8
(255 - rate) >> 1 // instead of (255 - rate) / 2
i & 0x0007        // instead of i % 8
```

### Static Caching for Expensive Computations

Cache results in static locals when the input rarely changes between calls:

```cpp
static uint16_t lastKelvin = 0;
static byte correctionRGB[4] = {255,255,255,0};
if (lastKelvin != kelvin) {
  colorKtoRGB(kelvin, correctionRGB);  // expensive — only recalculate when input changes
  lastKelvin = kelvin;
}
```

### Inlining Strategy

- Move frequently-called small functions to headers for inlining (e.g. `WS2812FX::setPixelColor` is in `FX.h`)
- Use `static inline` for file-local helpers
- On ESP32 with `WLEDMM_FASTPATH`, color utilities are inlined from `colorTools.hpp`; on ESP8266 or `WLEDMM_SAVE_FLASH`, they fall back to `colors.cpp`

### Colors

- Store and pass colors as `uint32_t` (0xWWRRGGBB)
- Extract channels with macros: `R(c)`, `G(c)`, `B(c)`, `W(c)`, compose with `RGBW32(r,g,b,w)`
- Use `CRGB` (FastLED type) only when interfacing with FastLED functions; convert at boundaries
- Use 16-bit intermediates for channel math to ensure 32-bit (not 64-bit) arithmetic:
  ```cpp
  uint16_t r1 = R(color1);  // 16-bit ensures 32-bit multiply on ESP32
  ```

## Multi-Task Synchronization

ESP32 runs multiple FreeRTOS tasks concurrently (e.g. network handling, LED output, JSON parsing). Use the WLED-MM mutex macros for synchronization — they expand to FreeRTOS recursive semaphore calls on ESP32 and compile to no-ops on ESP8266:

| Macro | Signature | Description |
|---|---|---|
| `esp32SemTake(mux, timeout)` | `mux`: `SemaphoreHandle_t`, `timeout`: milliseconds | Acquire a recursive mutex. Returns `pdTRUE` on success, `pdFALSE` on timeout. |
| `esp32SemGive(mux)` | `mux`: `SemaphoreHandle_t` | Release a previously acquired recursive mutex. |

Pre-defined mutex handles (declared in `wled.h`):

| Mutex | Protects |
|---|---|
| `busDrawMux` | Concurrent `strip.show()` and `strip.service()` — acquire before writing pixels from background tasks (DDP, E1.31, Art-Net) |
| `segmentMux` | Segment array modifications — acquire before adding, removing, or iterating segments |
| `jsonBufferLockMutex` | Shared JSON document buffer |
| `presetFileMux` | `presets.json` file reads and writes |

Usage pattern:

```cpp
if (esp32SemTake(busDrawMux, 200) == pdTRUE) { // wait max 200 ms
  // ... critical section ...
  esp32SemGive(busDrawMux);
}
```

Always pair every `esp32SemTake` with a matching `esp32SemGive`. Choose a timeout appropriate for the operation — typically 200 ms for drawing, up to 2500 ms for file I/O.

Not every shared resource needs a mutex. Some synchronization is guaranteed by the overall control flow. For example, `volatile bool` flags like `suspendStripService`, `doInitBusses`, `loadLedmap`, and `OTAisRunning` (declared in `wled.h`) are checked sequentially in the main loop (`wled.cpp`), so they serialize access without requiring a semaphore. Use mutexes when true concurrent access from multiple FreeRTOS tasks is possible. Rely on control-flow ordering when operations are sequenced within the same loop iteration.

## General

- Follow the existing style in the file you are editing
- Include `"wled.h"` as the primary project header where needed
