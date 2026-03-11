#!/usr/bin/env bash
set -euo pipefail

RULES_SRC="$(cd "$(dirname "$0")" && pwd)/99-esp32-serial.rules"
RULES_DST="/etc/udev/rules.d/99-esp32-serial.rules"

if [[ ! -f "$RULES_SRC" ]]; then
  echo "Missing rules file: $RULES_SRC" >&2
  exit 1
fi

if ! id -nG "$USER" | grep -qw dialout; then
  echo "Adding user '$USER' to dialout group..."
  sudo usermod -aG dialout "$USER"
  echo "You must log out/in after this change."
fi

echo "Installing udev rules..."
sudo cp "$RULES_SRC" "$RULES_DST"
sudo udevadm control --reload-rules
sudo udevadm trigger

echo "Disabling ModemManager to prevent serial-port locking..."
sudo systemctl disable --now ModemManager || true

# Load common USB-serial drivers if available.
for mod in usbserial cp210x ch341 ftdi_sio; do
  sudo modprobe "$mod" || true
done

echo
echo "Done. Unplug/replug the ESP32 board and check available ports with:"
echo "  ls -l /dev/ttyUSB* /dev/ttyACM* 2>/dev/null || true"
echo "Then in VS Code run: ESP-IDF: Select Port"
