// // implement:
// //  void uart_init(void)
// //
// //  int uart_can_getc(void);
// //  int uart_getc(void);
// //
// //  int uart_can_putc(void);
// //  void uart_putc(unsigned c);
// //
// //  int uart_tx_is_empty(void) {
// //
// // see that hello world works.
// //
// //
// #include "rpi.h"
// #include "gpio.h"
// #include "sw-uart.h"
// #include <stdint.h>

// struct UART
// {
//     uint32_t data;
//     uint32_t ier;
//     uint32_t iir;
//     uint32_t lcr;
//     uint32_t mcr;
//     uint32_t lsr;
//     uint32_t msr;
//     uint32_t scratch;
//     uint32_t cntl;
//     uint32_t stat;
//     uint32_t baud;
// };

// // AUX
// #define AUX_ENABLE_BASE 0x20215004
// #define AUX_ENABLE 0x00000001

// // MINI UART
// #define MINI_UART_BASE 0x20215040

// #define MINI_UART_IIR_RX_FIFO_CLEAR 0x00000002
// #define MINI_UART_IIR_TX_FIFO_CLEAR 0x00000004
// #define MINI_UART_IIR_RX_FIFO_ENABLE 0x00000080
// #define MINI_UART_IIR_TX_FIFO_ENABLE 0x00000040

// #define MINI_UART_LSR_RX_READY 0x00000001
// #define MINI_UART_LSR_TX_READY 0x00000040
// #define MINI_UART_LSR_TX_EMPTY 0x00000020

// #define MINI_UART_STAT_TX_EMPTY 0x00000200

// #define MINI_UART_IER_TX_INT 0x00000001
// #define MINI_UART_IER_RX_INT 0x00000002

// #define MINI_UART_LCR_8BIT_MODE 0x00000003

// #define MINI_UART_CNTL_TX_ENABLE 0x00000002
// #define MINI_UART_CNTL_RX_ENABLE 0x00000001

// static volatile struct UART *uart = (struct UART *)MINI_UART_BASE;
// // static bool initialized = ;

// // called first to setup uart to 8n1 115200  baud,
// // no interrupts.
// //  - you will need memory barriers, use <dev_barrier()>
// //
// //  later: should add an init that takes a baud rate.
// void uart_init(void)
// {
//     gpio_set_function(14, GPIO_FUNC_ALT5);
//     gpio_set_function(15, GPIO_FUNC_ALT5);

//     uint32_t *aux = (uint32_t *)AUX_ENABLE_BASE;
//     *aux |= AUX_ENABLE;

//     uart->cntl = 0;
//     uart->ier = 0; // close interupt
//     uart->lcr |= MINI_UART_LCR_8BIT_MODE;
//     uart->mcr = 0; // don't need this
//     uart->iir |= MINI_UART_IIR_RX_FIFO_CLEAR | MINI_UART_IIR_TX_FIFO_CLEAR | MINI_UART_IIR_RX_FIFO_ENABLE | MINI_UART_IIR_TX_FIFO_ENABLE;
//     // from github rpi issue, I found that :
//     // baudrate = system_clock_freq / (8 * ( baudrate_reg + 1 ))
//     uart->baud = 270;
//     uart->cntl = MINI_UART_CNTL_TX_ENABLE | MINI_UART_CNTL_TX_ENABLE;
//     // initialized = true;
// }

// // disable the uart.
// void uart_disable(void)
// {
//     uart->cntl = 0;
// }

// // 1 = at least one byte on rx queue, 0 otherwise
// static int uart_can_getc(void)
// {
//     if (uart->lsr & MINI_UART_LSR_RX_READY)
//         return 1;
//     return 0;
// }

// // returns one byte from the rx queue, if needed
// // blocks until there is one.
// int uart_getc(void)
// {
//     while(!uart_can_getc()) ;
//     return uart->data & 0xFF;
// }

