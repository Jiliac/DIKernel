#include "table_walk.h"
#include <linux/dik/dacr.h>
#include <linux/dik/domain.h>
#include <linux/dik/myprint.h>

/**************************************************************************/
void dump(void) {
    pgd_t * pgd;
    pmd_t *pmd;
    int i;

    pgd = init_mm.pgd;
    for (i = 0; i < PTRS_PER_PGD; i++, pgd++) {
        pmd = pmd_offset(pud_offset(pgd, 0), 0);
        if(pmd[0] != 0 || pmd[1] != 0)
            dbg_pr("pgd[%i]=%p: pmd[0]: 0x%x, pmd[1]: 0x%x.\n",
                    i, pgd, pmd[0], pmd[1]);
    }
}

//void read_ttbr(void) {
//    unsigned int ttbr0, ttbr1;
//    int miscellanious;
//    asm volatile("MRRC p15, 0, %0, %1, c2" : "=r" (ttbr0), "=r" (miscellanious) : );
//    dbg_pr("ttbr0: %8x.\n", ttbr0);
//    asm volatile("MRRC p15, 1, %0, %1, c2" : "=r" (ttbr1), "=r" (miscellanious) : );
//    dbg_pr("ttbr1: %8x.\n", ttbr1);
//
//    return;
//}

/**************************** Miscellanious *********************************/
unsigned int get_pmd_bit(unsigned long addr) {
    return (addr >> 20) & 1;
}
/****************************************************************************/

#define     FIRST_LVL_SHIFT 20
#define     FIRST_LVL_SIZE  (1UL << FIRST_LVL_SHIFT)
#define     FIRST_LVL_MASK  (FIRST_LVL_SIZE-1)
unsigned int* get_first_lvl(unsigned int addr) {
    unsigned int ttbr0, miscellanious;
    unsigned int *pgd;
    unsigned int *first_lvl_descriptor_addr;

    asm volatile("MRRC p15, 0, %0, %1, c2" : "=r" (ttbr0), 
        "=r" (miscellanious) : );
    pgd = (unsigned int*) ((ttbr0 & 0xfffff000) + PAGE_OFFSET);

    first_lvl_descriptor_addr = pgd + ((addr & 0xfff00000) >> FIRST_LVL_SHIFT);
    dbg_pr("addr: 0x%x, pgd: %p, first_lvl_descriptor_addr: %p.\n",
            addr, pgd, first_lvl_descriptor_addr);
    return first_lvl_descriptor_addr;
}

/* Memory is supposed contiguous between addr and addr+size. */
void change_domain_id(unsigned int addr, size_t domain_id, unsigned int size) {
    int i;
    unsigned int *first_lvl_descriptor_addr;
    unsigned int first_lvl_max_index;
    size = (size*4) + (addr & FIRST_LVL_MASK);
    first_lvl_max_index = (size / FIRST_LVL_SIZE) + 1;
    dbg_pr("size: 0x%x - 1st lvl index: %d.\n", size, first_lvl_max_index);
    //first_lvl_max_index = (size*4 / FIRST_LVL_SIZE) + 1;
    for(i = 0; i < first_lvl_max_index; ++i) {
        first_lvl_descriptor_addr = get_first_lvl(addr);
        modify_domain_id(first_lvl_descriptor_addr, domain_id);
        addr += FIRST_LVL_SIZE;
    }
    dbg_pr("change_domain_id: addr 0x%x covers %d different PDE(s).\n",
        addr, first_lvl_max_index);
}
// Used in drivers/public_domain/daehee_test.c. To be Removed. @TODO
EXPORT_SYMBOL(change_domain_id);

/***************** test on first level descriptor modification **************/
void corrupt_pt(unsigned int addr) {
    unsigned int *first_lvl_descriptor_addr;
    size_t pmd;
    first_lvl_descriptor_addr = get_first_lvl(addr);
    pmd = *first_lvl_descriptor_addr;
    printk("first level decriptor before modification: 0x%x.\n", pmd);
    *first_lvl_descriptor_addr = pmd & (~0x3);
    printk("first level decriptor after modification: 0x%x.\n",
        *first_lvl_descriptor_addr);
}


/**************************************************************************/
#ifdef CONFIG_VIRTUAL_BK_DID
/* That was in case there is a problem specifically with domain 0.
 * Maybe something hard coded in the hardware.
 * The whole base kernel would be transfer to another domain.
 */

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
        dbg_pr("Domain ID should be between 0 and 15.\n");

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
#endif // CONFIG_VIRTUAL_BK_DID
/**************************************************************************/


