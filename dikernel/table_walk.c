#include <linux/vmalloc.h>
#include <asm/pgtable.h>
#include <asm/pgtable-2level.h> // for PTRS_PER_PGD
#include <asm/memory.h>     // virt_to_phys function
#include <asm/proc-fns.h>   // cpu_get_ttbr or pgd macros
#include "table_walk.h"

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

void dump_pmds_k(void) {
    int i;
    pgd_t * pgd;
    pgd = init_mm.pgd;
    printk("pgd from init_mm: %p - pgd physical location: %x\n", pgd, virt_to_phys(pgd));
    for(i=0; i < PTRS_PER_PGD; i++) {
        pmd_t pmd = *(pgd[i]);
        int domain, pt_type_nb;
        if (pmd != 0){
            domain = (pmd >> 5) & 0xf;
            pt_type_nb = pmd & 3;
            printk("address: %p - value: %x\n", &(pgd[i]), pmd);
            printk("\tdomain id: %x - page table type: %s\n", domain,
                pt_type[pt_type_nb]);
        }
    }
    return;
}

/****************************************************************************
 *****************************************************************************
 ********************* Recursive Page Tables Walking *************************
 *********************** If allocated with vmalloc ***************************
 *****************************************************************************
 ****************************************************************************/
void walk_pmd(pud_t *pud, unsigned long addr, unsigned long end){
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
    } while(pmd++, addr = next, addr != end);
    isb();
}

void walk_pud(pgd_t *pgd, unsigned long addr, unsigned long end){
    pud_t *pud;
    unsigned long next;
    pud = pud_offset(pgd, addr);
    do {
        next = pud_addr_end(addr, end);
        if(pud_none_or_clear_bad(pud))
            continue;
        walk_pmd(pud, addr, next);
    } while(pud++, addr = next, addr != end);
}

void walk_pgd(long unsigned int addr) {
    pgd_t *pgd;
    struct vmap_area *va;
    unsigned long end;
    unsigned long next;
    printk("dikernel/table_walk: address %lx, PGDIR_SHIFT %d, pgd %p\n", addr,
    PGDIR_SHIFT, (&init_mm)->pgd);

    pgd = pgd_offset_k(addr);
    // Should I "get_vmalloc_info" instead?
    va = find_vmap_area((unsigned long) addr);
    end = va->va_end;
    do {
        next = pgd_addr_end(addr, end);
        if(pgd_none_or_clear_bad(pgd))
            continue;
        printk("dikernel/table_walk: pgd address %p\n", pgd);
        walk_pud(pgd, addr, next);
    } while(pgd++, addr = next, addr != end);
}
