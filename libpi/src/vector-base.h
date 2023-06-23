#ifndef __VECTOR_BASE_SET_H__
#define __VECTOR_BASE_SET_H__
#include "libc/bit-support.h"
#include "asm-helpers.h"

/*
 * vector base address register:
 *   3-121 --- let's us control where the exception jump table is!
 *
 * defines: 
 *  - vector_base_set  
 *  - vector_base_get
 */

static inline void *vector_base_get(void) {
    // // unimplemented();
    // unsigned _out;
    // // MRC p15, 0, <Rd>, c12, c0, 0
  	// asm volatile ("MRC p15, 0, %0, c12, c0, 0" : "=r"(_out));
    // return (unsigned*)_out;

// MRC p15, 0, <Rd>, c12, c0, 1
    uint32_t val;
    __asm__ volatile("mrc p15, 0, %0, c12, c0, 0" : "=r"(val));
    // printk("the val is %x\n", val);
    return (void *)val;

}

// set the vector register to point to <vector_base>.
// must: 
//    - check that it satisfies the alignment restriction.
static inline void vector_base_set(void *vector_base) {
    // unimplemented();
    // MCR p15, 0, <Rd>, c12, c0, 0
    // unsigned* in = vector_base_get();
    // printk("the vector_base is %x\n", vector_base);
    // assert(in == vector_base);             
    assert(vector_base);      
    uint32_t v = (uint32_t)vector_base;              
    if(bits_get(v, 0, 4) != 0)
        panic("lower 5 bits of the vector base should be 0, have: %x\n", v);

    asm volatile("mcr p15, 0, %0, c12, c0, 0" :: "r"(vector_base)); 
    dev_barrier();
    // printk("+++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    // printk("the vector_base_get() is %x\n", vector_base_get());
    // printk("the vector_base is %x\n", vector_base);
    // printk("+++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    assert(vector_base_get() == vector_base);
}
#endif
