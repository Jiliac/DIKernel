#ifndef _DIK__TABLE_WALK_H
#define _DIK__TABLE_WALK_H

#include <asm/pgtable.h>
#include <asm/pgtable-2level.h> // pgd_t type
void modify_domain_id_coarsepg(long unsigned int addr, unsigned int id);
void read_ttbr(void);
pgd_t* get_section_base_addr(unsigned long addr);
void modify_section_domain_id(unsigned long addr, unsigned int id);

#endif
