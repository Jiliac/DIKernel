#include <asm/pgtable.h>
#include <asm/domain.h>

/********** Domain Value in Register *************/

size_t read_DACR(void) {
        size_t dacr_value;
        printk("dikernel/darc.c: Trying to read the Domain Access Control Register.\n");
        asm volatile("MRC p15, 0, %0, c3, c0, 0" : "=r" (dacr_value) :);
        printk("dikernel/darc.c: Current dacr value of DACR %8x.\n", dacr_value);
        return dacr_value;
}

void write_DACR(size_t domain, size_t type) {
        int arg;
        if(domain > 15 || type > 3)
                return;
        arg = read_DACR() & ~(3 << (domain * 2));
        arg |= (type << (domain * 2));
        printk("dikernel/darc.c: new DACR value: %8x\n", arg);
        // instruction do write the register
        set_domain(arg);
}

/********** Domain ID in page tables *************/

unsigned int get_domain_id(unsigned int pmd) {
    return (pmd >> 5) & 0xf;
}

#define DOMAIN_ID_POS   5
#define DOMAIN_ID_MASK  ~(0xf << DOMAIN_ID_POS)
void set_domain_id(pmd_t * pmd_p, unsigned int id) {
    if(id < 16)
        *pmd_p = ((*pmd_p) & DOMAIN_ID_MASK) | (id << DOMAIN_ID_POS);
    else
        printk("dikernel/dacr.c: set_domain_id: domain id should be between 0"
            " and 16.\n");
}

void modify_domain_id(unsigned int * descriptor, unsigned int id) {
    int domain_id;
    domain_id = get_domain_id((unsigned int) *descriptor);
    printk("descriptor: 0x%x - current domain id: %i - new domain id: %i.\n", *descriptor,
        domain_id, id);
    set_domain_id(descriptor, id);
    printk("modified descriptor: 0x%x.\n", *descriptor);
}
