#!/bin/bash
# unload_module.sh — Unload the memory_hook kernel module
# SPDX-License-Identifier: MIT
set -e

echo "Unloading memory_hook module..."
sudo rmmod memory_hook

echo "Module unloaded. Original sys_write restored."
echo "Check kernel messages:"
sudo dmesg | tail -3 | grep -i memory_hook || echo "(no recent memory_hook messages)"

echo ""
echo "Verification:"
echo "  echo '42' | ./userspace-c/read_number"
echo "  (should now print 42, not 15)"
