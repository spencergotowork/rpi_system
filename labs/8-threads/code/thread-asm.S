/*
 * implement these.  ordered in difficulty.
 */
#include "rpi-asm.h"

#define NOTREACHED mov r0, pc; bl asm_not_reached
#define UNIMPL mov r0, pc; bl asm_not_implemented

@ return the current stack pointer.
MK_FN(rpi_get_sp)
    UNIMPL
	bx lr

MK_FN(rpi_cswitch)
@    UNIMPL
@    bx lr
    push {r4, r5, r6, r7, r8, r9, sl, fp, lr}
    str	sp, [r0]
    mov	sp, r1
    pop	{r4, r5, r6, r7, r8, r9, sl, fp, lr}
    bx	lr
    
@ [Make sure you can answer: why do we need to do this?]
@
@ use this to setup each thread for the first time.
@ setup the stack so that when cswitch runs it will:
@	- load address of <rpi_init_trampoline> into LR
@	- <code> into r1, 
@	- <arg> into r0
@ 
MK_FN(rpi_init_trampoline)
    mov	r0, r4
    blx	r5
    mov	r0, #0
    bl  rpi_exit