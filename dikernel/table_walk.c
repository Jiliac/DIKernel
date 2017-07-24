#include "table_walk.h"
#include <linux/dik/dacr.h>
#include <linux/dik/domain.h>
#include <linux/dik/myprint.h>

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

void change_domain_id(unsigned int addr, size_t domain_id, unsigned int size) {
    int i;
    unsigned int *first_lvl_descriptor_addr;
    unsigned int first_lvl_max_index;
    size = (size*4) + (addr & FIRST_LVL_MASK);
    first_lvl_max_index = (size / FIRST_LVL_SIZE) + 1;
    dbg_pr("size: 0x%x - 1st lvl index: %d.\n", size, first_lvl_max_index);
    for(i = 0; i < first_lvl_max_index; ++i) {
        first_lvl_descriptor_addr = get_first_lvl(addr);
        modify_domain_id(first_lvl_descriptor_addr, domain_id);
        addr += FIRST_LVL_SIZE;
    }
    dbg_pr("change_domain_id: addr 0x%x covers %d different PDE(s).\n",
        addr, first_lvl_max_index);
}
// Used in drivers/diswitcher/daehee_test.c. To be Removed. @TODO
EXPORT_SYMBOL(change_domain_id);
