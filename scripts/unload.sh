#!/bin/bash
#
# unload.sh — Remove the memory_hook kernel module
#
set -euo pipefail

MODULE="memory_hook"

echo "[+] Unloading $MODULE..."
sudo rmmod "$MODULE"

echo "[✓] Module unloaded."
