#include <asm/pgtable.h>
#include <asm/domain.h>
#include <linux/dik/myprint.h>

/********** Domain Value in Register *************/

size_t read_DACR(void) {
    size_t dacr_value;
    asm volatile("MRC p15, 0, %0, c3, c0, 0" : "=r" (dacr_value) :);
    dbg_pr("dikernel/darc.c: Current dacr value of DACR %8x.\n", dacr_value);
    return dacr_value;
}

size_t compute_DACR(size_t domain, size_t type, struct thread_info *info) {
    size_t new_dacr, dacr;
    dacr = info->cpu_domain;
    new_dacr = dacr & ~(DOMAIN_MANAGER << (domain * 2));
    new_dacr |= (type << (domain * 2));
    return new_dacr;
}

void write_cpu_domain(size_t domain, size_t type) {
    size_t new_dacr;
    struct thread_info *info;
    if(domain > 15 || type > DOMAIN_MANAGER)
        return;

    info = current_thread_info();
    new_dacr = compute_DACR(domain, type, info);
    info->cpu_domain = new_dacr;
    set_domain(new_dacr);

    dbg_pr("dikernel/darc.c: thread->cpu_domain: 0x%x.\n",
            (current_thread_info())->cpu_domain);
}

/********** Domain ID in page tables *************/

#define DOMAIN_ID_POS   5
#define DOMAIN_ID_MASK  ~(0xf << DOMAIN_ID_POS)

unsigned int get_domain_id(unsigned int pmd) {
    return (pmd >> DOMAIN_ID_POS) & 0xf;
}

void set_domain_id(pmd_t * pmd_p, unsigned int id) {
    if(id < 16)
        *pmd_p = ((*pmd_p) & DOMAIN_ID_MASK) | (id << DOMAIN_ID_POS);
    else
        dbg_pr("dikernel/dacr.c: set_domain_id: domain id should be between 0"
                " and 16.\n");
}

void modify_domain_id(unsigned int * descriptor, unsigned int id) {
    unsigned int domain_id;
    size_t desc_content = *descriptor;
    domain_id = get_domain_id((unsigned int) desc_content);
    dbg_pr("descriptor: 0x%x - current domain id: %d - new domain id: %i.\n",
        desc_content, domain_id, id);
    set_domain_id(descriptor, id);
    dbg_pr("modified descriptor: 0x%x.\n", *descriptor);
}
