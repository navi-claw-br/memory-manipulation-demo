# Userspace Java — `ReadNumber`

## 📋 Overview

A minimal Java program that reads a 1- or 2-digit number from stdin and prints it.

Same behavior as the C version — proof that the kernel module manipulation **works across languages** because the interception happens at the syscall level, not at the language runtime level.

## 🔧 Build

```bash
# Compile
javac ReadNumber.java

# Or use the build script
./build.sh
```

## 🚀 Usage

```bash
java ReadNumber
# -> Digite um numero de ate 2 digitos: 42
# -> Numero lido: 42
```

## 🧪 Test Without Module

```bash
echo "42" | java ReadNumber
# Output: Numero lido: 42
```

## 🧪 Test With Module

```bash
sudo insmod ../kernel-module/memory_hook.ko
echo "42" | java ReadNumber
# Output: Numero lido: 15   ← manipulated!
```

## 💡 Why This Matters

Both C and Java programs exhibit the same behavior under the kernel module because:

1. **Java** → `System.out.println()` → native `write()` syscall → kernel
2. **C** → `printf()` → `write()` syscall → kernel

The hook intercepts **after** both runtimes, at the OS level. This demonstrates that memory manipulation via kernel module is **language-agnostic** and **transparent to userspace**.
