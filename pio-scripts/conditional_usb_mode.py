#!/usr/bin/env python3
"""
Conditional USB Mode Script for WLED-MM

This script automatically manages ARDUINO_USB_MODE based on the build context:
- For development builds: ARDUINO_USB_MODE=1 (allows USB debugging)  
- For release builds (CI): ARDUINO_USB_MODE is removed (prevents hanging, allows normal boot)

The script detects release builds by checking for the WLED_RELEASE environment variable
which is set to True in the GitHub Actions CI workflow.

CRITICAL: This change only applies to boards with USB-OTG (ARDUINO_USB_CDC_ON_BOOT=1).
For boards with classical UART-to-USB chips (ARDUINO_USB_CDC_ON_BOOT=0), 
ARDUINO_USB_MODE=1 is harmless and left unchanged.

IMPORTANT: We remove ARDUINO_USB_MODE entirely for release builds rather than setting 
it to 0, because ARDUINO_USB_MODE=0 breaks Serial functionality when CDC_ON_BOOT=1.
When ARDUINO_USB_MODE is undefined, the framework uses appropriate defaults.
"""
## This script was created with the help of an AI, reviewed and tested by @softhack007

Import('env')
import os

def has_cdc_on_boot_enabled(env):
    """
    Check if ARDUINO_USB_CDC_ON_BOOT is set to 1 in the build configuration.
    
    Returns True if CDC_ON_BOOT=1, False otherwise.
    This is used to identify boards with USB-OTG (native USB) vs UART-to-USB chips.
    """
    build_unflags = env.get('BUILD_UNFLAGS', [])
    build_flags = env.get('BUILD_FLAGS', [])
    cpp_defines = env.get('CPPDEFINES', [])

    # Check in raw build unflags - unflags have priority over flags
    for unflag in build_unflags:
        if isinstance(unflag, str) and 'ARDUINO_USB_CDC_ON_BOOT=1' in unflag:
            return False
    
    # Check in CPPDEFINES
    for define in cpp_defines:
        if isinstance(define, (list, tuple)) and len(define) == 2:
            if define[0] == 'ARDUINO_USB_CDC_ON_BOOT' and define[1] == '1':
                return True
    
    # # Check in raw build flags - results can be misleading, because this ignores build_unflags
    for flag in build_flags:
        if isinstance(flag, str) and 'ARDUINO_USB_CDC_ON_BOOT=1' in flag:
            return True
    
    return False

def has_usb_mode_enabled(env):
    """
    Check if ARDUINO_USB_MODE is set to 1 in the build configuration.
    
    Returns True if USB_MODE=1, False otherwise.
    """
    build_unflags = env.get('BUILD_UNFLAGS', [])
    build_flags = env.get('BUILD_FLAGS', [])
    cpp_defines = env.get('CPPDEFINES', [])
    # Check in raw build unflags - unflags have priority over flags
    for unflag in build_unflags:
        if isinstance(unflag, str) and 'ARDUINO_USB_MODE=1' in unflag:
            return False
    
    # Check in CPPDEFINES
    for define in cpp_defines:
        if isinstance(define, (list, tuple)) and len(define) == 2:
            if define[0] == 'ARDUINO_USB_MODE' and define[1] == '1':
                return True
    
    # Check in raw build flags - results can be misleading, because this ignores build_unflags
    for flag in build_flags:
       if isinstance(flag, str) and 'ARDUINO_USB_MODE=1' in flag:
           return True
    
    return False

def conditional_usb_mode(env):
    """
    Conditionally manage ARDUINO_USB_MODE based on build context.
    
    For ESP32-C3, ESP32-S2, and ESP32-S3 variants with USB-OTG (CDC_ON_BOOT=1):
    - Development builds: ARDUINO_USB_MODE=1 (default, good for debugging)
    - Release builds: ARDUINO_USB_MODE removed (prevents hanging, preserves Serial functionality)
    
    For boards with classical UART-to-USB chip (CDC_ON_BOOT=0):
    - ARDUINO_USB_MODE=1 is harmless and left unchanged
    
    Note: We remove the flag entirely rather than setting to 0, because 
    ARDUINO_USB_MODE=0 breaks Serial functionality with CDC_ON_BOOT=1.
    """
    
    # Check if this is a release build (CI sets WLED_RELEASE=True)
    is_release_build = os.environ.get('WLED_RELEASE', '').lower() in ('true', '1', 'yes')
    
    if is_release_build:
        # Check if this board uses USB-OTG (CDC_ON_BOOT=1)
        if not has_cdc_on_boot_enabled(env):
            print("WLED Release build detected - board uses UART-to-USB chip (CDC_ON_BOOT=0)")
            print("  Keeping ARDUINO_USB_MODE=1 (harmless for UART-to-USB boards)")
            return
        
        print("WLED Release build detected - board uses USB-OTG (CDC_ON_BOOT=1)")
        print("  Removing ARDUINO_USB_MODE definition for production")
        
        # Check if ARDUINO_USB_MODE=1 is present
        if has_usb_mode_enabled(env):
            # Remove ARDUINO_USB_MODE entirely - don't set it to 0 as that breaks Serial
            # When undefined, the framework uses appropriate defaults based on CDC_ON_BOOT
            env.Append(BUILD_UNFLAGS=["-DARDUINO_USB_MODE=1"])
            print(f"  Removed ARDUINO_USB_MODE definition (was 1)")
            
    else:
        # Development build
        has_usb_mode = has_usb_mode_enabled(env)
        has_cdc_boot = has_cdc_on_boot_enabled(env)
        
        if has_usb_mode and has_cdc_boot:
            print("Development build detected - keeping ARDUINO_USB_MODE=1 for USB-OTG debugging")
            # Warning in orange/yellow using ANSI color codes
            print("\033[93m  WARNING: This build is NOT suitable for production devices!\033[0m")
            print("\033[93m  Production builds require WLED_RELEASE=True environment variable.\033[0m")
        elif has_cdc_boot:
            # CDC_ON_BOOT=1 present but not USB_MODE=1 
            print("Development build detected - USB-OTG enabled, but ARDUINO_USB_MODE=1 missing for debugging")
        elif has_usb_mode:
            # USB_MODE=1 present but not CDC_ON_BOOT=1 (UART-to-USB board)
            print("Development build detected - board uses UART-to-USB chip")
        # If neither flag is present, don't print anything

# Apply the conditional USB mode logic
conditional_usb_mode(env)
