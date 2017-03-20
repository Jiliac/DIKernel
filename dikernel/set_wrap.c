#include<linux/dik/set_wrap.h>
#include "syms_modif.h"

/****************** initcall wrapper setup ************/

int (*wrapper_initcall) (initcall_t fn) = 0;
void register_wrapper_initcall(void * ptr) {
    printk("dikernel/set_wrap.c: register_wrapper_initcall called.\n");
    wrapper_initcall = ptr;
}
EXPORT_SYMBOL(register_wrapper_initcall);

int call_wrapper_initcall(initcall_t fn) {
    if(!wrapper_initcall) {
        return fn();
    }
    else {
        return wrapper_initcall(fn);
    }
}

/****************** exitcall wrapper setup ************/
void (*wrapper_exitcall) (void (*fn) (void)) = 0;
void register_wrapper_exitcall(void * ptr) {
    printk("dikernel/set_wrap.c: register_wrapper_exitcall called.\n");
    wrapper_exitcall = ptr;
}
EXPORT_SYMBOL(register_wrapper_exitcall);

void call_wrapper_exitcall(void (*fn) (void)) {
    if(!wrapper_exitcall) {
        fn();
    }
    else
        wrapper_exitcall(fn);
}


/****************** switch to mod setup ***************/

void (*switcher_to_mod) (void) = 0;
void register_switcher_to_mod(void * ptr) {
    printk("dikernel/set_wrap.c: register_switcher_to_mod called.\n");
    switcher_to_mod = ptr;
}
EXPORT_SYMBOL(register_switcher_to_mod);

void call_switcher_to_mod(void) {
    switcher_to_mod();
}

int is_switcher_to_mod_registered(void) {
    return (int) switcher_to_mod;
}

/********** setting wrapper and utility **************/

void modify_sym_in_mod(char * mod_name, char * target_name, char * sym_to_change) {
    struct module * mod = NULL;
    const struct kernel_symbol * sym;

    mod = find_module(mod_name);

    if(mod) {
        printk("Found module %s.\n", mod_name);
        sym = find_symbol(sym_to_change, &mod, NULL, true, true);
        if(sym)
            modify_symbol(target_name, sym->value);
        else
            printk("dik/wrapper.c:setting_wrappers couldn't find %s symbol to "
                    "modify.\n", sym_to_change);
    }
    else
        printk("Didn't find '%s' among already loaded modules.\n", mod_name);
}

#define WRAPPER_MODULE  "wrapper"
void setting_wrappers() {
    printk("!!!!!!!!!! setting_wrappers start !!!!!!!!!!\n");
    if(!request_module(WRAPPER_MODULE)) {
        printk("setting_wrapper: %s module loaded.\n", WRAPPER_MODULE);
        modify_sym_in_mod(WRAPPER_MODULE, "__kmalloc", "wrapper__kmalloc");
    } else
        printk("Couldn't load %s module.\n", WRAPPER_MODULE);
    printk("!!!!!!!!!! setting_wrappers end !!!!!!!!!!\n");
}
