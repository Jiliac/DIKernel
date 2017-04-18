#include <linux/vmalloc.h>
#include <asm/memory.h>     // virt_to_phys function
#include <asm/proc-fns.h>   // cpu_get_ttbr or pgd macros
#include <linux/sched.h>    // for init_mm
#include "table_walk.h"
#include <linux/dik/dacr.h>

/**************************************************************************/
void dump(void) {
    pgd_t * pgd;
    pmd_t *pmd;
    int i;

    pgd = init_mm.pgd;
    for (i = 0; i < PTRS_PER_PGD; i++, pgd++) {
        pmd = pmd_offset(pud_offset(pgd, 0), 0);
        if(pmd[0] != 0 && pmd[1] != 0)
            printk("pgd[%i]: pmd[0]: 0x%x, pmd[1]: 0x%x.\n", i, pmd[0], pmd[1]);
    }
}
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

/*****************************************************************************
*************************** "Section" Addressing *****************************
*****************************************************************************/

pgd_t* get_section_base_addr(unsigned long addr) {
    pgd_t * pgd;
    pgd_t * section_base_addr;
    
    pgd = init_mm.pgd;
    section_base_addr = pgd + (addr >> PGDIR_SHIFT);
    //printk("get_section_base_addr: addr: 0x%lx, pgd %p, section_base_addr %p.\n",
    //    addr, pgd, section_base_addr);
    return section_base_addr;
}

void modify_section_domain_id(unsigned long addr, size_t id) {
    pgd_t * section_base_addr;
    unsigned int section_base;

    if(id > 15)
        printk("Domain ID should be between 0 and 15.\n");

    section_base_addr = get_section_base_addr(addr);
    section_base = *((unsigned int*) section_base_addr);
    //printk("Current domain of 0x%8x section is %i.\n", section_base,
    //    get_domain_id(section_base));
    modify_domain_id((unsigned int*) section_base_addr, id);
    modify_domain_id((unsigned int*) section_base_addr + 1, id);
}

/*****************************************************************************
 ********************* Recursive Page Tables Walking *************************
 *********************** If allocated with vmalloc ***************************
 ****************** i.e. with "normal" page table adressing ******************
 ****************************************************************************/
void walk_pmd(pud_t *pud, unsigned long addr, unsigned long end, size_t id){
    pmd_t *pmd;
    unsigned long next;
    pmd = pmd_offset(pud, addr);
    do {
        next = pmd_addr_end(addr, end);
        if(pmd_none_or_clear_bad(pmd)){
            continue;
        }
        printk("dikernel/table_walk: pmd address %p - pmd value %x\n",
            pmd, *pmd);
        modify_domain_id((unsigned int*) pmd, id);
        modify_domain_id((unsigned int*) pmd + 1, id);
    } while(pmd++, addr = next, addr != end);
    isb();
}

void walk_pud(pgd_t *pgd, unsigned long addr, unsigned long end, size_t id){
    pud_t *pud;
    unsigned long next;
    pud = pud_offset(pgd, addr);
    do {
        next = pud_addr_end(addr, end);
        if(pud_none_or_clear_bad(pud))
            continue;
        walk_pmd(pud, addr, next, id);
    } while(pud++, addr = next, addr != end);
}

void modify_domain_id_coarsepg(long unsigned int addr, size_t id) {
    pgd_t *pgd;
    struct vmap_area *va;
    unsigned long end;
    unsigned long next;

    pgd = pgd_offset_k(addr);
    va = find_vmap_area((unsigned long) addr);
    end = va->va_end;
    do {
        next = pgd_addr_end(addr, end);
        if(pgd_none_or_clear_bad(pgd))
            continue;
        walk_pud(pgd, addr, next, id);
    } while(pgd++, addr = next, addr != end);
}
