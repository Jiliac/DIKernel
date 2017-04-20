#ifndef _DIK__TABLE_WALK_H
#define _DIK__TABLE_WALK_H

#include <asm/pgtable.h>
#include <asm/pgtable-2level.h> // pgd_t type
void dump(void);
void change_all_ids(unsigned int id);
void read_ttbr(void);
void change_domain_id(unsigned int addr, size_t domain_id);
unsigned int* get_first_lvl(int addr);

#endif
