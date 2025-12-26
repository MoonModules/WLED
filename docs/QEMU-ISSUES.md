# QEMU ESP32 Testing - Known Issues and Limitations

## QEMU Limitations

ESP32 QEMU emulation is not perfect and has several known limitations:

### Hardware Emulation
- **WiFi**: Not fully emulated - WiFi operations may fail or behave differently
- **Bluetooth**: Not emulated
- **I2C/SPI**: Limited emulation - some peripherals may not work
- **GPIO**: Partial emulation - LED outputs and some inputs work, but not all
- **ADC**: Not emulated
- **Touch sensors**: Not emulated
- **RTC**: Limited emulation

### Common Crash Patterns

#### 1. WiFi-Related Crashes
**Symptom**: Crashes when trying to initialize WiFi or connect to networks  
**Cause**: WiFi hardware is not fully emulated in QEMU  
**Analysis**: Check if crash occurs during WiFi initialization  
**Solution**: These are expected in QEMU and don't indicate real firmware bugs

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
