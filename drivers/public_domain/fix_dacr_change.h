#ifndef __FIX_DACR__
#define __FIX_DACR__

#include <linux/dik/myprint.h>

#define TO_D4   0x555555dc 
#define TO_D5   0x555555dc 
#define TO_D6   0x555555dc 

void change_dacr_to_d4(void) {
    asm volatile("MCR p15, 0, %0, c3, c0, 0" : : "r" (TO_D4));
}

void switch_dacr_to_domain(size_t domain) {
    switch(domain) {
        case 4:
            change_dacr_to_d4();
            break;
        default:
            dbg_pr("No action defined for domain %d.\n", domain);
    }
}

#endif
