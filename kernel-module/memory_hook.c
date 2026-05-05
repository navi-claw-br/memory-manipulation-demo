// SPDX-License-Identifier: GPL-2.0
/*
 * memory_hook.c — Kernel module that hooks the read() syscall
 * to intercept numeric input and replace it with the fixed value 15.
 *
 * WARNING: This is a demonstration/educational module only.
 * Hooking syscalls at runtime bypasses kernel safety mechanisms.
 * Do NOT use in production.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kallsyms.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/version.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Navi");
MODULE_DESCRIPTION("Demo: hook read() syscall to replace numeric input with 15");
MODULE_VERSION("0.1");

/* Pointers to the original and the syscall table entry */
static unsigned long *syscall_table;
static asmlinkage long (*original_read)(unsigned int fd, char __user *buf, size_t count);

/* The hook function */
asmlinkage long hook_read(unsigned int fd, char __user *buf, size_t count)
{
    long ret;
    char *kbuf;
    int i;

    /* Call the original read() first */
    original_read = (void *)syscall_table[__NR_read];
    ret = original_read(fd, buf, count);

    /* Only intercept stdin (fd == 0) and successful reads */
    if (fd == 0 && ret > 0) {
        /* Allocate kernel buffer */
        kbuf = kmalloc(ret + 1, GFP_KERNEL);
        if (!kbuf)
            return ret;

        /* Copy user data to kernel space */
        if (copy_from_user(kbuf, buf, ret)) {
            kfree(kbuf);
            return ret;
        }
        kbuf[ret] = '\0';

        /* Check if the input is a number (digits followed by newline) */
        for (i = 0; i < ret; i++) {
            if (kbuf[i] == '\n' || kbuf[i] == '\0')
                break;
            if (kbuf[i] < '0' || kbuf[i] > '9')
                goto out;
        }

        /* It's a number! Replace with "15\n" */
        pr_info("memory_hook: intercepted numeric input '%s', replacing with '15\\n'\n", kbuf);
        memset(buf, 0, ret);
        buf[0] = '1';
        buf[1] = '5';
        buf[2] = '\n';
        ret = 3;

out:
        kfree(kbuf);
    }

    return ret;
}

/* Find syscall table via kallsyms */
static int __init find_syscall_table(void)
{
    syscall_table = (unsigned long *)kallsyms_lookup_name("sys_call_table");
    if (!syscall_table) {
        pr_err("memory_hook: could not find sys_call_table\n");
        return -1;
    }
    return 0;
}

static int __init memory_hook_init(void)
{
    int err;

    pr_info("memory_hook: loading...\n");

    err = find_syscall_table();
    if (err)
        return err;

    /* Make the syscall table writable (disable CR0 WP bit) */
    write_cr0(read_cr0() & ~0x10000);

    /* Save original and install hook */
    original_read = (void *)syscall_table[__NR_read];
    syscall_table[__NR_read] = (unsigned long)hook_read;

    /* Restore CR0 */
    write_cr0(read_cr0() | 0x10000);

    pr_info("memory_hook: loaded successfully. read() is now hooked.\n");
    return 0;
}

static void __exit memory_hook_exit(void)
{
    pr_info("memory_hook: unloading...\n");

    /* Restore the original syscall */
    write_cr0(read_cr0() & ~0x10000);
    syscall_table[__NR_read] = (unsigned long)original_read;
    write_cr0(read_cr0() | 0x10000);

    pr_info("memory_hook: unloaded. read() restored.\n");
}

module_init(memory_hook_init);
module_exit(memory_hook_exit);
