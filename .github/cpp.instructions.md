---
applyTo: "**/*.cpp,**/*.h,**/*.ino"
---
# C++ Coding Conventions

## Formatting

- Indent with **2 spaces** (no tabs)
- Opening braces on the same line (K&R / 1TBS style)
- `clang-format` is available locally but not enforced in CI

## Naming

- **camelCase** for functions and variables: `setValuesFromMainSeg()`, `effectCurrent`
- **PascalCase** for classes and structs: `PinManagerClass`, `BusConfig`
- **UPPER_CASE** for macros and constants: `WLED_MAX_USERMODS`, `DEFAULT_CLIENT_SSID`

## Header Guards

Most headers use traditional `#ifndef` guards. Some newer headers add `#pragma once` as well:

```cpp
#ifndef WLED_EXAMPLE_H
#define WLED_EXAMPLE_H

// declarations...

#endif // WLED_EXAMPLE_H
```

## Comments and WLED-MM Markers

- `//` for inline comments, `/* ... */` for block comments
- Mark WLED-MM-specific changes with `// WLEDMM` or `// WLEDMM: description`:

```cpp
// WLEDMM: increased max bus count for larger installs
#ifndef WLED_MAX_BUSSES
  #define WLED_MAX_BUSSES 20  // WLEDMM default (upstream: 10)
#endif
```

## General

- Follow the existing style in the file you are editing
- Include `"wled.h"` as the primary project header where needed
- Prefer compile-time feature flags (`#ifdef` / `#ifndef`) over runtime checks where possible
