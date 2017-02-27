#include<linux/slab.h>

void * wrapper__kmalloc(size_t size, gfp_t gfp);
void modify_symbol(char * target_name, unsigned long new_value);
void setting_wrappers(void);
