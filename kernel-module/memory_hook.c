// SPDX-License-Identifier: GPL-2.0
/*
 * memory_hook.c — Kernel module that hooks sys_write to demonstrate
 * memory manipulation from kernel space.
 *
 * When loaded, it intercepts write() calls to stdout (fd=1),
 * detects 2-digit numbers embedded in the output buffer, and replaces
 * them with a fixed value (default: 15).
 *
 * For educational/demo purposes only. Do NOT use in production.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kprobes.h>
#include <linux/kallsyms.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Navi (OpenClaw)");
MODULE_DESCRIPTION("Hook sys_write to manipulate memory output for security demo");
MODULE_VERSION("1.0");

/* ============================================================
 * Tunable parameters
 * ============================================================ */

/* The value that will replace the user's number in output */
static unsigned long replace_value = 15;
module_param(replace_value, ulong, 0644);
MODULE_PARM_DESC(replace_value, "Value to replace in output (default: 15)");

/* ============================================================
 * Syscall hook internals
 * ============================================================ */

/* Pointers to the original sys_call_table and sys_write */
static unsigned long *sys_call_table;
static asmlinkage long (*orig_sys_write)(unsigned int fd,
                                          const char __user *buf,
                                          size_t count);

/* Kprobe to resolve kallsyms_lookup_name when it's not exported */
static struct kprobe kp = {
    .symbol_name = "kallsyms_lookup_name",
};

/* Helper: resolve a symbol address via kprobe */
static unsigned long resolve_sym(const char *name)
{
    typedef unsigned long (*kln_t)(const char *);
    kln_t kallsyms_lookup_name_fn;
    unsigned long addr;

    if (register_kprobe(&kp) < 0)
        return 0;

    kallsyms_lookup_name_fn = (kln_t)kp.addr;
    addr = kallsyms_lookup_name_fn(name);
    unregister_kprobe(&kp);

    return addr;
}

/* ============================================================
 * Our hooked sys_write
 * ============================================================
 *
 * Strategy: We intercept writes to stdout/stderr (fd 1, 2).
 *
 * Detection logic:
 *   1. Copy the user buffer into kernel space.
 *   2. If the ENTIRE buffer (ignoring trailing newlines) is a 1- or 2-digit
 *      number, we replace the whole buffer with the fixed value.
 *      (This catches simple cases like printf("42\n"))
 *   3. Additionally, scan for two consecutive digits embedded inside a longer
 *      string (e.g. "Numero lido: 42\n"), and replace only those two digits
 *      in-place.
 *      (This catches printf("Numero lido: %d\n", 42))
 *
 * The in-place replacement preserves the surrounding text.
 */

/* Max buffer size we'll process */
#define MAX_BUF 256

static char write_buf[MAX_BUF];     /* scratch buffer (kernel copy) */
static char output_buf[MAX_BUF];    /* modified buffer sent to userspace */

/*
 * Replace the first occurrence of a 2-digit number found within `src`
 * with the replacement value string. Returns the index where the number
 * was found, or -1 if none found.
 */
static int find_and_replace_number(const char *src, char *dst, size_t dst_size,
                                    unsigned long repl_val, size_t src_len)
{
    int i, j;
    char repl_str[8];
    int repl_len;

    /* Build replacement string */
    repl_len = snprintf(repl_str, sizeof(repl_str), "%lu", repl_val);
    if (repl_len < 0 || (size_t)repl_len >= sizeof(repl_str))
        return -1;

    /* Search for two consecutive digits in the buffer */
    for (i = 0; i < (int)src_len - 1; i++) {
        if (src[i] >= '0' && src[i] <= '9' &&
            src[i+1] >= '0' && src[i+1] <= '9') {

            /* Found a 2-digit number at position i */
            int number = (src[i] - '0') * 10 + (src[i+1] - '0');

            pr_info("[memory_hook] Found number \"%c%c\" (value=%d) at offset %d\n",
                    src[i], src[i+1], number, i);

            /* Rebuild: everything before the number + replacement + everything after */
            {
                int before_len = i;
                int after_len  = src_len - (i + 2);
                size_t pos = 0;

                /* Copy text before the number */
                if (before_len > 0) {
                    memcpy(dst + pos, src, before_len);
                    pos += before_len;
                }

                /* Insert the replacement */
                if (pos + (size_t)repl_len < dst_size) {
                    memcpy(dst + pos, repl_str, repl_len);
                    pos += repl_len;
                } else {
                    break;
                }

                /* Copy text after the number */
                if (after_len > 0 && pos + (size_t)after_len < dst_size) {
                    memcpy(dst + pos, src + i + 2, after_len);
                    pos += after_len;
                }

                dst[pos] = '\0';
            }

            /* Return the number we replaced */
            return number;
        }
    }

    return -1;
}

