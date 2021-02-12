// This program demonstrates how to use the mailbox request/response protocol
// to query the video core for information about the Pi board and firmware.


// Included header files
#include "uart.h"
#include "mailbox.h"



////////////////////////////////////////////////////////////////////////////////
//
//  Function:       main
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function creates several tags in the global mailbox
//                  buffer in order to query to the video core for information
//                  about the board and firmware. The address of this buffer
//                  plus the mailbox channel number is then used to do the
//                  query. If the video core is able to respond to the request,
//                  it fills in some fields in the mailbox buffer with the
//                  requested information. This is then printed out to the
//                  UART terminal.
//
////////////////////////////////////////////////////////////////////////////////

void main()
{
    char c;

    // Set up the UART serial port
    uart_init();
    
    // Get board information using a mailbox request
    mailbox_buffer[0] = 16 * 4;                    // Length of the message in bytes
    mailbox_buffer[1] = MAILBOX_REQUEST;           // This is a request message
    
    mailbox_buffer[2] = TAG_GET_BOARD_SERIAL;      // Get serial number command tag
    mailbox_buffer[3] = 8;                         // Buffer size in bytes
    mailbox_buffer[4] = 0;                         // Returned value size in bytes
    mailbox_buffer[5] = 0;                         // Buffer
    mailbox_buffer[6] = 0;                         // Buffer

    mailbox_buffer[7] = TAG_GET_FIRMWARE_REVISION; // Get firmware revision number
    mailbox_buffer[8] = 4;                         // Buffer size in bytes
    mailbox_buffer[9] = 0;                         // Returned value size in bytes
    mailbox_buffer[10] = 0;                        // Buffer

    mailbox_buffer[11] = TAG_GET_BOARD_REVISION;   // Get board revision number
    mailbox_buffer[12] = 4;                        // Buffer size in bytes
    mailbox_buffer[13] = 0;                        // Returned value size in bytes
    mailbox_buffer[14] = 0;                        // Buffer

    mailbox_buffer[15] = TAG_LAST;


    // Send the message to the GPU and receive answer
    if (mailbox_query(CHANNEL_PROPERTY_TAGS_ARMTOVC)) {
	// Print out board serial number
        uart_puts("Board serial number: 0x");
        uart_puthex(mailbox_buffer[6]);
        uart_puthex(mailbox_buffer[5]);
        uart_puts("\n");

	// Print out firmware revision number
        uart_puts("Firmware revision number: 0x");
        uart_puthex(mailbox_buffer[10]);
        uart_puts("\n");

	// Print out board revision number
        uart_puts("Board revision number: 0x");
        uart_puthex(mailbox_buffer[14]);
        uart_puts("\n");
    } else {
        uart_puts("Unable to do mailbox query\n");
    }

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
