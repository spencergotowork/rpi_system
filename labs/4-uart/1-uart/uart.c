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
#include "gpio.h"
#include "sw-uart.h"
#include <stdint.h>

// #define AUX_BASE 0x20210000

// typedef struct {
//     unsigned uart_irq : 1;
//     unsigned spi1 : 1;
//     unsigned spi2 : 1;
//     unsigned unused : 29;
// } AUXIRQ;

// typedef struct {
//     unsigned uart_enb : 1;
//     unsigned unused : 31;
// } AUXENB;

// typedef struct {
//     unsigned txrx : 8;
//     unsigned unused : 24;
// } AUX_MU_IO_REG;

// typedef struct {
//     unsigned enb_tx_irq : 1;
//     unsigned enb_rx_irq : 1;
//     unsigned enb_gap : 5;
//     unsigned unused : 24;
// } AUX_MU_IIR_REG;

// typedef struct {
//     unsigned irq_pend : 1;
//     unsigned wr_bits : 2;
//     unsigned gap : 3;
//     unsigned fifo_enb: 2;
//     unsigned unused : 24;
// } AUX_MU_IER_REG;

// typedef struct {
//     unsigned size : 1;
//     unsigned gap : 5;
//     unsigned break : 1;
//     unsigned DLAB_acs : 1;
//     unsigned unused : 24;
// } AUX_MU_LCR_REG;

// typedef struct {
//     unsigned gap : 1;
//     unsigned rts : 1;
//     unsigned unused : 30;
// } AUX_MU_MCR_REG;

// typedef struct {
//     unsigned data_ready : 1;
//     unsigned rcv_overrun : 1;
//     unsigned gap : 3;
//     unsigned trans_empty : 1;
//     unsigned trans_idle : 1;
//     unsigned unused : 25;
// } AUX_MU_LSR_REG;

// typedef struct {
//     unsigned gap : 4;
//     unsigned cts : 1;
//     unsigned unused : 26;
// } AUX_MU_MSR_REG;

// typedef struct {
//     unsigned scratch : 8;
//     unsigned unused : 24;
// } AUX_MU_SCRATCH;

// typedef struct {
//     unsigned rcv_enb : 1;
//     unsigned tx_enb : 1;
//     unsigned enb_rcv_rts : 1;
//     unsigned enb_tx_cts : 1;
//     unsigned rts_lev : 2;
//     unsigned rts_ast_lev : 1;
//     unsigned cts_ast_lev : 1;
//     unsigned unused : 24;
// } AUX_MU_CNTL_REG;

// typedef struct {
//     unsigned syb_avl : 1;
//     unsigned spc_avl : 1;
//     unsigned rcv_idle : 1;
//     unsigned tx_idle : 1;
//     unsigned rcv_overrun : 1;
//     unsigned tx_fifo_full : 1;
//     unsigned rts_status : 1;
//     unsigned cts_line : 1;
//     unsigned tx_fifo_empty : 1;
//     unsigned tx_done : 1;
//     unsigned gap : 6;
//     unsigned rcv_fifo_lev : 4;
//     unsigned gap1:4;
//     unsigned tx_fifo_fill_lev:4;
//     unsigned unused : 4;
// } AUX_MU_STAT_REG;

// typedef struct {
//     unsigned baudrate : 16;
//     unsigned unused : 16;
// } AUX_MU_BAUD;

struct UART
{
    uint32_t data;
    uint32_t ier;
    uint32_t iir;
    uint32_t lcr;
    uint32_t mcr;
    uint32_t lsr;
    uint32_t msr;
    uint32_t scratch;
    uint32_t cntl;
    uint32_t stat;
    uint32_t baud;
};

// AUX
#define AUX_ENABLE_BASE 0x20215004
#define AUX_ENABLE 0x00000001

// MINI UART
#define MINI_UART_BASE 0x20215040

#define MINI_UART_IIR_RX_FIFO_CLEAR 0x00000002
#define MINI_UART_IIR_TX_FIFO_CLEAR 0x00000004
#define MINI_UART_IIR_RX_FIFO_ENABLE 0x00000080
#define MINI_UART_IIR_TX_FIFO_ENABLE 0x00000040

#define MINI_UART_LSR_RX_READY 0x00000001
#define MINI_UART_LSR_TX_READY 0x00000040
#define MINI_UART_LSR_TX_EMPTY 0x00000020

#define MINI_UART_STAT_TX_EMPTY 0x00000200

#define MINI_UART_IER_TX_INT 0x00000001
#define MINI_UART_IER_RX_INT 0x00000002

#define MINI_UART_LCR_8BIT_MODE 0x00000003

#define MINI_UART_CNTL_TX_ENABLE 0x00000002
#define MINI_UART_CNTL_RX_ENABLE 0x00000001

static volatile struct UART *uart = (struct UART *)MINI_UART_BASE;
// static bool initialized = ;

// called first to setup uart to 8n1 115200  baud,
// no interrupts.
//  - you will need memory barriers, use <dev_barrier()>
//
//  later: should add an init that takes a baud rate.
void uart_init(void)
{
    gpio_set_function(14, GPIO_FUNC_ALT5);
    gpio_set_function(15, GPIO_FUNC_ALT5);

    uint32_t *aux = (uint32_t *)AUX_ENABLE_BASE;
    *aux |= AUX_ENABLE;

    uart->cntl = 0;
    uart->ier = 0; // close interupt
    uart->lcr |= MINI_UART_LCR_8BIT_MODE;
    uart->mcr = 0; // don't need this
    uart->iir |= MINI_UART_IIR_RX_FIFO_CLEAR | MINI_UART_IIR_TX_FIFO_CLEAR | MINI_UART_IIR_RX_FIFO_ENABLE | MINI_UART_IIR_TX_FIFO_ENABLE;
    // from github rpi issue, I found that :
    // baudrate = system_clock_freq / (8 * ( baudrate_reg + 1 ))
    uart->baud = 270;
    uart->cntl = MINI_UART_CNTL_TX_ENABLE | MINI_UART_CNTL_TX_ENABLE;
    // initialized = true;
}

// disable the uart.
void uart_disable(void)
{
    uart->cntl = 0;
}

// 1 = at least one byte on rx queue, 0 otherwise
static int uart_can_getc(void)
{
    if (uart->lsr & MINI_UART_LSR_RX_READY)
        return 1;
    return 0;
}

// returns one byte from the rx queue, if needed
// blocks until there is one.
int uart_getc(void)
{
    while(!uart_can_getc()) ;
    return uart->data & 0xFF;
}

// 1 = space to put at least one byte, 0 otherwise.
int uart_can_putc(void)
{
    if (uart->lsr & MINI_UART_LSR_TX_READY)
        return 1;
    return 0;
}

// put one byte on the tx qqueue, if needed, blocks
// until TX has space.
void uart_putc(unsigned c)
{
    while(!uart_can_putc());
    uart->data = c & 0xFF;
}

// simple wrapper routines useful later.

// a maybe-more intuitive name for clients.
int uart_has_data(void)
{
    return uart_can_getc();
}

// return -1 if no data, otherwise the byte.
int uart_getc_async(void)
{
    if (!uart_has_data())
        return -1;
    return uart_getc();
}

// 1 = tx queue empty, 0 = not empty.
int uart_tx_is_empty(void)
{
    // unimplemented();
    if(uart->stat & MINI_UART_STAT_TX_EMPTY)
        return 1;
    return 0;
}

// flush out all bytes in the uart --- we use this when
// turning it off / on, etc.
void uart_flush_tx(void)
{
    while (!uart_tx_is_empty())
        ;
}
