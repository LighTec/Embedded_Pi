#include "uart.h"
#include "gpio.h"
#include "irq.h"
#include "IRQEnable.h"

#define DELAY_LO    ((volatile unsigned int *)(MMIO_BASE + 0x00003004))
#define DELAY_HI    ((volatile unsigned int *)(MMIO_BASE + 0x00003008))

void init_GPIO17_to_output();
void init_GPIO22_to_output();
void init_GPIO27_to_output();
void set_GPIO();
void clear_GPIO();
void microsecond_delay();
void init_GPIO23_to_fallingEdgeInterrupt();
void init_GPIO24_to_risingEdgeInterrupt();
void setState0();
void setState1();
void setState2();

//global shared variable
unsigned int sharedValue;

void main()
{
    uart_init();
    init_GPIO17_to_output();
    init_GPIO22_to_output();
    init_GPIO27_to_output();
    init_GPIO23_to_fallingEdgeInterrupt();
    init_GPIO24_to_risingEdgeInterrupt();
    enableInterrupts();

    // marks the pattern direction
    sharedValue = 0;
    // marks the current state of the LEDs
    int currentState = 0;
    // Loop forever, blinking the LEDs on and off
    while (1) {
      if(currentState == 0){
        if(sharedValue == 0){
          // State 0,0 -> state 1
          currentState = 1;
          setState1();
          microsecond_delay(250000);
        }else{
          // State 0,1 -> state 2
          currentState = 2;
          setState2();
        }
      }else if(currentState == 1){
        if(sharedValue == 0){
          // State 1,0 -> state 2
          currentState = 2;
          setState2();
          microsecond_delay(250000);
        }else{
          // State 1,1 -> state 0
          currentState = 0;
          setState0();
        }
      }else if(currentState == 2){
        if(sharedValue == 0){
          // State 2,0 -> state 0
          currentState = 0;
          setState0();
          microsecond_delay(250000);
        }else{
          // State 2,1 -> state 1
          currentState = 1;
          setState1();
        }
      }else{
        uart_puts("This shouldn't have happened! current state invalid.");
        currentState = 0;
      }
    }
}

void setState0(){
  set_GPIO(17);
  clear_GPIO(22);
  clear_GPIO(27);
  uart_puts("0\n");
  microsecond_delay(250000);
}

void setState1(){
  clear_GPIO(17);
  clear_GPIO(22);
  set_GPIO(27);
  uart_puts("1\n");
  microsecond_delay(250000);
}

void setState2(){
  clear_GPIO(17);
  set_GPIO(22);
  clear_GPIO(27);
  uart_puts("2\n");
  microsecond_delay(250000);
}

void init_GPIO17_to_output()
{
    register unsigned int r;

    r = *GPFSEL1;
    // Clear bits 21-23
    r &= ~(0x7 << 21);
    // Set FSEL17 to 001
    r |= (0x1 << 21);
    // save FSEL17
    *GPFSEL1 = r;
    // Disable pull-up/pull-down
    *GPPUD = 0x0;
    // Wait 150 cycles
    r = 150;
    while (r--) {
      asm volatile("nop");
    }
    //write pullup pulldown clock register
    *GPPUDCLK0 = (0x1 << 17);
    //wait
    r = 150;
    while (r--) {
      asm volatile("nop");
    }
    *GPPUDCLK0 = 0;
}

void init_GPIO22_to_output()
{
    register unsigned int r;

    r = *GPFSEL2;
    // Clear bits 6-8
    r &= ~(0x7 << 6);
    // Set FSEL22 to 001
    r |= (0x1 << 6);
    // Write back
    *GPFSEL2 = r;
    // Disable pull-up pull-down
    *GPPUD = 0x0;
    //wait
    r = 150;
    while (r--) {
      asm volatile("nop");
    }
    //write pullup pulldown clock register
    *GPPUDCLK0 = (0x1 << 22);
    //wait
    r = 150;
    while (r--) {
      asm volatile("nop");
    }
    *GPPUDCLK0 = 0;
}

void init_GPIO27_to_output()
{
    register unsigned int r;

    r = *GPFSEL2;
    // Clear bits 21 - 23
    r &= ~(0x7 << 21);
    // Set FSEL27 to 001
    r |= (0x1 << 21);
    // Write back
    *GPFSEL2 = r;
    // Disable pull-up pull-down
    *GPPUD = 0x0;
    //wait a while
    r = 150;
    while (r--) {
      asm volatile("nop");
    }
    //write pullup pulldown clock register
    *GPPUDCLK0 = (0x1 << 27);
    // Wait a while
    r = 150;
    while (r--) {
      asm volatile("nop");
    }
    *GPPUDCLK0 = 0;
}

