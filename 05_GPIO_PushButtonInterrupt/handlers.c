// This file contains C functions to handle particular kinds of exceptions.
// Only a function to handle IRQ exceptions is currently implemented.

// Header files
#include "uart.h"
#include "gpio.h"
#include "irq.h"
#include "sysreg.h"

// Reference to the global shared value
extern unsigned int sharedValue;



////////////////////////////////////////////////////////////////////////////////
//
//  Function:       IRQ_handler
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function first prints out some basic information about
//                  the state of the interrupt controller, GPIO pending
//                  interrupts, and selected system registers. It then
//                  determines the particular kind of pending interrupt (which
//                  for the moment is a rising edge event on GPIO pin 17). The
//                  interrupt is cleared, and interrupt is handled in a simple-
//                  minded way by incrementing the shared global variable.
//
////////////////////////////////////////////////////////////////////////////////

void IRQ_handler()
{
    unsigned int r;

    // Print out exception type
    uart_puts("\nInside IRQ exception handler:\n");

    // Print out further information about the exception
    r = getCurrentEL();
    uart_puts("    CurrentEL is:  0x");
    uart_puthex(r);
    uart_puts("\n");

    r = getDAIF();
    uart_puts("    DAIF is:  0x");
    uart_puthex(r);
    uart_puts("\n");

    r = *IRQ_PENDING_2;
    uart_puts("    IRQ_PENDING_2 is:  0x");
    uart_puthex(r);
    uart_puts("\n");

    r = *GPEDS0;
    uart_puts("    GPEDS0 is:  0x");
    uart_puthex(r);
    uart_puts("\n");


    // Handle GPIO interrupts in general
    if (*IRQ_PENDING_2 == (0x1 << 20)) {
      // Handle the interrupt associated with GPIO pin 17
      if (*GPEDS0 == (0x1 << 23)) {
	     // Clear the interrupt by writing a 1 to the GPIO Event Detect
	     // Status Register at bit 17 (p. 96 of the Broadcom Manual)
	     *GPEDS0 = (0x1 << 23);
	     sharedValue = 1;
     }else if(*GPEDS0 == (0x1 << 24)){
       // Clear the interrupt by writing a 1 to the GPIO Event Detect
	     // Status Register at bit 17 (p. 96 of the Broadcom Manual)
	     *GPEDS0 = (0x1 << 24);
	     sharedValue = 0;
     }else{
       // do nothing
     }
    }

    // Return to the IRQ exception handler stub
    return;
}
