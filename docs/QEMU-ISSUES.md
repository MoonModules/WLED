# QEMU ESP32 Testing - Known Issues and Limitations

## Build Configuration

**Important**: QEMU testing uses the **V4 Sound Reactive ethernet build** (`esp32_4MB_V4_S_eth`).

### Why esp32_4MB_V4_S_eth Build?
- WiFi hardware is not emulated in QEMU
- WiFi initialization causes crashes in QEMU
- Ethernet build uses `WLED_USE_ETHERNET` flag
- Disables ESP-NOW with `WLED_DISABLE_ESPNOW` (requires WiFi)
- Uses ESP32-POE board configuration (`WLED_ETH_DEFAULT=2`)
- Allows network functionality without WiFi hardware
- HTTP server works via emulated ethernet (open_eth model)

### Ethernet Configuration for QEMU
The build uses ESP32-POE board configuration (index 2):
- **PHY Address**: 0
- **Power Pin**: 12
- **MDC Pin**: 23
- **MDIO Pin**: 18
- **PHY Type**: LAN8720
- **Clock Mode**: GPIO17_OUT

This configuration is compatible with QEMU's `open_eth` model, which emulates standard ESP32 RMII ethernet interface.

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
**Solution**: These may be QEMU-specific issues

#### 3. Real Firmware Bugs
**Symptom**: Crashes in application code (not hardware access)  
**Cause**: Actual bugs in WLED firmware  
**Analysis**: Look for null pointers, stack overflows, buffer overruns  
**Solution**: These should be fixed in the firmware

## Analyzing Crashes

### Check Exception Type
Common ESP32 exceptions:
- `LoadProhibited`: Reading from invalid memory address
- `StoreProhibited`: Writing to invalid memory address  
- `IllegalInstruction`: Executing invalid code

### Expected Behavior in QEMU
For WLED testing in QEMU, we expect:
- ✅ Web server to start successfully
- ✅ HTTP requests to be handled
- ✅ Web UI pages to load
- ⚠️ WiFi operations to fail/be limited
- ⚠️ Some LED control features may not work fully

See full QEMU logs in GitHub Actions artifacts.
