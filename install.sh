#!/bin/bash
# Loic Andre (C) 2025
# This script will install cm5_monitor utility in /opt/cm5_monitor and create a systemd service to start at boot.
# File lists :
# - cm5_monitor.py	-> /opt/cm5_monitor/
# - leds_ctrl (C app)	-> /opt/cm5_monitor/
# - cm5_monitor.service -> /etc/systemd/system/

echo "cm5_monitor installer, Loic Andre (c)2025"
echo "This script will install cm5_monitor utility in /opt/cm5_monitor and start service"

# Check if the script is running as root
if [ "$EUID" -ne 0 ]; then
  echo "Please run this script with sudo."
  exit 1
fi

echo "Prepare to build leds_ctrl app"
echo "leds_ctrl: Create cmake environement"
cmake .

echo "leds_ctrl: make"
if ! make; then
  echo "leds_ctrl: Error during make, check manually, Exiting.."
  exit 1
fi

if [ ! -e "./leds_ctrl" ]; then
  echo "led_ctrl: file not found"
  exit 1
fi

echo "leds_ctrl: make sucessful"

if [ ! -e "./cm5_monitor.py" ]; then
  echo "cm5_monitor.py: file not found"
  exit 1
fi

if [ ! -e "./cm5_monitor.service" ]; then
  echo "cm5_monitor.service: file not found"
  exit 1
fi

if [ ! -e "/opt/cm5_monitor" ]; then
  echo "Creating /opt/cm5_monitor folder"
  mkdir /opt/cm5_monitor
else
  echo "/opt/cm5_monitor already exists"
fi

echo "copiying to /opt/cm5_monitor.py"
cp ./cm5_monitor.py /opt/cm5_monitor

echo "copiying to /opt/leds_ctrl"
cp ./leds_ctrl /opt/cm5_monitor

echo "creating service in /etc/systemd/system/cm5_monitor.service"
cp ./cm5_monitor.service /etc/systemd/system/

systemctl daemon-reload

if ! systemctl enable cm5_monitor.service; then
  echo "cm5_monitor.service : service failed to be enabled"
  exit 1
fi

if ! systemctl start cm5_monitor.service; then
  echo "cm5_monitor.service: service failed to be started"
fi

if systemctl is-active --quiet "cm5_monitor.service"; then
  echo "service cm5_monitor.service successfully started"
fi

echo "CM5_monitor successfully installed ! enjoyy :)"
