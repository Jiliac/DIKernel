#include<linux/init.h>

#define SWITCHER_MODULE "domain_switcher"
#define WRAPPER_MODULE  "wrapper"

extern void modify_data_sym(char * data_sym_name, unsigned long new_value);

int call_wrapper_initcall(initcall_t fn);
void call_wrapper_exitcall(void (*fn) (void));

void setting_wrappers(void);
int get_wrapper_set(void);