asmlinkage long hook_sys_write(unsigned int fd,
                                const char __user *buf,
                                size_t count)
{
    size_t read_len;
    bool replaced = false;

    /* Only intercept stdout/stderr */
    if (fd != 1 && fd != 2)
        goto pass_through;

    /* Limit buffer read */
    read_len = min(count, (size_t)(MAX_BUF - 1));

    if (copy_from_user(write_buf, buf, read_len))
        goto pass_through;

    write_buf[read_len] = '\0';

    /* --- Strategy 1: Buffer is entirely a number (with possible trailing newline) --- */
    {
        char stripped[MAX_BUF];
        int strip_len;
        int all_digits;
        int i;

        /* Remove trailing newlines */
        strscpy(stripped, write_buf, sizeof(stripped));
        strip_len = strlen(stripped);
        while (strip_len > 0 && (stripped[strip_len-1] == '\n' ||
                                 stripped[strip_len-1] == '\r'))
            stripped[--strip_len] = '\0';

        all_digits = 1;
        if (strip_len == 0 || strip_len > 2)
            all_digits = 0;
        else {
            for (i = 0; i < strip_len; i++) {
                if (stripped[i] < '0' || stripped[i] > '9') {
                    all_digits = 0;
                    break;
                }
            }
        }

        if (all_digits) {
            /* Replace the entire output with our fixed value */
            int n = snprintf(output_buf, sizeof(output_buf), "%lu\n", replace_value);
            if ((size_t)n <= count) {
                if (copy_to_user((void __user *)buf, output_buf, n)) {
                    pr_err("[memory_hook] copy_to_user failed (strategy 1)\n");
                    goto pass_through;
                }
                pr_info("[memory_hook] Replaced output: \"%s\" -> \"%s\" (strategy 1)\n",
                        stripped, output_buf);
                return n;
            }
        }
    }

    /* --- Strategy 2: Embedded 2-digit number inside longer text --- */
    {
        int found_num;
        int orig_count = count;

        found_num = find_and_replace_number(write_buf, output_buf,
                                             sizeof(output_buf),
                                             replace_value, read_len);

        if (found_num >= 0) {
            size_t new_len = strlen(output_buf);
            /* Ensure we don't exceed original write length */
            if (new_len > orig_count)
                new_len = orig_count;

            if (copy_to_user((void __user *)buf, output_buf, new_len)) {
                pr_err("[memory_hook] copy_to_user failed (strategy 2)\n");
                goto pass_through;
            }
            pr_info("[memory_hook] Replaced number %d -> %lu in: \"%s\"\n",
                    found_num, replace_value, output_buf);
            replaced = true;
            return new_len;
        }
    }

    /* Also handle 1-digit numbers embedded in text */
    {
        int i;
        char repl_one_digit[8];
        int repl_len = snprintf(repl_one_digit, sizeof(repl_one_digit), "%lu", replace_value);

        for (i = 0; i < (int)read_len; i++) {
            /* Single digit (preceded by non-digit or start, followed by non-digit or end) */
            if (write_buf[i] >= '0' && write_buf[i] <= '9') {
                /* Check this is NOT part of a 2-digit number (already handled above) */
                int is_2digit = 0;
                if (i > 0 && write_buf[i-1] >= '0' && write_buf[i-1] <= '9')
                    is_2digit = 1;
                if (i < (int)read_len - 1 && write_buf[i+1] >= '0' && write_buf[i+1] <= '9')
                    is_2digit = 1;

                if (!is_2digit) {
                    /* Replace single digit at position i */
                    char temp_out[MAX_BUF];
                    size_t pos = 0;

                    if (i > 0) {
                        memcpy(temp_out, write_buf, i);
                        pos = i;
                    }
                    memcpy(temp_out + pos, repl_one_digit, repl_len);
                    pos += repl_len;

                    if ((int)read_len > i + 1) {
                        int after = read_len - (i + 1);
                        if (pos + after < MAX_BUF) {
                            memcpy(temp_out + pos, write_buf + i + 1, after);
                            pos += after;
                        }
                    }
                    temp_out[pos] = '\0';

                    if (pos <= orig_sys_write(fd, buf, count)) {
                        /* Use original count as bound */
                        size_t copy_len = min(pos, count);
                        if (copy_to_user((void __user *)buf, temp_out, copy_len) == 0) {
                            pr_info("[memory_hook] Replaced single digit at %d -> %lu\n",
                                    i, replace_value);
                            replaced = true;
                            return copy_len;
                        }
                    }
                    break;
                }
            }
        }
    }

    if (!replaced)
        goto pass_through;

    /* Already returned above if replaced */

pass_through:
    return orig_sys_write(fd, buf, count);
}