// // 1 = space to put at least one byte, 0 otherwise.
// int uart_can_putc(void)
// {
//     if (uart->lsr & MINI_UART_LSR_TX_READY)
//         return 1;
//     return 0;
// }

// // put one byte on the tx qqueue, if needed, blocks
// // until TX has space.
// void uart_putc(unsigned c)
// {
//     while(!uart_can_putc());
//     uart->data = c & 0xFF;
// }

// // simple wrapper routines useful later.

// // a maybe-more intuitive name for clients.
// int uart_has_data(void)
// {
//     return uart_can_getc();
// }

// // return -1 if no data, otherwise the byte.
// int uart_getc_async(void)
// {
//     if (!uart_has_data())
//         return -1;
//     return uart_getc();
// }

// // 1 = tx queue empty, 0 = not empty.
// int uart_tx_is_empty(void)
// {
//     // unimplemented();
//     if(uart->stat & MINI_UART_STAT_TX_EMPTY)
//         return 1;
//     return 0;
// }

// // flush out all bytes in the uart --- we use this when
// // turning it off / on, etc.
// void uart_flush_tx(void)
// {
//     while (!uart_tx_is_empty())
//         ;
// }

// implement:
//  void uart_init(void)
//
//  int uart_can_getc(void);
//  int uart_getc(void);
//
//  int uart_can_putc(void);
//  void uart_putc(unsigned c);
//
//  int uart_tx_is_empty(void) {
//
// see that hello world works.
//
//
#include "rpi.h"

#define AUX_BASE 0x20215000

static const unsigned aux_start = (AUX_BASE);

// AUXIRQ
// An interrupt is pending on an auxillary device.
typedef struct auxirq {
    unsigned uart : 1;
    unsigned auxirq_unused : 31;
} auxirq_t;

// AUXENB
// Enable an auxillary device's registers for r/w.
typedef struct auxenb {
    unsigned auxenb_uart : 1;
    unsigned auxenb_unused : 31;
} auxenb_t;

// AUX_MU_IO_REG
// ioreg_data is where you can write to the back of the queue,
// and read from the front of the queue.
typedef struct ioreg {
    unsigned io_data : 8;
    unsigned io_reserved : 24;
} ioreg_t;

// AUX_MU_IER_REG
// enable/disable interrupts
typedef struct ierreg {
    unsigned ier_tx_interrupt : 1;
    unsigned ier_rx_interrupt : 1;
    unsigned ier_unused : 30;
} ierreg_t;

// AUX_MU_IIR_REG
// clear FIFOs for transmit and receive
typedef struct iirreg {
    unsigned iir_interrupt_pending : 1;
    unsigned iir_rx_fifo_clear : 1;
    unsigned iir_tx_fifo_clear : 1;
    unsigned iir_gap : 5;
    unsigned iir_reserved : 24;
} iireg_t;

// AUX_MU_LCR_REG
// set UART to 7 bit (clear) or 8 bit (set) mode
typedef struct lcrreg {
    unsigned lcr_data_size : 2;
    unsigned lcr_reserved : 30;
} lcrreg_t;

// AUX_MU_LSR_REG
// Check if we can read from the receive FIFO,
// and if we can write to the transmitting FIFO.
typedef struct lsrreg {
    unsigned lsr_rx_data_ready : 1;
    unsigned lsr_rx_overrun : 1;
    unsigned lsr_gap : 3;
    unsigned lsr_tx_data_ready : 1;
    unsigned lsr_tx_overrun : 1;
    unsigned lsr_reserved : 25;
} lsrreg_t;

// AUX_MU_CNTL_REG
// control register with extra features
typedef struct cntlreg {
    unsigned cntl_rx_enable : 1;
    unsigned cntl_tx_enable : 1;
    unsigned cntl_unused : 30;
} cntlreg_t;

