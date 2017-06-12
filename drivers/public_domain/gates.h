#ifndef __FIX_DACR__
#define __FIX_DACR__

#include <linux/dik/myprint.h>
#include <linux/dik/domain.h>
#include <asm/domain.h>

#define EXIT_DACR \
    (domain_val(DOMAIN_EXTENSION, DOMAIN_MANAGER) |  \
     domain_val(DOMAIN_USER, DOMAIN_MANAGER) |   \
     domain_val(DOMAIN_KERNEL, DOMAIN_MANAGER) | \
     domain_val(DOMAIN_IO, DOMAIN_CLIENT) |      \
     domain_val(DOMAIN_PUBLIC, DOMAIN_MANAGER))
#define ENTRY_DACR   (KERNEL_DACR)

#define     INT_WIDE(nb)    (nb & 0xffff)
#define     INT_TOP(nb)     ((nb >> 16) & 0xffff)

#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)
#define GATE(DACR_VALUE, label) \
    asm volatile goto(          \
        "movw    r6, #" STR(INT_WIDE(DACR_VALUE)) "\n\t"    \
        "movt    r6, #" STR(INT_TOP(DACR_VALUE)) "\n\t"     \
        "MCR     p15, 0, r6, c3, c0, 0\n\t"                 \
        "movw    r7, #" STR(INT_WIDE(DACR_VALUE)) "\n\t"    \
        "movt    r7, #" STR(INT_TOP(DACR_VALUE)) "\n\t"     \
        "cmp     r6, r7\n\t"    \
        "bne     %l0"           \
        : :                     \
        : : label)

void entry_gate(void) {
    GATE(ENTRY_DACR, entry_label);
    return;

entry_label:
    entry_gate();
}

void exit_gate(void) {
    int reg;
    dbg_pr("Loading EXIT value in DACR: 0x%x.\n", EXIT_DACR);

    GATE(EXIT_DACR, exit_label);

    asm volatile("MRC p15, 0, %0, c3, c0, 0" : "=r" (reg) :);
    dbg_pr("Current value in DACR: 0x%x\n", reg);

    return;

exit_label:
    exit_gate();
}
#endif
