# memory-manipulation-demo

This project demonstrates **runtime memory manipulation** at the kernel level by hooking the `read()` syscall to intercept and modify user-space input — specifically replacing any typed number with the fixed value `15`.

## Concept

When you type a number and press Enter in a terminal program:
1. The terminal sends the input to the program via the `read()` syscall.
2. A kernel module intercepts `read()`, checks if the input is numeric, and if so, replaces the buffer content with `"15\n"` before passing it back to user space.
3. The user-space program sees `15` regardless of what was typed.

This is a simplified demonstration of techniques used in:
- Rootkits / security research
- Runtime dynamic analysis
- Kernel-level debugging and instrumentation

## Suggested Distributions

| Distribution | Version |
|-------------|---------|
| **Fedora** | 41+ |
| **Ubuntu** | 24.04+ |

The kernel hook uses standard APIs (`kallsyms_lookup_name`, `write_cr0`/`read_cr0`) available on x86_64 Linux.

> ⚠️ **Warning:** This module disables the CR0 Write-Protect bit to modify the syscall table. This is inherently unsafe and bypasses kernel protection mechanisms. Use only in isolated test environments (VMs or dedicated test boxes).

## Project Structure

```
memory-manipulation-demo/
├── kernel-module/
│   ├── memory_hook.c      # Kernel module: hooks read() syscall
│   └── Makefile            # Build system
├── userspace-c/
│   └── read_number.c       # C program demonstrating the effect
├── userspace-java/
│   └── ReadNumber.java     # Java program demonstrating the effect
├── scripts/
│   ├── setup.sh            # Install dependencies
│   ├── load.sh             # Insert the module
│   └── unload.sh           # Remove the module
└── README.md               # This file
```

## How to Test

### 1. Install Dependencies

```bash
./scripts/setup.sh
```

### 2. Build the Kernel Module

```bash
cd kernel-module && make
```

### 3. Build the User-Space Programs

```bash
# C version
gcc -o ../userspace-c/read_number ../userspace-c/read_number.c

# Java version
javac -d ../userspace-java/ ../userspace-java/ReadNumber.java
```

### 4. Test Without the Module (Baseline)

```bash
echo 42 | ./userspace-c/read_number
# Expected output:
# Type a number and press Enter: You entered: 42
```

### 5. Load the Module and Retest

```bash
./scripts/load.sh
echo 42 | ./userspace-c/read_number
# Expected output (with module):
# Type a number and press Enter: You entered: 15
```

### 6. Verify Kernel Logs

```bash
sudo dmesg | tail -10
# Expected:
# memory_hook: intercepted numeric input '42', replacing with '15\n'
```

### 7. Unload

```bash
./scripts/unload.sh
```

## How It Works

1. The module uses `kallsyms_lookup_name("sys_call_table")` to locate the kernel's syscall table.
2. It clears the CR0 Write-Protect bit to make the table writable.
3. It saves the original `read()` syscall pointer and replaces it with the hook.
4. The hook calls the original `read()`, then inspects the returned buffer.
5. If the buffer contains only digits (plus a newline), it overwrites the buffer with `"15\n"`.

## Security Considerations

- **Do not run on production systems.**
- This technique is detected by security systems (SELinux, AppArmor, kernel lockdown).
- You may need to boot with `enforcing=0` or disable lockdown on some distributions.
- The module is **not cryptographically signed** — Secure Boot must be disabled.

## License

GPL v2 — This module uses GPL-licensed kernel symbols.
