#ifndef _WRAPPER_H
#define _WRAPPER_H

struct data_wrapper_info {
    char *data_sym_name;
    unsigned long new_value;
    void (*init_wrapper) (void);
    void (*update_wrapper) (void);
    void (*delete_wrapper) (void);
};

struct data_wrapper_info data_wrapper_info[] = {
    {"kmalloc_caches",  (unsigned long) wrapper_kmalloc_caches,
        wrapper_kmalloc_caches_init,    wrapper_kmalloc_caches_update,
        wrapper_kmalloc_caches_exit},
};

#endif
