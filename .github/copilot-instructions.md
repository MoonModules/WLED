# WLED-MM — ESP32/ESP8266 LED Controller Firmware

WLED is a fast, feature-rich ESP32/ESP8266 webserver for controlling NeoPixel (WS2812B, WS2811, SK6812) LEDs and SPI-based chipsets. WLED-MM is a fork focused on higher performance (ESP32, ESP32-S3, PSRAM boards), large installs, and advanced audio analysis.

## Setup

- Node.js 20+ (see `.nvmrc`)
- Install dependencies: `npm ci`
- PlatformIO for firmware builds: `pip install -r requirements.txt`

## Build and Test

| Command | Purpose | Typical Time | Timeout |
|---|---|---|---|
| `npm run build` | Build web UI → generates `wled00/html_*.h` headers | ~3 s | 30 s |
| `npm test` | Run test suite | ~40 s | 2 min |
| `npm run dev` | Watch mode — auto-rebuilds web UI on file changes | — | — |
| `pio run -e <env>` | Build firmware for a hardware target | 15–20 min | 30 min |

**Always run `npm run build` before `pio run`.** The web UI build generates `wled00/html_*.h` header files required by firmware compilation. Never cancel long-running builds.

Common firmware environments: `esp32_4MB_V4_M`, `esp32_16MB_V4_S_HUB75`, `esp32S3_8MB_PSRAM_M_qspi`, `esp32_16MB_V4_M_eth`, `esp8266_4MB_S`

## Before Finishing Work

Complete **all** of these before marking work done:

1. `npm test` — must pass
2. `pio run -e esp32_4MB_V4_M` — must succeed (set timeout ≥ 30 min, never cancel)
3. For web UI changes: manually test the interface

If any step fails, fix the issue before proceeding.

### Manual Web UI Testing

```sh
cd wled00/data && python3 -m http.server 8080
# Open http://localhost:8080/index.htm
```

Verify: page loads without JS errors, navigation works, color picker and brightness controls function, effects and settings forms submit correctly.

## Repository Structure

```
wled00/                 # Firmware source (C++)
  ├── data/             # Web UI source (HTML, CSS, JS)
  ├── src/              # Core modules, fonts, dependencies
  ├── html_*.h          # Auto-generated (DO NOT EDIT OR COMMIT)
  └── wled.h            # Main firmware configuration
usermods/               # Community addons (C++, with library.json)
tools/cdata.js          # Web UI → header build script
tools/cdata-test.js     # Test suite
platformio.ini          # Build targets and configuration
package.json            # Node.js scripts and release ID
```

Main development branch: `mdev`

## CI/CD

On every push and PR, CI will:
1. Install dependencies (Node.js, Python)
2. Run `npm test`
3. Build web UI (automatic via PlatformIO)
4. Compile firmware for **all** `default_envs` targets

Ensure `npm test` and at least one `pio run -e <env>` succeed locally before pushing.

## Troubleshooting

- **Missing `html_*.h`:** Run `npm run build`
- **Broken web UI:** Check browser console for JS errors
- **PlatformIO network errors:** Retry — downloads can be flaky
- **Node.js version mismatch:** Ensure Node.js 20+ (check `.nvmrc`)
- **Force web UI rebuild:** `npm run build -- -f`
- **Clean PlatformIO cache:** `pio run --target clean`
- **Reinstall Node deps:** `rm -rf node_modules && npm ci`

## General Guidelines

- **Never edit or commit** `wled00/html_*.h` — auto-generated from `wled00/data/`.
- **Repository language is English.** Suggest translations for non-English content.
- **When unsure, say so.** Gather more information rather than guessing.
- **PR reviews:** Authors do not need to commit `html_*.h` files; they are generated during firmware builds.
- **Provide references** when making analyses or recommendations. Base them on the correct branch or PR.
- No automated linting is configured — match existing code style in files you edit. See `cpp.instructions.md` and `web.instructions.md` for language-specific conventions.