// AUX_MU_STAT_REG
// Data about the tx/rx FIFO queues.
typedef struct statreg {
    unsigned stat_rx_available : 1;
    unsigned stat_tx_available : 1;
    unsigned stat_rx_idle : 1;
    unsigned stat_tx_idle : 1;
    unsigned stat_rx_overrun : 1;
    unsigned stat_tx_full : 1;
    unsigned stat_rts : 1;
    unsigned stat_cts : 1;
    unsigned stat_tx_empty : 1;
    unsigned stat_tx_done : 1; // stat_tx_idle & stat_tx_empty
    unsigned stat_unused : 22;
} statreg_t;

// AUX_MU_BAUD
// Setting + reading the baud rate.
typedef struct baudreg {
    unsigned baud_rate : 16;
    unsigned baud_reserved : 16;
} baudreg_t;

typedef struct muart_registers {
    union {
        unsigned auxirq;
        struct auxirq auxirq_fields;
    };
    union {
        unsigned auxenb;
        struct auxenb auxenb_fields;
    };
    // 56 byte gap (0x5040-0x5008=0x38)
    uint8_t gap[56];
    union {
        unsigned ioreg;
        struct ioreg ioreg_fields;
    };
    union {
        unsigned ierreg;
        struct ierreg ierreg_fields;
    };
    union {
        unsigned iirreg;
        struct iirreg iirreg_fields;
    };
    union {
        unsigned lcrreg;
        struct lcrreg lcrreg_fields;
    };
    union {
        unsigned mcrreg_unused;
    };
    union {
        unsigned lsrreg;
        struct lsrreg lsrreg_fields;
    };
    union {
        unsigned msrreg_unused;
    };
    union {
        unsigned scratchreg_unused;
    };
    union {
        unsigned cntlreg;
        struct cntlreg cntlreg_fields;
    };
    union {
        unsigned statreg;
        struct statreg statreg_fields;
    };
    union {
        unsigned baudreg;
        struct baudreg baudreg_fields;
    };
} muart_registers_t;

// TODO(amartinez): Calling init will set this global variable
static muart_registers_t *uart_registers;

// called first to setup uart to 8n1 115200  baud,
// no interrupts.
//  - you will need memory barriers, use <dev_barrier()>
//
//  later: should add an init that takes a baud rate.
void uart_init(void) {
    uart_registers = (muart_registers_t *)(aux_start);
    // set gpio pins 14/15 as tx/rx respectively.
    gpio_set_function(14, GPIO_FUNC_ALT5); // tx
    gpio_set_function(15, GPIO_FUNC_ALT5); // rx

    // turn on the uart in aux
    dev_barrier();
    uart_registers->auxenb = GET32((unsigned) &uart_registers->auxenb);
    uart_registers->auxenb_fields.auxenb_uart = 1;
    PUT32((unsigned) &uart_registers->auxenb, uart_registers->auxenb);
    dev_barrier();

    // disable current tx/rx
    uart_registers->cntlreg = GET32((unsigned) &uart_registers->cntlreg);
    uart_registers->cntlreg_fields.cntl_rx_enable = 0;
    uart_registers->cntlreg_fields.cntl_tx_enable = 0;
    PUT32((unsigned) &uart_registers->cntlreg, uart_registers->cntlreg);

    // disable interrupts
    uart_registers->ierreg = GET32((unsigned) &uart_registers->ierreg);
    uart_registers->ierreg_fields.ier_rx_interrupt = 0;
    uart_registers->ierreg_fields.ier_tx_interrupt = 0;
    PUT32((unsigned) &uart_registers->ierreg, uart_registers->ierreg);

    // clear FIFO queues
    uart_registers->iirreg = GET32((unsigned) &uart_registers->iirreg);
    uart_registers->iirreg_fields.iir_rx_fifo_clear = 1;
    uart_registers->iirreg_fields.iir_tx_fifo_clear = 1;
    PUT32((unsigned) &uart_registers->iirreg, uart_registers->iirreg);

    // configure 115200 baud rate
    // took core_freq = 250 to mean 250MHz and used the formula to get 270.2
    // Tested from 250-300, looks like it's pretty sensitive as it didn't work
    // at 250, nor 300. Why doesn't it work at other baudrate reg values?
    uart_registers->baudreg = GET32((unsigned) &uart_registers->baudreg);
    uart_registers->baudreg_fields.baud_rate = 270;
    PUT32((unsigned) &uart_registers->baudreg, uart_registers->baudreg);
    
    // configure 8n1
    uart_registers->lcrreg = GET32((unsigned) &uart_registers->lcrreg);
    uart_registers->lcrreg_fields.lcr_data_size = 3;
    PUT32((unsigned) &uart_registers->lcrreg, uart_registers->lcrreg);

    // enable tx/rx
    uart_registers->cntlreg = GET32((unsigned) &uart_registers->cntlreg);
    uart_registers->cntlreg_fields.cntl_rx_enable = 1;
    uart_registers->cntlreg_fields.cntl_tx_enable = 1;
    PUT32((unsigned) &uart_registers->cntlreg, uart_registers->cntlreg);
}

