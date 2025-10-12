Import("env")
import psutil

def kill_platformio_monitor():
    for proc in psutil.process_iter(['pid', 'name', 'cmdline']):
        try:
            if "platformio" in proc.info['name'].lower() and "device-monitor" in " ".join(proc.info['cmdline']):
                print(f"Killing monitor process PID {proc.info['pid']}")
                proc.kill()
        except (psutil.NoSuchProcess, psutil.AccessDenied):
            continue

kill_platformio_monitor()

