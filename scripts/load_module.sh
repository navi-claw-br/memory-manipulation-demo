#!/bin/bash
# load_module.sh — Load the memory_hook kernel module
# SPDX-License-Identifier: MIT
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
MODULE_PATH="$SCRIPT_DIR/../kernel-module/memory_hook.ko"

if [ ! -f "$MODULE_PATH" ]; then
    echo "ERROR: Module not found. Run setup.sh first."
    echo "  $MODULE_PATH"
    exit 1
fi

# Optional: accept replacement value as first argument
REPLACE_VAL="${1:-15}"

echo "Loading memory_hook module with replace_value=$REPLACE_VAL ..."
sudo insmod "$MODULE_PATH" replace_value="$REPLACE_VAL"

echo "Module loaded! Check kernel messages:"
sudo dmesg | tail -5 | grep -i memory_hook || sudo dmesg | tail -5

echo ""
echo "Test it:"
echo "  echo '42' | ./userspace-c/read_number"
echo "  echo '42' | java -cp userspace-java ReadNumber"
echo ""
echo "Watch kernel messages: sudo dmesg -w"
