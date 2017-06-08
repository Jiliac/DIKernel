#ifndef __FIX_DACR__
#define __FIX_DACR__

#include <linux/dik/myprint.h>
#include <linux/dik/domain.h>
#include <asm/domain.h>

//#define ENTRY_DACR   0x555555dc 
//#define EXIT_DACR   0x555555dc 
#define EXIT_DACR \
    (domain_val(DOMAIN_EXTENSION, DOMAIN_MANAGER) |  \
     domain_val(DOMAIN_USER, DOMAIN_MANAGER) |   \
     domain_val(DOMAIN_KERNEL, DOMAIN_MANAGER) | \
     domain_val(DOMAIN_IO, DOMAIN_CLIENT) |      \
     domain_val(DOMAIN_PUBLIC, DOMAIN_MANAGER))
#define ENTRY_DACR   (KERNEL_DACR)


void entry_gate(void) {
    int reg;
    do {
        asm volatile("mov r6, %0" :: "r" (ENTRY_DACR));
        asm volatile("MCR p15, 0, r6, c3, c0, 0" ::);
        asm volatile("mov %0, r6" : "=r" (reg):);
    } while(reg != ENTRY_DACR);
    dbg_pr("entry_gate: r6 (that has been loaded in dacr): %x", reg);
    asm volatile("MRC p15, 0, %0, c3, c0, 0" : "=r" (reg) :);
    dbg_pr(" - DACR value: %x\n", reg);
}

void exit_gate(void) {
    int reg;
    dbg_pr("Loading EXIT value in DACR: 0x%x.\n", EXIT_DACR);
    do {
        asm volatile("mov r6, %0" :: "r" (EXIT_DACR));
        asm volatile("MCR p15, 0, r6, c3, c0, 0" ::);
        asm volatile("mov %0, r6" : "=r" (reg):);
    } while(reg != EXIT_DACR);
    /* Be careful, these prints won't work once the base kernel is closed.
     */
    dbg_pr("exit_gate: r6 (that has been loaded in dacr): %x", reg);
    asm volatile("MRC p15, 0, %0, c3, c0, 0" : "=r" (reg) :);
    dbg_pr(" - DACR value: %x\n", reg);
}
#endif
