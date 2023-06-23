#include "vm-ident.h"
#include "libc/bit-support.h"

volatile struct proc_state proc;

// this is called by reboot: we turn off mmu so that things work.
void reboot_callout(void) {
    if(mmu_is_enabled())
        staff_mmu_disable();
}

void prefetch_abort_vector(unsigned lr) {
    unimplemented();
}

void data_abort_vector(unsigned lr) {
    // unimplemented();
    // b4-43
    unsigned dfsr = dfsr_get();
    unsigned fault_addr = far_get();
    printk("the dfsr_get() is %x\n", dfsr_get());
    printk("the far_get() is %x\n", far_get());

    unsigned dfsr_reason = (dfsr & 15) | ((dfsr >> 6) & 16);

    switch (dfsr_reason)
    {
    case 0b00101/* constant-expression */:
        /* code */
        // unsigned new_section = (fault_addr >> 20) << 20;
        // staff_mmu_map_section(pt, new_section - OneMB, new_section - OneMB, dom_id);
        // staff_mmu_sync_pte_mods();
        break;
    case 0b1101/* constant-expression */:
        /* code */
        
        break;
    default:
        break;
    }
    



    // staff_mmu_sync_pte_mods();
}
