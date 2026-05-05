# Kernel Module — `memory_hook`

## 📋 Overview

This kernel module demonstrates **memory manipulation from kernel space**.

It hooks the `sys_write` syscall by modifying the **system call table** (`sys_call_table`). When a userspace program writes a 1- or 2-digit number to stdout or stderr, the module intercepts the write call, modifies the buffer content in kernel memory, and writes a fixed replacement value (default: `15`) instead.

## 🧠 How It Works

1. **Symbol resolution via kprobe**: On modern kernels (5.7+), `kallsyms_lookup_name` is no longer exported. We use a `kprobe` to dynamically locate its address, then use it to find `sys_call_table`.

2. **Syscall table manipulation**: The module temporarily disables the CR0 write-protection bit (bit 16), replaces the `sys_write` entry in the syscall table with our hook function, then re-enables write protection.

3. **Buffer inspection**: In the hook, we:
   - Check if the write is to stdout (fd=1) or stderr (fd=2)
   - Copy the user-space buffer to kernel space
   - Strip trailing newlines and check if it's a 1- or 2-digit number
   - Replace the buffer content with our fixed value
   - Copy the modified buffer back to user space

4. **Clean restoration**: On module unload, the original `sys_write` is restored.

## ⚙️ Module Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `replace_value` | ulong | 15 | Value that replaces detected numbers in output |

*Example: `sudo insmod memory_hook.ko replace_value=99`*

## 🔧 Build & Load

```bash
# Build
make

# Load (normal output → becomes "15\n")
sudo insmod memory_hook.ko

# Load with custom replacement value
sudo insmod memory_hook.ko replace_value=99

# Unload
sudo rmmod memory_hook

# Check kernel logs
sudo dmesg | tail -20
```

## ⚠️ Security Notes

- This module modifies the syscall table at runtime — it can crash your system if something goes wrong.
- **Only run in a VM or dedicated test environment.**
- Do NOT use in production systems.
- The CR0 write-protection trick is architecture-dependent (x86/x86_64 only).

## 🧪 Test

```bash
# Terminal 1: watch kernel messages
sudo dmesg -w

# Terminal 2: load module and test
sudo insmod memory_hook.ko
echo "42" | ./userspace-c/read_number
# Expected output: 15  (not 42!)
```

## 🔗 References

- [Linux kernel syscall table](https://elixir.bootlin.com/linux/latest/source/arch/x86/entry/syscalls/syscall_64.tbl)
- [Kprobes documentation](https://www.kernel.org/doc/html/latest/trace/kprobes.html)
- [CR0 WP bit — x86 memory protection](https://en.wikipedia.org/wiki/Control_register#CR0)
