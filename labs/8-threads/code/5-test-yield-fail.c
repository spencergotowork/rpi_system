// yield with an empty run queueu.
#include "rpi.h"
#include "rpi-thread.h"

void trivial(void* arg) {
    trace("thread %d yielding\n", rpi_cur_thread()->tid);
    rpi_yield();
    trace("thread %d exiting\n", rpi_cur_thread()->tid);
    rpi_exit(0);
}

void notmain(void) {
    uart_init();
    kmalloc_init_set_start(1024 * 1024, 1024 * 1024);

    rpi_fork(trivial, 0);
    rpi_thread_start();
    printk("SUCCESS\n");
    clean_reboot();
}

void print_and_die(void) { panic("should not call\n"); }
// sp=0x101ff0
// stack=0x102018
// sp[0]=r4=0x100000
// sp[1]=r5=0x9f48  trivial
// sp[2]=r6=0x85ec  bl	8390 <rpi_cur_thread>
// sp[3]=r7=0x9808
// sp[4]=r8=0x8034  __FUNCTION__.0-0x74
// sp[5]=r9=0x0     trivial
// sp[6]=r10=0x8058  bl 8490 <rpi_yield>
// sp[7]=r11=0x0
// sp[8]=r14=0x8694