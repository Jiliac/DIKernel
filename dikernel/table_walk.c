#include <linux/vmalloc.h>
#include <asm/pgtable.h>
#include <asm/pgtable-2level.h> // for PTRS_PER_PGD
#include <asm/memory.h>     // virt_to_phys function
#include <asm/proc-fns.h>   // cpu_get_ttbr or pgd macros
#include "table_walk.h"
#include "dacr.h"

// Different available types of page table
char * pt_type[] = {"Fault", "Page Table Base", "Section", "Reserved"};

void read_ttbr(void) {
    unsigned int ttbr0, ttbr1;
    int miscellanious;
    // I believe for previous ARM version
    //asm volatile("MRC p15, 0, %0, c2, c0, 0" : "=r" (ttbr) : ); // ttbr0
    //asm volatile("MRC p15, 0, %0, c2, c0, 1" : "=r" (ttbr) : ); // ttbr1
    asm volatile("MRRC p15, 0, %0, %1, c2" : "=r" (ttbr0), "=r" (miscellanious) : );
    printk("ttbr0: %x, miscellanious: %x\n", ttbr0, miscellanious);
    asm volatile("MRRC p15, 1, %0, %1, c2" : "=r" (ttbr1), "=r" (miscellanious) : );
    printk("ttbr1: %x, miscellanious: %x\n", ttbr1, miscellanious);

    printk("pointed by ttbr0: %x\n", *((unsigned int*) (ttbr0&0x3fff)));

    return;
}

/****************************************************************************
 *****************************************************************************
 ********************* Recursive Page Tables Walking *************************
 *********************** If allocated with vmalloc ***************************
 *****************************************************************************
 ****************************************************************************/
void walk_pmd(pud_t *pud, unsigned long addr, unsigned long end, unsigned int id){
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
        modify_domain_id_in_pmd(*pmd, id);
    } while(pmd++, addr = next, addr != end);
    isb();
}

void walk_pud(pgd_t *pgd, unsigned long addr, unsigned long end, unsigned int id){
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

void modify_domain_id(long unsigned int addr, unsigned int id) {
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