void set_GPIO(int reg)
{
	  register unsigned int r;
	  // Put a 1 into the SET23 field of the GPIO Pin Output Set Register 0
	  r = (0x1 << reg);
	  *GPSET0 = r;
}

void clear_GPIO(int reg)
{
	  register unsigned int r;
	  // Put a 1 into the CLR23 field of the GPIO Pin Output Clear Register 0
	  r = (0x1 << reg);
	  *GPCLR0 = r;
}

void init_GPIO24_to_risingEdgeInterrupt()
{
    register unsigned int r;

    r = *GPFSEL2;
    // clear field
    r &= ~(0x7 << 12);
    // Write back
    *GPFSEL2 = r;
    // Disable pull-up pull-down
    *GPPUD = 0x0;
    //wait a bit
    r = 150;
    while (r--) {
        asm volatile("nop");
    }
    // write back 1 to 24th bit representing GPIO24
    *GPPUDCLK0 = (0x1 << 24);
    //wait
    r = 150;
    while (r--) {
        asm volatile("nop");
    }
    // Clear all bits in GPPUDCLK0
    *GPPUDCLK0 = 0;
    // enable rising edge detect on pin 24
    *GPREN0 = (0x1 << 24);
    // Enable the GPIO IRQS for all GPIO pins
    *IRQ_ENABLE_IRQS_2 = (0x1 << 20);
}

void init_GPIO23_to_fallingEdgeInterrupt()
{
    register unsigned int r;

    r = *GPFSEL2;
    //clear field
    r &= ~(0x7 << 9);
    //write back
    *GPFSEL2 = r;
    // Disable pull-up pull-down
    *GPPUD = 0x0;
    //wait a while
    r = 150;
    while (r--) {
        asm volatile("nop");
    }
    // write back 1 to 24th bit representing GPIO23
    *GPPUDCLK0 = (0x1 << 23);
    //wait a while
    r = 150;
    while (r--) {
        asm volatile("nop");
    }
    *GPPUDCLK0 = 0;
    *GPFEN0 = (0x1 << 23);
    // enable all pins again, even though init GPIO24 should already do it
    *IRQ_ENABLE_IRQS_2 = (0x1 << 20);
}

void IRQ_handler()
{
    if (*IRQ_PENDING_2 == (0x1 << 20)) {
      // handle pin 23
      if (*GPEDS0 == (0x1 << 23)) {
	     //clear the interrupt
	     *GPEDS0 = (0x1 << 23);
	     sharedValue = 1;
       // handle pin 24
     }else if(*GPEDS0 == (0x1 << 24)){
       //clear the interrupt
	     *GPEDS0 = (0x1 << 24);
	     sharedValue = 0;
     }else{
       // do nothing
     }
    }
    return;
}

// instead of busy loops, we'll now use system time with busy loops for better
// delay accuracy.
void microsecond_delay(unsigned int interval)
{
    unsigned long current_counter, target_counter;
	  unsigned int high, low;
    //Read the 2 registers for the system time
    high = *DELAY_HI;
    low = *DELAY_LO;
    //repeat the read in case DELAY_HI rolled over
    if (high != *DELAY_HI) {
        high = *DELAY_HI;
        low = *DELAY_LO;
    }
    current_counter = (((unsigned long)high << 32) | low );

    // Because Qemu does not emulate the system counter, the timer counter will
    // always be 0 and we cannot use it to do timing (it will result in an
    // infinite loop). In this case, we return immediately (without any delay).
    if (current_counter == 0) {
        return;
    }

    // Calculate the target value of the system timer counter. This will be
    // the specified number of microseconds into the future.
    target_counter = current_counter + interval;

    // Keep polling the system timer counter until we reach the target value
    while (current_counter < target_counter){
      //Read the 2 registers for the system time
      high = *DELAY_HI;
      low = *DELAY_LO;
      //repeat the read in case DELAY_HI rolled over
      if (high != *DELAY_HI) {
          high = *DELAY_HI;
          low = *DELAY_LO;
      }
      current_counter = (((unsigned long)high << 32) | low );
    }

    // Once we have reached this point, we have delayed the specified number
    // of microseconds, so return
    return;
}
