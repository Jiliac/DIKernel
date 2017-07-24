#ifndef _DIK__TABLE_WALK_H
#define _DIK__TABLE_WALK_H

#include <asm/pgtable.h>
#include <asm/pgtable-2level.h> // pgd_t type
void dump(void);
void change_all_ids(unsigned int id);
void change_kernel_domain(void);
void read_ttbr(void);

void change_domain_id(unsigned int addr, size_t domain_id, unsigned int size);
unsigned int* get_first_lvl(unsigned int addr);

void corrupt_pt(unsigned int addr);

#endif
