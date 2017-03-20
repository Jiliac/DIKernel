#define read_sp(sp) \
    asm volatile("mov %0, r13" : "=r" (sp) :)

#define write_sp(new_sp)    \
    asm volatile("mov r13, %0" : : "r" (new_sp))

void * allocate_stack(unsigned int stack_size, unsigned int id);
void change_stack_test(void);
