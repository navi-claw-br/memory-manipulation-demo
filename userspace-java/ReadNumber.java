import java.util.Scanner;

/**
 * ReadNumber — Simple Java program that reads a number from stdin
 * and prints it. When the kernel module is loaded, any numeric input
 * is silently replaced with 15 by the hooked read() syscall.
 */
public class ReadNumber {
    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);

        System.out.print("Type a number and press Enter: ");
        System.out.flush();

        String input = scanner.nextLine();

        System.out.println("You entered: " + input);

        scanner.close();
    }
}
