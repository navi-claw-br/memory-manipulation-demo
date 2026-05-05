// SPDX-License-Identifier: MIT
/*
 * ReadNumber.java — A simple Java program that:
 *   1. Reads a 1- or 2-digit number from stdin
 *   2. Waits for Enter
 *   3. Prints the number
 *
 * Without the kernel module: prints the number as typed.
 * With the kernel module:   prints 15 (or the configured value).
 *
 * Same principle as the C version — the kernel module intercepts
 * the write() syscall and manipulates the buffer in kernel space.
 */

import java.util.Scanner;

public class ReadNumber {

    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);

        System.out.print("Digite um numero de ate 2 digitos: ");
        System.out.flush();

        String input = scanner.nextLine().trim();

        // Validate: must be a 1- or 2-digit number
        if (input.isEmpty() || input.length() > 2) {
            System.out.println("Entrada invalida. Use 1 ou 2 digitos.");
            scanner.close();
            return;
        }

        for (int i = 0; i < input.length(); i++) {
            if (!Character.isDigit(input.charAt(i))) {
                System.out.println("Entrada invalida. Use apenas digitos.");
                scanner.close();
                return;
            }
        }

        int number = Integer.parseInt(input);
        System.out.println("Numero lido: " + number);

        scanner.close();
    }
}
