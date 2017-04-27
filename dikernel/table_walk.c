#include <linux/sched.h>    // for init_mm
#include "table_walk.h"
#include <linux/dik/dacr.h>
#include <linux/dik/domain.h>

/**************************************************************************/
void dump(void) {
    pgd_t * pgd;
    pmd_t *pmd;
    int i;

    pgd = init_mm.pgd;
    for (i = 0; i < PTRS_PER_PGD; i++, pgd++) {
        pmd = pmd_offset(pud_offset(pgd, 0), 0);
        if(pmd[0] != 0 || pmd[1] != 0)
            printk("pgd[%i]=%p: pmd[0]: 0x%x, pmd[1]: 0x%x.\n",
                    i, pgd, pmd[0], pmd[1]);
    }
}

#include <asm/domain.h>
void change_kernel_domain(void) {
    pgd_t * pgd;
    pmd_t *pmd;
    int i;

    pgd = init_mm.pgd;
    for (i = 0; i < PTRS_PER_PGD; i++, pgd++) {
        unsigned domain_id;
        pmd = pmd_offset(pud_offset(pgd, 0), 0);
        domain_id = get_domain_id(*pmd);
        if(domain_id == DOMAIN_KERNEL) {
            if(pmd[0] != 0)
               modify_domain_id((unsigned int*) pmd, DOMAIN_KERNEL_VIRTUAL);
            if(pmd[1] != 0)
                modify_domain_id((unsigned int*) pmd+1, DOMAIN_KERNEL_VIRTUAL);
        }
    }
}
EXPORT_SYMBOL(change_kernel_domain);

void change_all_ids(unsigned int id) {
    pgd_t * pgd;
    pmd_t *pmd;
    int i;

    if(id > 15)
        printk("Domain ID should be between 0 and 15.\n");

    pgd = init_mm.pgd;
    for (i = 0; i < PTRS_PER_PGD; i++, pgd++) {
        pmd = pmd_offset(pud_offset(pgd, 0), 0);
        if(pmd[0] != 0)
            modify_domain_id((unsigned int*) pmd, id);
        if(pmd[1] != 0)
            modify_domain_id((unsigned int*) pmd+1, id);
    }

}
EXPORT_SYMBOL(change_all_ids);
/**************************************************************************/

void read_ttbr(void) {
    unsigned int ttbr0, ttbr1;
    int miscellanious;
    asm volatile("MRRC p15, 0, %0, %1, c2" : "=r" (ttbr0), "=r" (miscellanious) : );
    printk("ttbr0: %8x.\n", ttbr0);
    asm volatile("MRRC p15, 1, %0, %1, c2" : "=r" (ttbr1), "=r" (miscellanious) : );
    printk("ttbr1: %8x.\n", ttbr1);

    return;
}

unsigned int get_ttbr0(void) {
    unsigned int ttbr0;
    int miscellanious;
    asm volatile("MRRC p15, 0, %0, %1, c2" : "=r" (ttbr0), "=r" (miscellanious) : );
    return ttbr0;
}

/**************************** Miscellanious *********************************/
unsigned int get_pmd_bit(unsigned long addr) {
    return (addr >> 20) & 1;
}
/****************************************************************************/

#define     FIRST_LVL_SHIFT 20
unsigned int* get_first_lvl(unsigned int addr) {
    unsigned int *pgd;
    unsigned int *first_lvl_descriptor_addr;

    pgd = (unsigned int*) (init_mm.pgd);
    first_lvl_descriptor_addr = pgd + ((addr & 0xfff00000) >> FIRST_LVL_SHIFT);
    printk("addr: 0x%x, pgd: %p, first_lvl_descriptor_addr: %p.\n",
            addr, pgd, first_lvl_descriptor_addr);
    return first_lvl_descriptor_addr;
}

void change_domain_id(unsigned int addr, size_t domain_id) {
    unsigned int *first_lvl_descriptor_addr;
    first_lvl_descriptor_addr = get_first_lvl(addr);
    modify_domain_id(first_lvl_descriptor_addr, domain_id);
}