/* ============================================================
 * Module lifecycle
 * ============================================================ */

static void disable_write_protection(void)
{
    unsigned long cr0 = read_cr0();
    clear_bit(16, &cr0);
    write_cr0(cr0);
}

static void enable_write_protection(void)
{
    unsigned long cr0 = read_cr0();
    set_bit(16, &cr0);
    write_cr0(cr0);
}

static int __init memory_hook_init(void)
{
    unsigned long sys_write_addr;
    int nr_write;

    pr_info("[memory_hook] Loading memory manipulation module\n");
    pr_info("[memory_hook] Replace value set to: %lu\n", replace_value);

    /* Resolve sys_call_table */
    sys_call_table = (unsigned long *)resolve_sym("sys_call_table");
    if (!sys_call_table) {
        pr_err("[memory_hook] Failed to find sys_call_table\n");
        return -EFAULT;
    }
    pr_info("[memory_hook] sys_call_table found at: %px\n", sys_call_table);

#if defined(__x86_64__)
    nr_write = 1;  /* __NR_write on x86_64 */
#elif defined(__i386__)
    nr_write = 4;
#else
    nr_write = 1;
#endif

    orig_sys_write = (asmlinkage long (*)(unsigned int, const char __user *, size_t))
                     sys_call_table[nr_write];

    /* Write our hook into the syscall table */
    disable_write_protection();
    sys_call_table[nr_write] = (unsigned long)hook_sys_write;
    enable_write_protection();

    sys_write_addr = (unsigned long)sys_call_table[nr_write];
    pr_info("[memory_hook] sys_write hooked! Original at: %px, Hook at: %px\n",
            (void *)orig_sys_write, (void *)sys_write_addr);

    return 0;
}

static void __exit memory_hook_exit(void)
{
    int nr_write;

#if defined(__x86_64__)
    nr_write = 1;
#elif defined(__i386__)
    nr_write = 4;
#else
    nr_write = 1;
#endif

    /* Restore original sys_write */
    disable_write_protection();
    sys_call_table[nr_write] = (unsigned long)orig_sys_write;
    enable_write_protection();

    pr_info("[memory_hook] sys_write restored. Module unloaded.\n");
}

module_init(memory_hook_init);
module_exit(memory_hook_exit);
