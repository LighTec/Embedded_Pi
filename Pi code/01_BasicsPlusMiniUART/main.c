// This is an example of a program that demonstrates communication
// with the Raspberry Pi using the Mini UART peripheral.
// Make sure the host machine is running Minicom, and is connected to the
// Pi via a USB-to-Serial cable. Then boot the Pi by applying power.
// A message should be printed in the Minicom terminal, and you should
// be able to send keystrokes to the Pi, which are echoed back to the
// terminal.


#include "uart.h"

void main()
{
    char c;
  
    // Set up the UART serial port
    uart_init();
    
    // Print out a message to the console
    uart_puts("Goodbye cruel world!\n");
    
    // Loop forever, echoing characters received from the console
    // on a separate line with : : around the character
    while (1) {
        // Wait for a character input from the console
      	c = uart_getc();

        // Output the character
        uart_puts(":");
        uart_putc(c);
        uart_puts(":\n");
    }
}
