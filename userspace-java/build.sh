#!/bin/bash
# build.sh — Compile and run the Java ReadNumber demo
# SPDX-License-Identifier: MIT

set -e

echo "=== Compiling ReadNumber.java ==="
javac ReadNumber.java
echo "OK: ReadNumber.class created"

echo ""
echo "=== Running (no module) ==="
echo "42" | java ReadNumber

echo ""
echo "=== Running with piped input ==="
echo "7" | java ReadNumber
