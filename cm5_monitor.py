
import os
import time
import threading
import subprocess
import psutil
from datetime import datetime

internet = False
eth_connected = False
wifi_connected = False
cpu_overtemp = -1.0
cpu_overload = False
memory_full = False
swap_Use = False
undervoltage = False
error = False

def log(msg):
    msg
    print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] {msg}")

# --- Every minutes ---
def check_every_minutes():
    global swap_Use, memory_full, cpu_overload
    while True:
        #cpu load
        load1, load5, load15 = os.getloadavg()
        load = load1 / os.cpu_count()
        if load > 0.85:
            cpu_overload = True
        else:
            cpu_overload = False
        log(f"🧠 CPU Load (1m): {load:.2f} {'⚠️ High' if cpu_overload else '✅ OK'}")

        # Memory usage
        mem = psutil.virtual_memory()
        mem_usage_percent = mem.percent
        if mem_usage_percent > 90:
            memory_full = True
        else:
            memory_full = False
        log(f"🧠 Memory Usage: {mem_usage_percent:.1f}% {'⚠️ High' if mem_usage_percent > 90 else '✅ OK'}")

        # Swap usage
        swap = psutil.swap_memory()
        swap_mb = swap.used / (1024 * 1024)
        if swap_mb > 50:
            swap_Use = True
        else:
            swap_Use = False
        log(f"📦 Swap Used: {swap_mb:.1f} MB {'⚠️ Used' if swap_mb > 50 else '✅ Not Used'}")
        



        time.sleep(60)


# --- Start Threads ---
threading.Thread(target=check_every_minutes, daemon=True).start()

# Keep main thread alive
while True:

    # Internet check
    internet = os.system("ping -c 1 -W 2 8.8.8.8 > /dev/null 2>&1") == 0
    log(f"🌐 Internet: {'Connected' if internet else 'Disconnected'}")



    # Ethernet check
    if os.path.exists(r"/sys/class/net/eth0/carrier"):
        try:
            with open(r"/sys/class/net/eth0/carrier", "r") as f:
                eth_connected = f.read().strip() == "1"
        except:
            eth_connected = False
    else:
            eth_connected = False
    log(f"🔌 Ethernet: {'Connected' if eth_connected else 'Disconnected'}")

    # Wi-Fi check
    if os.path.exists(r"/sys/class/net/wlan0/carrier"):
        try:
            with open(r"/sys/class/net/wlan0/carrier", "r") as f:
                wifi_connected = f.read().strip() == "1"
        except:
            wifi_connected = False
    else:
            wifi_connected = False
    log(f"📶 Wi-Fi: {'Connected' if wifi_connected else 'Disconnected'}")

    # CPU temperature
    try:
        with open(r"/sys/class/thermal/thermal_zone0/temp", "r") as f:
            cpu_temp = int(f.read()) / 1000.0
        if cpu_temp >= 70 :
            cpu_overtemp = True
        else:
            cpu_overtemp = False
        log(f"🌡️ CPU Temp: {cpu_temp:.1f}°C {'✅ OK' if not cpu_overtemp else '⚠️ High'}")
    except:
        log("🌡️ CPU Temp: Unable to read")

    # under-voltage
    try:
        dmesg_output = subprocess.check_output("dmesg | grep -i 'voltage' | tail -10", shell=True).decode()
        if "under-voltage" in dmesg_output.lower():
            undervoltage = True
            log("🔋 Power Warning: ⚠️ Under-voltage detected")
        else:
            undervoltage = False
            log("🔋 Power Warning: ✅ No recent warning")
    except:
        log("🔋 Power Warning: Unable to check")

    # set network led : 
    if not wifi_connected and not eth_connected and not internet:
        led_net= "r" # 🔴 Red
    elif wifi_connected and not eth_connected and not internet:
        led_net= "v" # 🟣 Violet
    elif not wifi_connected and eth_connected and not internet:
        led_net= "y" # 🟡 Yellow
    elif wifi_connected and eth_connected and not internet:
        led_net= "o" # 🟠 Orange
    elif wifi_connected and not eth_connected and internet:
        led_net= "b" # 🔵 Blue
    elif not wifi_connected and eth_connected and internet:
        led_net= "g" # 🟢 Green
    elif wifi_connected and eth_connected and internet:
        led_net= "g" # 🟢 Green
    elif not wifi_connected and not eth_connected and internet:
        led_net= "w" # ⚪ White

    log("mem="+str(memory_full)+", swap="+str(swap_Use)+", ovrTmp="+str(cpu_overtemp)+", cpuL="+str(cpu_overload)+", oV="+str(undervoltage)+", error="+ str(error) )
    if not memory_full and not swap_Use and not cpu_overtemp and not cpu_overload and not undervoltage and not error:
        led_stat= "g" # 🟢 Green
    else:
        if memory_full or swap_Use:
            led_stat= "v" # 🟣 Violet
        elif cpu_overload:
            led_stat= "o" # 🟠 Orange
        elif cpu_overtemp or undervoltage or error:
            led_stat= "r" # 🔴 Red
        else:
            led_stat= "x"

    os.system("/opt/cm5_monitor/leds_ctrl -s " + led_stat + " -n " + led_net)
    
    time.sleep(2)
