void change_stack_domain_id(unsigned int new_id);

#define read_sp(sp) \
    asm volatile("mov %0, r13" : "=r" (sp) :)

#define write_sp(new_sp)    \
    asm volatile("mov r13, %0" : : "r" (new_sp))

extern size_t print_sp(void);
