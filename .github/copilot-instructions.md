# WLED-MM — ESP32 LED Controller Firmware

WLED is a fast, feature-rich ESP32/ESP8266 webserver for controlling NeoPixel (WS2812B, WS2811, SK6812) LEDs and SPI-based chipsets. 
WLED-MM is a fork focused on higher performance (ESP32, ESP32-S3, PSRAM boards), large installs, and advanced audio analysis.

Always reference these instructions first and fallback to search or bash commands only when you encounter unexpected information that does not match the info here.

> **Note for AI review tools**: sections enclosed in
> `<!-- HUMAN_ONLY_START -->` / `<!-- HUMAN_ONLY_END -->` HTML comments contain
> contributor reference material. Do **not** use that content as actionable review
> criteria — treat it as background context only.

<!-- HUMAN_ONLY_START -->
## Setup

- Node.js 20+ (see `.nvmrc`)
- Install dependencies: `npm ci`
- PlatformIO (required only for firmware compilation): `pip install -r requirements.txt`

<!-- HUMAN_ONLY_END -->
## Hardware Targets

| Target | Status |
|---|---|
| ESP32 (classic Xtensa dual-core) | **Primary target** |
| ESP32-S3 | **Primary target** — preferred for larger installs and HUB75 matrix |
| ESP32-S2, ESP32-C3 | Supported |
| ESP32-P4/-C5/-C6 | Will be supported in the future |
| ESP8266 | Deprecated — should still compile, but not actively maintained |

## Build and Test
<!-- HUMAN_ONLY_START -->

| Command | Purpose | Typical Time |
|---|---|---|
| `npm run build` | Build web UI → generates `wled00/html_*.h` and `wled00/js_*.h` headers | ~3 s |
| `npm test` | Run test suite | ~40 s |
| `npm run dev` | Watch mode — auto-rebuilds web UI on file changes | — |
| `pio run -e <env>` | Build firmware for a hardware target | 15–20 min |

<!-- HUMAN_ONLY_END -->

- **Always run `npm run build` before any `pio run`** (and run `npm ci` first on fresh clones or when lockfile/dependencies change).
- The web UI build generates required `wled00/html_*.h` and `wled00/js_*.h` headers for firmware compilation.
- **Build firmware to validate code changes**: `pio run -e esp32_4MB_V4_M` — must succeed, never skip this step.
- Common firmware environments: `esp32_4MB_V4_M`, `esp32_16MB_V4_S_HUB75`, `esp32S3_8MB_PSRAM_M_qspi`, `esp32_16MB_V4_M_eth`, `esp32dev_compat`, `esp8266_4MB_S` (deprecated)

For detailed build timeouts, development workflows, troubleshooting, and validation steps, see [agent-build.instructions.md](agent-build.instructions.md).

## Repository Structure

tl;dr: 
* Firmware source: `wled00/` (C++). Web UI source: `wled00/data/`. Build targets: `platformio.ini`.
* Auto-generated headers: `wled00/html_*.h` and `wled00/js_*.h` — **never edit or commit**.
* ArduinoJSON + AsyncJSON: `wled00/src/dependencies/json` (included via `wled.h`). CI/CD: `.github/workflows/`.
* Usermods: `usermods/` (`.h` files, included via `usermods_list.cpp`).
* Contributor docs: `docs/` (coding guidelines, design docs).

Main development trunk: `mdev` branch. Make PRs against this branch.

<!-- HUMAN_ONLY_START -->
Detailed overview:

