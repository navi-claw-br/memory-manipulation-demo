#!/bin/bash
# setup.sh — Install dependencies and compile all components
# SPDX-License-Identifier: MIT
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

echo "============================================"
echo "  Memory Manipulation Demo — Setup"
echo "============================================"
echo ""

# --- Install dependencies ---
echo "=== Step 1: Installing system dependencies ==="
if command -v apt-get &>/dev/null; then
    sudo apt-get update
    sudo apt-get install -y \
        build-essential \
        linux-headers-$(uname -r) \
        gcc \
        make \
        default-jdk \
        default-jre \
        dkms \
        git
elif command -v dnf &>/dev/null; then
    sudo dnf groupinstall -y "Development Tools"
    sudo dnf install -y \
        kernel-devel \
        kernel-headers \
        gcc \
        make \
        java-17-openjdk \
        java-17-openjdk-devel \
        git
elif command -v pacman &>/dev/null; then
    sudo pacman -S --noconfirm \
        base-devel \
        linux-headers \
        gcc \
        make \
        jdk-openjdk \
        git
else
    echo "WARNING: Unknown package manager. Install dependencies manually."
    echo "Required: build-essential, linux-headers, gcc, make, java (jdk+jre)"
fi

echo ""
echo "=== Step 2: Compiling Kernel Module ==="
cd "$PROJECT_DIR/kernel-module"
make clean
make
echo "  -> memory_hook.ko built successfully"

echo ""
echo "=== Step 3: Compiling C Program ==="
cd "$PROJECT_DIR/userspace-c"
make clean
make
echo "  -> read_number built successfully"

echo ""
echo "=== Step 4: Compiling Java Program ==="
cd "$PROJECT_DIR/userspace-java"
javac ReadNumber.java 2>/dev/null || {
    echo "  WARNING: javac not found. Install JDK and re-run."
}
echo "  -> ReadNumber.class built successfully"

echo ""
echo "============================================"
echo "  Setup complete!"
echo "============================================"
echo ""
echo "Next steps:"
echo "  ./scripts/load_module.sh    — Load the kernel module"
echo "  ./scripts/unload_module.sh  — Unload the kernel module"
echo ""
echo "  # Test C version with module loaded:"
echo "  echo '42' | ./userspace-c/read_number"
echo ""
echo "  # Test Java version with module loaded:"
echo "  echo '42' | java -cp userspace-java ReadNumber"
