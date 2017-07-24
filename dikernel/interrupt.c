#include <linux/dik/myprint.h>
#include <linux/kernel.h>

void disable_interrupt(void) {
    size_t cpsr=0;

    // reading
    asm("mrs %0, cpsr" :  "=r" (cpsr));
    dbg_pr("CPSR: 0x%x.\n", cpsr);

    // modifying
    cpsr = cpsr | 0x1c0;
    dbg_pr("New CPSR: 0x%x.\n", cpsr);
    asm("msr cpsr_cxsf, %0" :: "r" (cpsr));

#ifdef DEBUG
    // reading again
    asm("mrs %0, cpsr" :  "=r" (cpsr));
    dbg_pr("CPSR after update: 0x%x.\n", cpsr);
#endif
}
EXPORT_SYMBOL(disable_interrupt);

void reenable_interrupt(void) {
    size_t cpsr=0;

    // reading
    asm("mrs %0, cpsr" :  "=r" (cpsr));
    dbg_pr("CPSR: 0x%x.\n", cpsr);

    // modifying
    cpsr = cpsr & ~0x1c0;
    dbg_pr("New CPSR: 0x%x.\n", cpsr);
    asm("msr cpsr_cxsf, %0" :: "r" (cpsr));

#ifdef DEBUG
    // reading again
    asm("mrs %0, cpsr" :  "=r" (cpsr));
    dbg_pr("CPSR after update: 0x%x.\n", cpsr);
#endif
}
EXPORT_SYMBOL(reenable_interrupt);
