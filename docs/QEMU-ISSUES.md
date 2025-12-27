# QEMU ESP32 Testing - Known Issues and Limitations

## Build Configuration

**Important**: QEMU testing uses the **V4 Mainline ethernet debug build** (`esp32_16MB_V4_M_eth_debug`).

### Why esp32_16MB_V4_M_eth_debug Build?
- **16MB Flash**: Provides sufficient space for all features and debugging symbols
- WiFi hardware is not emulated in QEMU
- WiFi initialization causes crashes in QEMU
- Ethernet build uses `WLED_USE_ETHERNET` flag
- Disables ESP-NOW with `WLED_DISABLE_ESPNOW` (requires WiFi)
- Uses ESP32-POE board configuration (`WLED_ETH_DEFAULT=2`)
- Allows network functionality without WiFi hardware
- HTTP server works via emulated ethernet (open_eth model)
- Debug build provides better crash analysis capabilities

**Flash Size Configuration**: The QEMU setup creates a 16MB flash image to match the build requirements. This is configured in `run-qemu.sh`.

### Ethernet Configuration for QEMU
The build uses ESP32-POE board configuration (index 2):
- **PHY Address**: 0
- **Power Pin**: 12
- **MDC Pin**: 23
- **MDIO Pin**: 18
- **PHY Type**: LAN8720
- **Clock Mode**: GPIO17_OUT

This configuration is compatible with QEMU's `open_eth` model, which emulates standard ESP32 RMII ethernet interface.

### QEMU Ethernet Hardware Workaround
**Critical**: The build includes `WLED_QEMU` flag which skips actual ethernet hardware initialization (`ETH.begin()`). This is necessary because:
- QEMU's `open_eth` model doesn't fully emulate all ethernet MAC hardware registers
- Calling `ETH.begin()` crashes with `LoadStorePIFAddrError` in `emac_ll_clock_enable_rmii_output`
- The crash occurs when trying to enable RMII clock output (hardware register access at 0x3ff6980c)
- With `WLED_QEMU` defined, the code skips hardware init but marks ethernet as configured
- Network stack still functions via QEMU's user-mode networking (slirp)
- HTTP server and web UI work without actual hardware initialization

**For real hardware**: Remove the `WLED_QEMU` flag - it should only be used for QEMU testing.

### Network Configuration in QEMU
QEMU's user-mode networking (slirp) provides:
- **DHCP Server**: Built-in DHCP server (default network 10.0.2.0/24)
  - Guest IP: 10.0.2.15 (assigned via DHCP)
  - Gateway: 10.0.2.2
  - DNS: 10.0.2.3
- **Port Forwarding**: TCP port 80 on guest → port 8080 on host (localhost:8080)

**DHCP vs Static IP:**
- WLED normally uses DHCP on ethernet
- QEMU provides a DHCP server by default
- If DHCP doesn't work (connection issues), enable static IP in platformio.ini:
  ```
  -D WLED_STATIC_IP_DEFAULT_1=10
  -D WLED_STATIC_IP_DEFAULT_2=0
  -D WLED_STATIC_IP_DEFAULT_3=2
  -D WLED_STATIC_IP_DEFAULT_4=15
  ```
- Static IP 10.0.2.15 matches QEMU's default guest IP assignment

## QEMU Limitations

ESP32 QEMU emulation is not perfect and has several known limitations:

### Network Configuration
- **DHCP**: QEMU provides a built-in DHCP server (10.0.2.0/24 network)
- **Expected behavior**: ESP32 should receive IP 10.0.2.15 via DHCP
- **If DHCP fails**: Enable static IP in platformio.ini (see Build Configuration above)
- **Port forwarding**: HTTP port 80 on ESP32 → localhost:8080 on host

### Hardware Emulation
- **WiFi**: Not emulated - **causes crashes if enabled**
- **Bluetooth**: Not emulated
- **I2C/SPI**: Limited emulation - some peripherals may not work
- **GPIO**: Partial emulation - LED outputs and some inputs work, but not all
- **ADC**: Not emulated
- **Touch sensors**: Not emulated
- **RTC**: Limited emulation
- **Ethernet**: Emulated via open_eth model (used for testing)

### Common Crash Patterns

#### 1. WiFi-Related Crashes
**Symptom**: Crashes when trying to initialize WiFi or connect to networks  
**Cause**: WiFi hardware is not fully emulated in QEMU  
**Analysis**: Check if crash occurs during WiFi initialization  
**Solution**: Use ethernet build (`esp32_4MB_M_eth`) which disables WiFi

#### 2. Peripheral Access Crashes
**Symptom**: Crashes when accessing I2C, SPI, or other peripherals  
**Cause**: Peripheral emulation is incomplete  
**Analysis**: Check which peripheral is being accessed in the backtrace  
**Solution**: These may be QEMU-specific issues, use ethernet debug build for better diagnostics

#### 3. Real Firmware Bugs
**Symptom**: Crashes in application code (not hardware access)  
**Cause**: Actual bugs in WLED firmware  
**Analysis**: Look for null pointers, stack overflows, buffer overruns  
**Solution**: These should be fixed in the firmware

## Analyzing Crashes

