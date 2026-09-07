# fs_data (PlatformIO data_dir)

This folder is used as a minimal PlatformIO filesystem payload (`data_dir`) for builds that use `buildfs/uploadfs`.

## Why it exists
In some setups (e.g. ESP32 4MB flash with smaller FS partitions), building a filesystem image from the default `wled00/data` content can exceed the available FS size and fail with errors like:
- `SPIFFS_write error: File system is full`

To avoid that, `fs_data` contains only small config placeholders (e.g. `cfg.json`, `presets.json`) so filesystem builds remain reproducible on limited partitions.

## What to put here
- `cfg.json` (optional, can be `{}`)
- `presets.json` (optional, can be `{}`)
- `wsec.json` (optional) — **do not commit secrets** (WiFi credentials). Keep it empty (`{}`) or manage locally.

## How it is used
Typically enabled via `platformio_override.ini`, e.g.:
- `data_dir = ${PROJECT_DIR}/fs_data`

This keeps upstream files untouched while allowing local/CI builds to succeed.