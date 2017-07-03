#ifndef __FIX_DACR__
#define __FIX_DACR__

#include <linux/dik/myprint.h>
#include <linux/dik/interrupt.h>
#include <linux/dik/domain.h>
#include <asm/domain.h>

     /*domain_val(DOMAIN_KERNEL, DOMAIN_MANAGER) | \*/
#ifdef CONFIG_DIK_USE_THREAD
#define EXIT_DACR \
    (domain_val(DOMAIN_USER, DOMAIN_MANAGER) |   \
     domain_val(DOMAIN_IO, DOMAIN_CLIENT) |      \
     domain_val(DOMAIN_EXTENSION, DOMAIN_MANAGER))
#else
#define EXIT_DACR \
    (domain_val(DOMAIN_USER, DOMAIN_MANAGER) |   \
     domain_val(DOMAIN_KERNEL, DOMAIN_MANAGER) | \
     domain_val(DOMAIN_IO, DOMAIN_CLIENT) |      \
     domain_val(DOMAIN_EXTENSION, DOMAIN_MANAGER))
#endif
#define ENTRY_DACR   (KERNEL_DACR)

#define     INT_WIDE(nb)    (nb & 0xffff)
#define     INT_TOP(nb)     ((nb >> 16) & 0xffff)

#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)
#define GATE(DACR_VALUE, label) \
    asm volatile goto(          \
        "push    {r8, r9}\n\t"  \
        "movw    r8, #" STR(INT_WIDE(DACR_VALUE)) "\n\t"    \
        "movt    r8, #" STR(INT_TOP(DACR_VALUE)) "\n\t"     \
        "MCR     p15, 0, r8, c3, c0, 0\n\t"                 \
        "movw    r9, #" STR(INT_WIDE(DACR_VALUE)) "\n\t"    \
        "movt    r9, #" STR(INT_TOP(DACR_VALUE)) "\n\t"     \
        "cmp     r8, r9\n\t"    \
        "pop     {r8, r9}\n\t"  \
        "bne     %l0"           \
        :::: label);


/* Modify the cpu_domain field in the thread_info structure?
 * For scheduling handling. But wouldn't really work because it implies the base
 * kernel would have to close itself at some point.
 */
    /*reenable_interrupt();       \*/
#define entry_gate(label)       \
    GATE(ENTRY_DACR, label);    \
    isb()

void exit_gate(void) {
    dbg_pr("Loading EXIT value in DACR: 0x%x.\n", EXIT_DACR);

    //disable_interrupt();
    GATE(EXIT_DACR, exit_label);
    isb();

    return;

exit_label:
    exit_gate();
}

#endif
