# Userspace C — `read_number`

## 📋 Overview

A minimal C program that reads a 1- or 2-digit number from stdin and prints it.

This program is the **target** of the kernel module's memory manipulation — when `memory_hook` is loaded, the output will be silently replaced with the fixed value (default: `15`).

## 🔧 Build

```bash
make
```

## 🚀 Usage

```bash
# Run interactively
./read_number
# -> Digite um numero de ate 2 digitos: 42
# -> Numero lido: 42

# Or pipe input
echo "42" | ./read_number
# -> Numero lido: 42
```

## 🧪 Test Without Module

```bash
echo "42" | ./read_number
# Output: Numero lido: 42
```

## 🧪 Test With Module

```bash
# Terminal 1: watch kernel messages
sudo dmesg -w

# Terminal 2:
sudo insmod ../kernel-module/memory_hook.ko
echo "42" | ./read_number
# Output: Numero lido: 15   ← manipulated!
```

## 💡 What's Happening

The program calls `printf("Numero lido: %d\n", 42)`, which eventually calls `write(1, "Numero lido: 42\n", ...)`. The kernel module intercepts this `write()` syscall, detects the "42" in the buffer, replaces it with "15", then returns the modified buffer to userspace.

The C program has no idea this happened — it just prints whatever `printf` writes.
