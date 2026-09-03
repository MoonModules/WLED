# M5Stack Core S3 Display Usermod

Display usermod for the ILI9342C 320x240 TFT display on the M5Stack Core S3, using LovyanGFX.

**NOTE:** The M5Stack Core S3 has 8MB of PSRAM, but it's uncommonly **QSPI PSRAM**. This was likely a design tradeoff to free up more pins for add-on modules over using octal PSRAM which blocks off more pins. 

## Pin Mapping (M5Stack Core S3)

| ESP32-S3 | ILI9342C | Description     |
|----------|----------|-----------------|
| G37      | MOSI     | SPI Data        |
| G36      | SCLK     | SPI Clock       |
| G3       | CS       | Chip Select     |
| G35      | DC       | Data/Command    |

Reset is controlled via the AW9523B GPIO expander (P1_1). Backlight is powered via AXP2101 PMU (DLDO1).

## Building

In `platformio_override.ini` for your M5Stack Core S3 environment:

```ini
build_flags =
    -D USERMOD_M5STACK_CORE_S3_DISPLAY
    ;; For the M5Stack ModuleAudio:
    -D SR_ENABLE_DEFAULT
    -D SR_DMTYPE=6
    -D I2S_SDPIN=13
    -D I2S_WSPIN=6
    -D I2S_CKPIN=0
    -D MCLK_PIN=7
    -D HW_SDA_PIN=12
    -D HW_SCL_PIN=11

lib_deps =
    https://github.com/lovyan03/LovyanGFX
```

## Features

- SSID and IP address in header bar
- 16-band graphic equalizer bars (differential drawing)
- Real audio reactive data when Audioreactive usermod is enabled
- Simulated bouncing bars when no audio data
- Rainbow color per bar (red → violet)
- Maximum of 100 FPS to match AudioReactive
- Minimum of 5 FPS so it updates even if you use unlimited FPS mode.

## Display Notes

- Uses LovyanGFX with `SPI3_HOST` (HSPI)
- Native landscape 320x240 resolution
- RGB color order, display inversion enabled
- Backlight always on (controlled by AXP2101 DLDO1)

## TroyHacks Recommended AudioReactive Settings

For the M5Stack Core S3 + ModuleAudio line-in:

- The ModuleAudio uses an ES8388
- Squelch & Gain at 1
- AGC is Normal
- MicLev is Freeze
- Mic Quality is Perfect
- FFT Window is Nutall (or whatever you prefer)
- Profile is Generic Line-In
- Limiter is Enabled, with a rise of 1 and a fall of 250 (to 500).
