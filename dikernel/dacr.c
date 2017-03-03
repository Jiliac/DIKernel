#include <asm/pgtable.h>
#include <asm/domain.h>

unsigned int get_domain_id(pmd_t pmd) {
    return (pmd >> 5) & 0xf;
}

#define DOMAIN_ID_MASK ~(0xf << 5)
void set_domain_id(pmd_t * pmd_p, unsigned int id) {
    if(id < 16)
        *pmd_p = ((*pmd_p) & DOMAIN_ID_MASK) | (id << 5);
    else
        printk("dikernel/dacr.c: set_domain_id: domain id should be between 0"
            " and 16.\n");
}

void modify_domain_id_in_pmd(pmd_t pmd, unsigned int id) {
    int domain_id;
    domain_id = get_domain_id(pmd);
    printk("pmd: 0x%x - new domain id: %i.\n", pmd, domain_id);
    set_domain_id(&pmd, id);
    printk("modified pmd: 0x%x.\n", pmd);
}
