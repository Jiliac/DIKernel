#include<linux/init.h>

#define WRAPPER_MODULE  "wrapper"

void call_switcher_to_mod(void);

int call_wrapper_initcall(initcall_t fn, size_t domain);
void call_wrapper_exitcall(void (*fn) (void));

int is_switcher_to_mod_registered(void);
void setting_wrappers(void);
int get_wrapper_set(void);
