#!/bin/bash
#
# load.sh — Insert the memory_hook kernel module
#
set -euo pipefail

MODULE="kernel-module/memory_hook.ko"

if [ ! -f "$MODULE" ]; then
    echo "[-] Module not found. Build it first:"
    echo "    cd kernel-module && make"
    exit 1
fi

echo "[+] Loading memory_hook.ko..."
sudo insmod "$MODULE"

echo "[✓] Module loaded. Check dmesg for details:"
echo "    sudo dmesg | tail -5"
