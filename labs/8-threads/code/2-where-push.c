// what address does the first stack operation work on?
#include "rpi.h"
#include "rpi-thread.h"

enum { push_val = 0xdeadbeef };


// implement this assembly routine <0-where-push-asm.S>
// should take a few lines: 
//   - push argument (in r0) onto the stack.
//   - call <after_push> with:
//      - the first argument: sp after you do the push.
//      - second argument: sp before you do the push.
void check_push_asm(uint32_t push_val);

void after_push(uint32_t *sp_after, uint32_t *sp_before) {
    trace("sp_after=%p (val=%x), sp_before=%p (val=%x)\n",
        sp_after, *sp_after, sp_before, *sp_before);

    if(*sp_after == push_val)
        trace("wrote to stack after modifying sp\n");
    else if(*sp_before == push_val)
        trace("wrote to stack before modifying sp\n");
    clean_reboot();
}

void notmain() {
    printk("about to check asm push\n");
    check_push_asm(push_val);
    not_reached();
}


// 000080dc <notmain>:
//     80dc:	e92d4010 	push	{r4, lr}
//     80e0:	e59f0020 	ldr	r0, [pc, #32]	; 8108 <notmain+0x2c>
//     80e4:	eb000238 	bl	89cc <printk>
//     80e8:	e59f001c 	ldr	r0, [pc, #28]	; 810c <notmain+0x30>
//     80ec:	eb00000a 	bl	811c <check_push_asm>
//     80f0:	e3a03020 	mov	r3, #32
//     80f4:	e59f2014 	ldr	r2, [pc, #20]	; 8110 <notmain+0x34>
//     80f8:	e59f1014 	ldr	r1, [pc, #20]	; 8114 <notmain+0x38>
//     80fc:	e59f0014 	ldr	r0, [pc, #20]	; 8118 <notmain+0x3c>
//     8100:	eb000231 	bl	89cc <printk>
//     8104:	eb00042c 	bl	91bc <clean_reboot>
//     8108:	000098c0 	andeq	r9, r0, r0, asr #17
//     810c:	deadbeef 	cdple	14, 10, cr11, cr13, cr15, {7}
//     8110:	0000990c 	andeq	r9, r0, ip, lsl #18
//     8114:	000098dc 	ldrdeq	r9, [r0], -ip
//     8118:	000098ec 	andeq	r9, r0, ip, ror #17

// 0000811c <check_push_asm>:
//     811c:	e1a0100d 	mov	r1, sp
//     8120:	e52d0004 	push	{r0}		; (str r0, [sp, #-4]!)
//     8124:	e1a0000d 	mov	r0, sp
//     8128:	ebffffc1 	bl	8034 <after_push>
//     812c:	e1a0000f 	mov	r0, pc
//     8130:	eb00000b 	bl	8164 <asm_not_reached>