### Common Exception Types
ESP32 exceptions with EXCCAUSE codes:
- `0x00000000` (IllegalInstruction): Executing invalid code
- `0x00000001` (Syscall): Syscall instruction
- `0x00000002` (InstructionFetchError): Cannot fetch instruction
- `0x00000003` (LoadStoreError): Load/store alignment error
- `0x00000005` (LoadStoreAlignmentCause): Load/store alignment error
- `0x00000006` (InstructionDataError): Data error during instruction fetch
- `0x00000007` (LoadStoreDataError): Data error during load/store
- `0x00000009` (LoadStorePrivilegeViolation): Privilege violation
- `0x0000000f` (LoadStorePIFAddrError): Invalid PIF address (common in QEMU)
- `0x0000001c` (InstructionAddrError): Address error during instruction fetch
- `0x0000001d` (LoadStoreAddrError): Address error during load/store
- `0x0000001e` (InstructionBusError): Bus error during instruction fetch
- `0x0000001f` (LoadStoreBusError): Bus error during load/store

### LoadStorePIFAddrError (0x0000000f)
This is **very common in QEMU** and usually indicates:
- Accessing hardware registers not emulated by QEMU
- Accessing invalid memory-mapped peripheral addresses
- Often occurs during peripheral initialization (I2C, SPI, ADC, etc.)
- **May work fine on real hardware** - QEMU limitation

### Decoding Crash Backtraces

When you see a crash like:
```
Guru Meditation Error: Core 1 panic'ed (LoadStorePIFAddrError)
Backtrace: 0x401771aa:0x3ffb2090 0x4015b4c5:0x3ffb20c0 ...
```

#### Method 1: Using PlatformIO Exception Decoder
```bash
# In the WLED-MM directory
pio device monitor --filter esp32_exception_decoder

# Then paste the exception output (registers + backtrace)
# The decoder will show function names and file locations
```

#### Method 2: Using ESP-IDF addr2line
```bash
# Install toolchain (if not already from PlatformIO)
~/.platformio/packages/toolchain-xtensa-esp32/bin/xtensa-esp32-elf-addr2line \
  -pfiaC -e .pio/build/esp32_16MB_V4_M_eth_debug/firmware.elf \
  0x401771aa 0x4015b4c5 0x40134813 0x40103cd0 0x40135d33 0x401383c6 0x4016107e
```

Replace the addresses with those from your backtrace.

#### Method 3: Online Decoder
1. Get firmware.elf from build artifacts
2. Use https://github.com/me-no-dev/EspExceptionDecoder
3. Paste exception info and upload firmware.elf
4. Get decoded stack trace

### Example Decoded Output
```
0x401771aa: emac_hal_init at components/hal/esp32/emac_hal.c:45
0x4015b4c5: esp_eth_mac_esp32_init at components/esp_eth/src/esp_eth_mac_esp32.c:123
0x40134813: NetworkClass::begin at wled00/network.cpp:234
```

This shows the crash occurred in ethernet MAC initialization - likely a QEMU emulation limitation.

### Analyzing the Crash Location

1. **Check the function names**: Are they in hardware/peripheral code?
   - `emac_`, `i2c_`, `spi_`, `adc_`, etc. → Likely QEMU limitation
   - Application functions → Likely real bug

2. **Check EXCVADDR**: The address being accessed
   - `0x3ff69xxx` range → Peripheral registers (QEMU issue)
   - `0x00000000` or very low → Null pointer (real bug)
   - Stack addresses → Possible stack overflow

3. **Check PC (Program Counter)**: Where code was executing
   - ROM addresses (`0x4000xxxx`) → ESP32 ROM functions
   - Flash addresses (`0x400dxxxx - 0x4017xxxx`) → Your firmware
   - RAM addresses (`0x4008xxxx`) → RAM-loaded code

### Common QEMU-Specific Crashes

#### Ethernet MAC Initialization
```
Backtrace: ... esp_eth_mac_esp32_init ... emac_hal_init ...
```
**Cause**: QEMU's ethernet emulation may not fully support all MAC features  
**Action**: Check if ethernet link comes up; web server may still work

#### I2C/SPI Peripheral Access
```
Backtrace: ... i2c_master_cmd_begin ... 
```
**Cause**: I2C peripherals not emulated  
**Action**: Expected in QEMU; disable or mock peripheral access

#### WiFi Functions
```
Backtrace: ... esp_wifi_init ... wifi_hw_init ...
```
**Cause**: WiFi not emulated  
**Action**: Use ethernet build (already configured)

### Expected Behavior in QEMU
For WLED testing in QEMU, we expect:
- ✅ Web server to start successfully
- ✅ HTTP requests to be handled
- ✅ Web UI pages to load
- ✅ Basic ethernet connectivity
- ⚠️ WiFi operations to fail/be disabled
- ⚠️ Some LED control features may not work fully
- ⚠️ Peripheral access (I2C, SPI) may crash
- ⚠️ Some hardware features cause QEMU-specific crashes

### Investigating Crashes

1. **Download QEMU logs** from GitHub Actions artifacts
2. **Find the exception** in qemu-output.log
3. **Copy the backtrace addresses**
4. **Decode using one of the methods above**
5. **Analyze the decoded output**:
   - Hardware access? → Probably QEMU limitation
   - Application logic? → Likely real bug to fix
   - Initialization code? → May need QEMU workaround

See full QEMU logs in GitHub Actions artifacts (`qemu-logs`).
