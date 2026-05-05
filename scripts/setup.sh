#!/bin/bash
#
# setup.sh — Install dependencies for the memory-manipulation-demo project
#
# Detects the distribution (Fedora or Ubuntu) and installs the required
# packages: kernel development headers, GCC, Java, and make.
#

set -euo pipefail

echo "=== memory-manipulation-demo: Setup ==="

detect_distro() {
    if [ -f /etc/fedora-release ]; then
        echo "fedora"
    elif [ -f /etc/lsb-release ] && grep -qi ubuntu /etc/lsb-release 2>/dev/null; then
        echo "ubuntu"
    elif [ -f /etc/debian_version ]; then
        echo "debian"
    else
        echo "unknown"
    fi
}

install_fedora() {
    echo "[+] Detected Fedora. Installing packages..."
    sudo dnf install -y \
        kernel-devel \
        kernel-headers \
        gcc \
        make \
        java-17-openjdk \
        java-17-openjdk-devel \
        elfutils-libelf-devel \
        git
}

install_ubuntu() {
    echo "[+] Detected Ubuntu/Debian. Installing packages..."
    sudo apt-get update
    sudo apt-get install -y \
        linux-headers-$(uname -r) \
        gcc \
        make \
        default-jdk \
        git
}

distro=$(detect_distro)

case "$distro" in
    fedora)
        install_fedora
        ;;
    ubuntu|debian)
        install_ubuntu
        ;;
    *)
        echo "[-] Unsupported distribution. Please install manually:"
        echo "    - kernel development headers"
        echo "    - gcc, make"
        echo "    - Java (JDK 17+)"
        exit 1
        ;;
esac

echo ""
echo "[✓] Setup complete. Run 'cd kernel-module && make' to build the module."
