// SPDX-License-Identifier: MIT
/*
 * read_number.c — A simple C program that:
 *   1. Reads a 1- or 2-digit number from stdin
 *   2. Waits for Enter
 *   3. Prints the number
 *
 * Without the kernel module: prints the number as typed.
 * With the kernel module:   prints 15 (or the configured value).
 *
 * This demonstrates how a kernel module can manipulate user-space
 * memory transparently.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 16

int main(void)
{
    char input[BUFFER_SIZE];
    int number;

    printf("Digite um numero de ate 2 digitos: ");
    fflush(stdout);

    if (fgets(input, sizeof(input), stdin) == NULL) {
        printf("Erro ao ler entrada.\n");
        return 1;
    }

    /* Remove trailing newline */
    input[strcspn(input, "\n")] = '\0';

    /* Validate: must be a 1- or 2-digit number */
    size_t len = strlen(input);
    if (len == 0 || len > 2) {
        printf("Entrada invalida. Use 1 ou 2 digitos.\n");
        return 1;
    }

    for (size_t i = 0; i < len; i++) {
        if (input[i] < '0' || input[i] > '9') {
            printf("Entrada invalida. Use apenas digitos.\n");
            return 1;
        }
    }

    number = atoi(input);
    printf("Numero lido: %d\n", number);

    return 0;
}
