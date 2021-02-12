// This program implements and demonstrates timing delays using the BCM
// System Timer. It is based on the GPIO Blinking LED program, which has
// been changed to use the system timer instead of a busy loop.
// The program sets up GPIO pin 23 as an output pin, which is assumed to
// be connected to an LED on a breadboard. The program then blinks the LED
// on and off in an infinite loop. The program also writes ON and OFF to
// the console terminal as the program runs.

// Include files
#include "uart.h"
#include "gpio.h"
#include "systimer.h"

// Function prototypes
void init_GPIO23_to_output();
void set_GPIO23();
void clear_GPIO23();


////////////////////////////////////////////////////////////////////////////////
//
//  Function:       main
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function initializes GPIO pin 23 to an output pin
//                  without a pull-up or pull-down resistor. It then turns
//                  the output pin on and off (1 and 0) in an infinite loop.
//
////////////////////////////////////////////////////////////////////////////////

void main()
{
    // Set up the UART serial port
    uart_init();
    
    // Set up GPIO pin #23 for output
    init_GPIO23_to_output();
    
    // Print out a message to the console
    uart_puts("System Timer Delay Program starting.\n");
    
    // Loop forever, blinking the LED on and off
    while (1) {
        // Turn on the LED
        set_GPIO23();
        
        // Print a message to the console
        uart_puts("ON\n");
        
        // Delay 0.25 second using the system timer
    	microsecond_delay(250000);
    	
    	// Turn the LED off
        clear_GPIO23();
        
        // Print a message to the console
        uart_puts("OFF\n");
        
        // Delay 0.25 second using the system timer
    	microsecond_delay(250000);
    }
}



////////////////////////////////////////////////////////////////////////////////
//
//  Function:       init_GPIO23_to_output
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function sets GPIO pin 23 to an output pin without
//                  any pull-up or pull-down resistors.
//
////////////////////////////////////////////////////////////////////////////////

void init_GPIO23_to_output()
{
    register unsigned int r;
    
    
    // Get the current contents of the GPIO Function Select Register 2
    r = *GPFSEL2;

    // Clear bits 9 - 11. This is the field FSEL23, which maps to GPIO pin 23.
    // We clear the bits by ANDing with a 000 bit pattern in the field.
    r &= ~(0x7 << 9);

    // Set the field FSEL23 to 001, which sets pin 23 to an output pin.
    // We do so by ORing the bit pattern 001 into the field.
    r |= (0x1 << 9);

    // Write the modified bit pattern back to the
    // GPIO Function Select Register 2
    *GPFSEL2 = r;

    // Disable the pull-up/pull-down control line for GPIO pin 23. We follow the
    // procedure outlined on page 101 of the BCM2837 ARM Peripherals manual. The
    // internal pull-up and pull-down resistor isn't needed for an output pin.

    // Disable pull-up/pull-down by setting bits 0:1
    // to 00 in the GPIO Pull-Up/Down Register 
    *GPPUD = 0x0;

    // Wait 150 cycles to provide the required set-up time 
    // for the control signal
    r = 150;
    while (r--) {
      asm volatile("nop");
    }

    // Write to the GPIO Pull-Up/Down Clock Register 0, using a 1 on bit 23 to
    // clock in the control signal for GPIO pin 23. Note that all other pins
    // will retain their previous state.
    *GPPUDCLK0 = (0x1 << 23);

    // Wait 150 cycles to provide the required hold time
    // for the control signal
    r = 150;
    while (r--) {
        asm volatile("nop");
    }

    // Clear all bits in the GPIO Pull-Up/Down Clock Register 0
    // in order to remove the clock
    *GPPUDCLK0 = 0;
}



////////////////////////////////////////////////////////////////////////////////
//
//  Function:       set_GPIO23
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function sets the GPIO output pin 23
//                  to a 1 (high) level.
//
////////////////////////////////////////////////////////////////////////////////

void set_GPIO23()
{
    register unsigned int r;
	  
    // Put a 1 into the SET23 field of the GPIO Pin Output Set Register 0
    r = (0x1 << 23);
    *GPSET0 = r;
}



////////////////////////////////////////////////////////////////////////////////
//
//  Function:       clear_GPIO23
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function clears the GPIO output pin 23
//                  to a 0 (low) level.
//
////////////////////////////////////////////////////////////////////////////////

void clear_GPIO23()
{
    register unsigned int r;
	  
    // Put a 1 into the CLR23 field of the GPIO Pin Output Clear Register 0
    r = (0x1 << 23);
    *GPCLR0 = r;
}
