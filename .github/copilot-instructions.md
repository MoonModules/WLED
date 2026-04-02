# WLED-MM — ESP32/ESP8266 LED Controller Firmware

WLED is a fast, feature-rich ESP32/ESP8266 webserver for controlling NeoPixel (WS2812B, WS2811, SK6812) LEDs and SPI-based chipsets. WLED-MM is a fork focused on higher performance (ESP32, ESP32-S3, PSRAM boards), large installs, and advanced audio analysis.

## Setup

- Node.js 20+ (see `.nvmrc`)
- Install dependencies: `npm ci`
- PlatformIO (required only for firmware compilation): `pip install -r requirements.txt`

## Build and Test

| Command | Purpose | Typical Time |
|---|---|---|
| `npm run build` | Build web UI → generates `wled00/html_*.h` headers | ~3 s |
| `npm test` | Run test suite | ~40 s |
| `npm run dev` | Watch mode — auto-rebuilds web UI on file changes | — |
| `pio run -e <env>` | Build firmware for a hardware target | 15–20 min |

**Always run `npm run build` before `pio run`.** The web UI build generates `wled00/html_*.h` header files required by firmware compilation.

Common firmware environments: `esp32_4MB_V4_M`, `esp32_16MB_V4_S_HUB75`, `esp32S3_8MB_PSRAM_M_qspi`, `esp32_16MB_V4_M_eth`, `esp8266_4MB_S`

For detailed build timeouts, development workflows, troubleshooting, and validation steps, see [agent-build-instructions.md](agent-build-instructions.md).

## Repository Structure

```
wled00/                 # Firmware source (C++)
  ├── data/             # Web UI source (HTML, CSS, JS)
  ├── src/              # Core modules, fonts, dependencies
  ├── html_*.h          # Auto-generated (DO NOT EDIT OR COMMIT)
  └── wled.h            # Main firmware configuration
usermods/               # Community addons (.h files, included via usermods_list.cpp)
tools/cdata.js          # Web UI → header build script
tools/cdata-test.js     # Test suite
platformio.ini          # Build targets and configuration
package.json            # Node.js scripts and release ID
```

Main development branch: `mdev`

## General Guidelines

- **Never edit or commit** `wled00/html_*.h` — auto-generated from `wled00/data/`.
- **Repository language is English.** Suggest translations for non-English content.
- **When unsure, say so.** Gather more information rather than guessing.
- **PR reviews:** Authors do not need to commit `html_*.h` files; they are generated during firmware builds.
- **Provide references** when making analyses or recommendations. Base them on the correct branch or PR.
- No automated linting is configured — match existing code style in files you edit. See `cpp.instructions.md` and `web.instructions.md` for language-specific conventions.
