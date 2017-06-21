#include<linux/init.h>

#define SWITCHER_MODULE "domain_switcher"
#define WRAPPER_MODULE  "wrapper"

int call_wrapper_initcall(initcall_t fn);
void call_wrapper_exitcall(void (*fn) (void));

void setting_wrappers(void);
int get_wrapper_set(void);
