/*
 * read_number.c — Simple userspace C program that reads a number from stdin
 * and prints it. When the kernel module is loaded, any numeric input is
 * silently replaced with 15.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    char buf[64];

    printf("Type a number and press Enter: ");
    fflush(stdout);

    if (!fgets(buf, sizeof(buf), stdin)) {
        printf("Error reading input.\n");
        return 1;
    }

    /* Remove trailing newline */
    buf[strcspn(buf, "\n")] = '\0';

    printf("You entered: %s\n", buf);

    return 0;
}
