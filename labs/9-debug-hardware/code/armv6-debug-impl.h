#ifndef __ARMV6_DEBUG_IMPL_H__
#define __ARMV6_DEBUG_IMPL_H__
#include "armv6-debug.h"

// all your code should go here.  implementation of the debug interface.

// example of how to define get and set for status registers
coproc_mk(status, p14, 0, c0, c1, 0)

// you'll need to define these and a bunch of other routines.
// static inline uint32_t cp15_dfsr_get(void);
// static inline uint32_t cp15_ifar_get(void);
// static inline uint32_t cp15_ifsr_get(void);
// static inline uint32_t cp14_dscr_get(void);
// static inline uint32_t cp14_bcr0_get(void);
// static inline uint32_t cp14_wvr0_get(void);
// static inline uint32_t cp14_wfar_get(void);

// static inline uint32_t cp14_dscr_set(uint32_t status);
// static inline uint32_t cp14_wcr0_set(uint32_t r);

coproc_mk(dacr, p15, 0, c3, c0, 0);
coproc_mk(dfsr, p15, 0, c5, c0, 0);
coproc_mk(ifar, p15, 0, c6, c0, 2);
coproc_mk(ifsr, p15, 0, c5, c0, 1);
coproc_mk(far, p15, 0, c6, c0, 0);


coproc_mk(dscr, p14, 0, c0, c1, 0);
coproc_mk(bvr0, p14, 0, c0, c1, 4);
coproc_mk(bcr0, p14, 0, c0, c1, 5);
coproc_mk(wvr0, p14, 0, c0, c1, 6);
coproc_mk(wcr0, p14, 0, c0, c1, 7);
coproc_mk(wfar, p14, 0, c0, c6, 0);

// return 1 if enabled, 0 otherwise.  
//    - we wind up reading the status register a bunch:
//      could return its value instead of 1 (since is 
//      non-zero).
static inline int cp14_is_enabled(void) {
    // unimplemented();
    uint32_t dscr = cp14_dscr_get();
    return bit_is_on(dscr, 15) && bit_is_off(dscr, 14);
}

// enable debug coprocessor 
static inline void cp14_enable(void) {
    // if it's already enabled, just return?
    if(cp14_is_enabled())
        panic("already enabled\n");

    // for the core to take a debug exception, monitor debug mode has to be both 
    // selected and enabled --- bit 14 clear and bit 15 set.
    // unimplemented();
    uint32_t dscr = cp14_dscr_get();
    dscr = bit_set(dscr, 15);
    dscr = bit_clr(dscr, 14);
    cp14_dscr_set(dscr);
    assert(cp14_is_enabled());
}

// disable debug coprocessor
static inline void cp14_disable(void) {
    if(!cp14_is_enabled())
        return;

    // unimplemented();
    uint32_t b = cp14_dscr_get();
    b = bit_clr(b, 15);

    assert(!cp14_is_enabled());
}


static inline int cp14_bcr0_is_enabled(void) {
    // unimplemented();
    return cp14_bcr0_get() & 1;
}

// Write to the BCR with its fields set as follows:
// •BCR[22:21] meaning of BVR bit set to b00 or b10, to indicate that the value loaded
// into BVR is to be compared against the IMVA bus as a match or mismatch.
// •BCR[20] enable linking bit cleared, to indicate that this breakpoint is not to be
// linked.
// •BCR [15:14] Secure access BCR field as required.
// •BCR[8:5] byte address select BCR field as required.
// •BCR[2:1] supervisor access BCR field as required.
// •BCR[0] enable breakpoint bit set.
static inline void cp14_bcr0_enable(void) {
    // unimplemented();
    uint32_t bcr0 = cp14_bcr0_get();
    bcr0 = bits_clr(bcr0, 21, 22);
    bcr0 = bit_clr(bcr0, 20);
    bcr0 = bits_clr(bcr0, 14, 15);
    bcr0 = bits_set(bcr0, 5, 8, 0b1111);
    bcr0 = bits_set(bcr0, 1, 2, 0b11);
    bcr0 = bit_set(bcr0, 0);
    cp14_bcr0_set(bcr0);
}
static inline void cp14_bcr0_disable(void) {
    // unimplemented();
    uint32_t bcr0 = bit_clr(cp14_bcr0_get(), 0); // Clear bit 0 to disable breakpoint
    cp14_bcr0_set(bcr0);
}

// was this a brkpt fault?
static inline int was_brkpt_fault(void) {
    // use IFSR and then DSCR
    // unimplemented();
    uint32_t ifsr = bits_get(cp15_ifsr_get(), 0, 3);
    if (ifsr != 0b0010)
        return 0;
    // DSCR was breakpoint
    uint32_t dscr = bits_get(cp14_dscr_get(), 2, 5);
    if (dscr != 0b0001)
        return 0;
    return 1;
}

// was watchpoint debug fault caused by a load?
static inline int datafault_from_ld(void) {
    return bit_isset(cp15_dfsr_get(), 11) == 0;
}
// ...  by a store?
static inline int datafault_from_st(void) {
    return !datafault_from_ld();
}


// 13-33: tabl 13-23
static inline int was_watchpt_fault(void) {
    // use DFSR then DSCR
    // unimplemented();
    uint32_t dfsr = bits_get(cp15_dfsr_get(), 0, 3);
    if (dfsr != 0b0010)
        return 0;
    // DSCR was watchpoint
    uint32_t dscr = bits_get(cp14_dscr_get(), 2, 5);
    if (dscr != 0b0010)
        return 0;
    return 1;
}

static inline int cp14_wcr0_is_enabled(void) {
    // unimplemented();
    return bit_is_on(cp14_wcr0_get(), 0);
}
static inline void cp14_wcr0_enable(void) {
    uint32_t wcr0 = cp14_wcr0_get();
    // Turn off linking
    wcr0 = bit_clr(wcr0, 20);
    // Watchpoints both in secure and non-secur e
    wcr0 = bits_clr(wcr0, 14, 15);
    // For both loads and stores
    wcr0 = bits_set(wcr0, 3, 4, 0b11);
    // For both privileged and user
    wcr0 = bits_set(wcr0, 1, 2, 0b11);
    // Enable
    wcr0 = bit_set(wcr0, 0);
    // Byte address select for all accesses (0x0, 0x1, 0x2, 0x3)
    //wcr0 = bits_set(wcr0, 5, 8, 0b1111);
    // Put back into wcr0
    cp14_wcr0_set(wcr0);
}
static inline void cp14_wcr0_disable(void) {
    // unimplemented();
    cp14_wcr0_set(cp14_wcr0_get() & ~0b1);
    prefetch_flush();
}

// Get watchpoint fault using WFAR
static inline uint32_t watchpt_fault_pc(void) {
    // unimplemented();
    return cp14_wfar_get() - 8;
}

static inline uint32_t watchpt_fault_addr(void) {
    return cp15_far_get();
}
    
#endif