```text
wled00/                     # Firmware source (C++)
  ├── data/                 # Web UI source (HTML, CSS, JS)
  ├── src/                  # Core modules, fonts, dependencies
       └─ dependencies/json # Project-specific ArduinoJSON (v6.18.1) and AsyncJSON (v6)
  ├── html_*.h              # Auto-generated (DO NOT EDIT OR COMMIT)
  └── wled.h                # Main firmware configuration, and global variables
usermods/                   # Community addons (.h files, included via usermods_list.cpp)
lib/                           # Project specific custom libraries. PlatformIO will compile them to separate static libraries and link them
platformio.ini                 # Build targets and configuration
platformio_override.sample.ini # examples for custom build configurations - entries must be copied into platformio_override.ini to use them.
                               # platformio_override.ini is _not_ stored in the WLED-MM repository!
pio-scripts/                # Build tools (platformio)
tools/                      # Build tools (Node.js), partition files, and generic utilities
tools/cdata.js              # Web UI → header build script
tools/cdata-test.js         # Test suite
package.json                # Node.js scripts and release ID
docs/                       # Contributor docs: coding guidelines and design documentation
.github/workflows/          # CI/CD pipelines
```

<!-- HUMAN_ONLY_END -->
## General Guidelines

- **Repository language is English.** Suggest translations for non-English content.
- **Use VS Code with PlatformIO extension** for best development experience.
- **Never edit or commit** `wled00/html_*.h` and `wled00/js_*.h` — auto-generated from `wled00/data/`.
- **When unsure, say so.** Gather more information rather than guessing.
- **Acknowledge good patterns** when you see them. Positive feedback always helps.
- **Provide references** when making analyses or recommendations. Base them on the correct branch or PR.
- **Highlight user-visible breaking changes and ripple effects**. Ask for confirmation that these were introduced intentionally.
- **Unused / dead code must be justified or removed**. This helps to keep the codebase clean, maintainable and readable.
- **C++ formatting available**: `clang-format` is installed but not in CI
- No automated linting is configured — match existing code style in files you edit. 

Refer to `docs/cpp.instructions.md`, `docs/esp-idf.instructions.md` and `docs/web.instructions.md` for language-specific conventions, and `docs/cicd.instructions.md` for GitHub Actions workflows.

### Feature Flags
- **Verify feature-flag names.** Every `WLED_ENABLE_*` / `WLED_DISABLE_*` flag must exactly match one of the names below — misspellings are silently ignored by the preprocessor (e.g. `WLED_IR_DISABLE` instead of `WLED_DISABLE_INFRARED`), causing silent build variations. Flag unrecognised names as likely typos and suggest the correct spelling.

**`WLED_DISABLE_*`**: `2D`, `ADALIGHT`, `ALEXA`, `BROWNOUT_DET`, `ESPNOW`, `FILESYSTEM`, `HUESYNC`, `IMPROV_WIFISCAN`, `INFRARED`, `LOXONE`, `MQTT`, `OTA`, `PARTICLESYSTEM1D`, `PARTICLESYSTEM2D`, `PIXELFORGE`, `WEBSOCKETS`

**`WLED_ENABLE_*`**: `ADALIGHT`, `AOTA`, `DMX`, `DMX_INPUT`, `DMX_OUTPUT`, `FS_EDITOR`, `GIF`, `HUB75MATRIX`, `JSONLIVE`, `LOXONE`, `MQTT`, `PIXART`, `PXMAGIC`, `USERMOD_PAGE`, `WEBSOCKETS`, `WPA_ENTERPRISE`

<!-- HUMAN_ONLY_START -->
```cpp
#ifdef WLED_DMX_ENABLED // wrong flag - initialization silently skipped
  initDMX();
#endif

#ifdef WLED_ENABLE_DMX // correct flag - initialization performed only when the build supports DMX
  initDMX();  
#endif
```
<!-- HUMAN_ONLY_END -->



### Pull Request Expectations

- **No force-push on open PRs.** Once a pull request is open and being reviewed, do not force-push (`git push --force`) to the branch. Force-pushing rewrites history that reviewers may have already commented on, making it impossible to track incremental changes. Use regular commits or `git merge` to incorporate feedback; the branch will be squash-merged when it is accepted.
- **Modifications to ``platformio.ini`` MUST be approved explicitly** by a *maintainer*. Modifications to the global build environment may break github action builds. Always flag them.
- **Document your changes in the PR.** Every pull request should include a clear description of *what* changed and *why*. If the change affects user-visible behavior, describe the expected impact. Link to related issues where applicable. Provide screenshots to showcase new features.