// 1 = at least one byte on rx queue, 0 otherwise
static int uart_can_getc(void) {
    uart_registers = (muart_registers_t *)(aux_start);
    unsigned v = GET32((unsigned) &uart_registers->lsrreg);
    lsrreg_t *lsrreg = (lsrreg_t *) &v;
	return lsrreg->lsr_rx_data_ready;

    
    // uart_registers = (muart_registers_t *)(aux_start);
    // unsigned v = GET32((unsigned) &uart_registers->statreg);
    // statreg_t *statreg = (statreg_t *) &v;
	// return statreg->stat_rx_available;
}

// returns one byte from the rx queue, if needed
// blocks until there is one.
int uart_getc(void) {
    uart_registers = (muart_registers_t *)(aux_start);
    while (!uart_can_getc())
        ;
    unsigned v = GET32((unsigned) &uart_registers->ioreg);
    ioreg_t *ioreg = (ioreg_t *) &v;
	return ioreg->io_data;
}

// 1 = space to put at least one byte, 0 otherwise.
int uart_can_putc(void) {
    uart_registers = (muart_registers_t *)(aux_start);
    unsigned v = GET32((unsigned) &uart_registers->statreg);
    statreg_t *statreg = (statreg_t *) &v;
	return statreg->stat_tx_available;
}

// put one byte on the tx qqueue, if needed, blocks
// until TX has space.
void uart_putc(unsigned c) {
    uart_registers = (muart_registers_t *)(aux_start);
    while (!uart_can_putc())
        ;
    PUT32((unsigned) &uart_registers->ioreg, c);
}

// simple wrapper routines useful later.

// a maybe-more intuitive name for clients.
int uart_has_data(void) {
    return uart_can_getc();
}

// return -1 if no data, otherwise the byte.
int uart_getc_async(void) {
    if(!uart_has_data())
        return -1;
    return uart_getc();
}

// 1 = tx queue empty, 0 = not empty.
int uart_tx_is_empty(void) {
    uart_registers = (muart_registers_t *)(aux_start);
    unsigned v = GET32((unsigned) &uart_registers->statreg);
    statreg_t *statreg = (statreg_t *) &v;
	return statreg->stat_tx_empty;
}

void uart_flush_tx(void) {
    while(!uart_tx_is_empty())
        ;
}

// disable the uart.
void uart_disable(void)
{
    uart_registers->cntlreg = GET32((unsigned) &uart_registers->cntlreg);
    uart_registers->cntlreg_fields.cntl_rx_enable = 0;
    uart_registers->cntlreg_fields.cntl_tx_enable = 0;
    PUT32((unsigned) &uart_registers->cntlreg, uart_registers->cntlreg);
}