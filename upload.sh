#!/bin/bash
# Upload script for Arduino Uno R4 WiFi using arduino-cli.
# arduino-cli handles the port re-enumeration correctly.
# Usage: ./upload.sh <firmware.bin>

set -e

FIRMWARE="$1"

if [ ! -f "$FIRMWARE" ]; then
  echo "Error: firmware file not found: $FIRMWARE"
  exit 1
fi

# Check nothing is holding the port
if lsof /dev/cu.usbmodem* 2>/dev/null | grep -q usbmodem; then
  echo "Error: another process has the serial port open. Close it first."
  lsof /dev/cu.usbmodem* 2>/dev/null
  exit 1
fi

PORT=$(ls /dev/cu.usbmodem* 2>/dev/null | head -1)
if [ -z "$PORT" ]; then
  echo "Error: no Arduino found. Is it plugged in?"
  exit 1
fi

echo "Uploading to $PORT..."
arduino-cli upload -p "$PORT" --fqbn arduino:renesas_uno:unor4wifi --input-file "$FIRMWARE"
