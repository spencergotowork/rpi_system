// engler, cs140 put your gpio implementations in here.
#include "rpi.h"
// #include "rpi-interrupts.h"

// see broadcomm documents for magic addresses.
#define GPIO_BASE 0x20200000
// static const unsigned gpio_set0  = (GPIO_BASE + 0x1C);
volatile uint32_t *gpio_set0  = (void*)(GPIO_BASE + 0x1C);
volatile uint32_t *gpio_clr0  = (void*)(GPIO_BASE + 0x28);
volatile uint32_t *gpio_lev0  = (void*)(GPIO_BASE + 0x34);
volatile uint32_t *gpio_ren0  = (void*)(GPIO_BASE + 0x4c);
volatile uint32_t *gpio_fen0  = (void*)(GPIO_BASE + 0x58);
volatile uint32_t *gpio_gpfsel0  = (void*)(GPIO_BASE + 0x00);
volatile uint32_t *gpio_peds0 = (void*)(GPIO_BASE + 0x40);
// interrupt
volatile uint32_t *irq_pending_2 = (void*)0x2000b208;
volatile uint32_t *enable_IRQs_2 = (void*)0x2000b214;

static void or32(volatile void *addr, uint32_t val) {
    // dev_barrier();
    put32(addr, get32(addr) | val);
    // dev_barrier();
}
static void OR32(uint32_t addr, uint32_t val) {
    or32((volatile void*)addr, val);
}

/*
  2c:	e351001f 	cmp	r1, #31
  30:	812fff1e 	bxhi	lr
  34:	e92d4070 	push	{r4, r5, r6, lr}
  38:	e1a05000 	mov	r5, r0
  3c:	e1a04001 	mov	r4, r1
  40:	ebfffffe 	bl	0 <dev_barrier>
  44:	e3a01001 	mov	r1, #1
  48:	e1a01411 	lsl	r1, r1, r4
  4c:	e1a00005 	mov	r0, r5
  50:	ebfffff2 	bl	20 <OR32>
  54:	ebfffffe 	bl	0 <dev_barrier>
  58:	e3a01802 	mov	r1, #131072	; 0x20000
  5c:	e59f0008 	ldr	r0, [pc, #8]	; 6c <gpio_int_set+0x40>
  60:	ebffffee 	bl	20 <OR32>
  64:	ebfffffe 	bl	0 <dev_barrier>
  68:	e8bd8070 	pop	{r4, r5, r6, pc}
  6c:	2000b214 	andcs	fp, r0, r4, lsl r2
*/
void gpio_int_set(uint32_t addr, uint32_t pin) {
    if(pin >= 32)
        return;
    dev_barrier();
    OR32(addr, 0b1 << pin);
    dev_barrier();
    or32(enable_IRQs_2, 0b1 << 17);
    dev_barrier();
}

// gpio_int_rising_edge and gpio_int_falling_edge (and any other) should
// call this routine (you must implement) to setup the right GPIO event.
// as with setting up functions, you should bitwise-or in the value for the 
// pin you are setting with the existing pin values.  (otherwise you will
// lose their configuration).  you also need to enable the right IRQ.   make
// sure to use device barriers!!
int is_gpio_int(unsigned gpio_int) {
    // unimplemented();
    unsigned idx = gpio_int - 49;
    int res = 0;
    if(idx < 3) {
        gpio_int &= 0x1f; //  0b11111
        res = (get32(irq_pending_2) >> gpio_int) & 1;
        return res;
    } else {
        panic("wrong is_gpio_int");
    }
}


// p97 set to detect rising edge (0->1) on <pin>.
// as the broadcom doc states, it  detects by sampling based on the clock.
// it looks for "011" (low, hi, hi) to suppress noise.  i.e., its triggered only
// *after* a 1 reading has been sampled twice, so there will be delay.
// if you want lower latency, you should us async rising edge (p99)
void gpio_int_rising_edge(unsigned pin) {
    // unimplemented();
    if(pin >= 32)
        return;
    gpio_int_set((uint32_t)gpio_ren0, pin);
}

// p98: detect falling edge (1->0).  sampled using the system clock.  
// similarly to rising edge detection, it suppresses noise by looking for
// "100" --- i.e., is triggered after two readings of "0" and so the 
// interrupt is delayed two clock cycles.   if you want  lower latency,
// you should use async falling edge. (p99)
void gpio_int_falling_edge(unsigned pin) {
    // unimplemented();
    if(pin >= 32)
        return;
    gpio_int_set((uint32_t)gpio_fen0, pin);
}

// p96: a 1<<pin is set in EVENT_DETECT if <pin> triggered an interrupt.
// if you configure multiple events to lead to interrupts, you will have to 
// read the pin to determine which caused it.
int gpio_event_detected(unsigned pin) {
    // unimplemented();
    if(pin >= 32)
        return 0;
    dev_barrier();
    
    int res = (get32(gpio_peds0) >> pin) & 1;
    dev_barrier();
    return res;
}

// p96: have to write a 1 to the pin to clear the event.
void gpio_event_clear(unsigned pin) {
    // unimplemented();
    if(pin >= 32)
        return;
    
    dev_barrier();
    put32(gpio_peds0, 0b1 << pin);
    dev_barrier();
    // OR32(gpio_clr0, 1 << pin);
}
