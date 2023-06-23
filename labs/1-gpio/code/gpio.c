/*
 * Implement the following routines to set GPIO pins to input or output,
 * and to read (input) and write (output) them.
 *
 * DO NOT USE loads and stores directly: only use GET32 and PUT32 
 * to read and write memory.  Use the minimal number of such calls.
 *
 * See rpi.h in this directory for the definitions.
 */
#include "rpi.h"

// see broadcomm documents for magic addresses.
#define GPIO_BASE 0x20200000
// static const unsigned gpio_set0  = (GPIO_BASE + 0x1C);
volatile unsigned *gpio_set0  = (void*)(GPIO_BASE + 0x1C);
volatile unsigned *gpio_clr0  = (void*)(GPIO_BASE + 0x28);
volatile unsigned *gpio_lev0  = (void*)(GPIO_BASE + 0x34);
volatile unsigned *gpio_gpfsel0  = (void*)(GPIO_BASE + 0x00);

//
// Part 1 implement gpio_set_on, gpio_set_off, gpio_set_output
//

// set <pin> to be an output pin.  
//
// note: fsel0, fsel1, fsel2 are contiguous in memory, so you 
// can (and should) use array calculations!
void gpio_set_output(unsigned pin) {
    // implement this
    // use <gpio_fsel0>
    if(pin >= 32)
        return;

    unsigned reg_num = pin/10;
    volatile unsigned *addr = gpio_gpfsel0 + reg_num; // bad code, gpio_gpfsel0 is a pointer!!!
    unsigned fsel_num = pin % 10;
    unsigned tmp = get32(addr);
    tmp &= ~(0b111 << (fsel_num * 3));
    tmp |= (0b001 << (3 * fsel_num));
    put32(addr, tmp);
}

// set GPIO <pin> on.
void gpio_set_on(unsigned pin) {
    // implement this
    // use <gpio_set0>
    if(pin >= 32)
        return;
    put32(gpio_set0, 1 << pin);
}

// set GPIO <pin> off
void gpio_set_off(unsigned pin) {
    // implement this
    // use <gpio_clr0>
    if(pin >= 32)
        return;
    put32(gpio_clr0, 1 << pin);

}

// set <pin> to <v> (v \in {0,1})
void gpio_write(unsigned pin, unsigned v) {
    if(v)
       gpio_set_on(pin);
    else
       gpio_set_off(pin);
}

//
// Part 2: implement gpio_set_input and gpio_read
//

// set <pin> to input.
void gpio_set_input(unsigned pin) {
    // implement.
    if(pin >= 32)
        return;
        
    unsigned reg_num = pin/10;
    volatile unsigned *addr = gpio_gpfsel0 + reg_num;//reg_num*4;
    unsigned fsel_num = pin % 10;
    unsigned tmp = get32(addr);
    tmp &= ~(0b111 << (fsel_num * 3));
    // tmp |= (0b001 << (3 * fsel_num));
    put32(addr, tmp);
}

// return the value of <pin>
int gpio_read(unsigned pin) {
    unsigned v = 0;
    if(pin >= 32)
        return -1;

    // implement.
    if(get32(gpio_lev0) & (0b1 << pin))
        v = 1;
    return v;
}

void reboot(void) {
      const int PM_RSTC = 0x2010001c;
      const int PM_WDOG = 0x20100024;
      const int PM_PASSWORD = 0x5a000000;
      const int PM_RSTC_WRCFG_FULL_RESET = 0x00000020;
      int i;
      for(i = 0; i < 100000; i++)
           nop();
      PUT32(PM_WDOG, PM_PASSWORD | 1);
      PUT32(PM_RSTC, PM_PASSWORD | PM_RSTC_WRCFG_FULL_RESET);
      while(1);
 }
