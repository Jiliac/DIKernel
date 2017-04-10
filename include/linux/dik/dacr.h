#define read_dacr(dacr)     \
    asm volatile("MRC p15, 0, %0, c3, c0, 0" : "=r" (dacr) :)
#define write_dacr(dacr)    \
            asm volatile("MCR p15, 0, %0, c3, c0, 0" : : "r" (dacr))

size_t read_DACR(void);
size_t compute_DACR(size_t domain, size_t type, struct thread_info *info);
void write_cpu_domain(size_t domain, size_t type);
void modify_domain_id(unsigned int * descriptor, unsigned int);
unsigned int get_domain_id(unsigned int descriptor);
