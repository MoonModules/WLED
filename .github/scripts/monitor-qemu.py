#!/usr/bin/env python3
"""
Monitor QEMU ESP32 serial output and decode exceptions
This script watches the QEMU serial output and uses the ESP32 exception decoder
to translate stack traces into human-readable format.
"""

import sys
import re
import subprocess
import os

def find_elf_file(firmware_dir):
    """Find the ELF file for symbol resolution"""
    elf_path = os.path.join(firmware_dir, "firmware.elf")
    if os.path.exists(elf_path):
        return elf_path
    return None

def decode_exception(lines, elf_file):
    """Decode an ESP32 exception using addr2line"""
    if not elf_file or not os.path.exists(elf_file):
        return None
    
    # Extract addresses from backtrace
    addresses = []
    for line in lines:
        # Look for patterns like: 0x4008xxxx:0x3ffbxxxx
        matches = re.findall(r'0x[0-9a-fA-F]{8}', line)
        addresses.extend(matches)
    
    if not addresses:
        return None
    
    # Use addr2line to decode addresses
    try:
        # Get the toolchain path from environment or use default
        toolchain_prefix = os.environ.get('TOOLCHAIN_PREFIX', 'xtensa-esp32-elf-')
        addr2line = f"{toolchain_prefix}addr2line"
        
        cmd = [addr2line, '-e', elf_file, '-f', '-C'] + addresses
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=5)
        
        if result.returncode == 0 and result.stdout:
            return result.stdout
    except Exception as e:
        print(f"[Decoder] Error decoding: {e}", file=sys.stderr)
    
    return None

def monitor_output(firmware_dir):
    """Monitor stdin and decode exceptions"""
    elf_file = find_elf_file(firmware_dir)
    
    if elf_file:
        print(f"[Decoder] Using ELF file: {elf_file}", file=sys.stderr)
    else:
        print(f"[Decoder] Warning: ELF file not found in {firmware_dir}", file=sys.stderr)
        print(f"[Decoder] Exception decoding will not be available", file=sys.stderr)
    
    exception_lines = []
    in_exception = False
    
    for line in sys.stdin:
        # Print the original line
        print(line, end='', flush=True)
        
        # Detect exception start
        if 'Guru Meditation Error' in line or 'Backtrace:' in line or 'abort()' in line:
            in_exception = True
            exception_lines = [line]
            print("\n[Decoder] ========== ESP32 EXCEPTION DETECTED ==========", file=sys.stderr)
        elif in_exception:
            exception_lines.append(line)
            
            # Check if exception block ended
            if line.strip() == '' or 'ELF file SHA256' in line or len(exception_lines) > 20:
                # Try to decode
                decoded = decode_exception(exception_lines, elf_file)
                if decoded:
                    print("\n[Decoder] Decoded stack trace:", file=sys.stderr)
                    print(decoded, file=sys.stderr)
                    print("[Decoder] ================================================\n", file=sys.stderr)
                else:
                    print("[Decoder] Could not decode exception (toolchain not available)", file=sys.stderr)
                    print("[Decoder] ================================================\n", file=sys.stderr)
                
                in_exception = False
                exception_lines = []

if __name__ == '__main__':
    firmware_dir = sys.argv[1] if len(sys.argv) > 1 else '.pio/build/esp32_4MB_V4_S_eth'
    monitor_output(firmware_dir)
