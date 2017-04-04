size_t read_DACR(void);
size_t compute_DACR(size_t domain, size_t type, struct thread_info *info);
void write_DACR(size_t domain, size_t type);
void modify_domain_id(unsigned int * descriptor, unsigned int);
unsigned int get_domain_id(unsigned int descriptor);
