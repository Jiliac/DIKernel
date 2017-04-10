#include <linux/vmalloc.h>
#include <asm/memory.h>     // virt_to_phys function
#include <asm/proc-fns.h>   // cpu_get_ttbr or pgd macros
#include <linux/sched.h>    // for init_mm
#include "table_walk.h"
#include <linux/dik/dacr.h>

// Different available types of page table
char * pt_type[] = {"Fault", "Page Table Base", "Section", "Reserved"};

void read_ttbr(void) {
    unsigned int ttbr0, ttbr1;
    int miscellanious;
    // I believe for previous ARM version
    //asm volatile("MRC p15, 0, %0, c2, c0, 0" : "=r" (ttbr) : ); // ttbr0
    //asm volatile("MRC p15, 0, %0, c2, c0, 1" : "=r" (ttbr) : ); // ttbr1
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
    printk("section base: 0x%8x.\n", *((unsigned int*) section_base_addr));
    return section_base_addr;
}

void modify_section_domain_id(unsigned long addr, size_t id) {
    pgd_t * section_base_addr;
    unsigned int section_base;

    if(id > 15)
        printk("Domain ID should be between 0 and 15.\n");

    section_base_addr = get_section_base_addr(addr);
    section_base = *((unsigned int*) section_base_addr);
    printk("Current domain of 0x%8x section is %i.\n", section_base,
        get_domain_id(section_base));
    modify_domain_id((unsigned int*) section_base_addr, id);
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
