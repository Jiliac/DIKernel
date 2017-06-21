#include<linux/dik/set_wrap.h>
#include <linux/dik/myprint.h>
#include "syms_modif.h"

/****************** initcall wrapper setup ************/
int (*wrapper_initcall) (initcall_t fn) = NULL;
void register_wrapper_initcall(void * ptr) {
    dbg_pr("dikernel/set_wrap.c: register_wrapper_initcall called.\n");
    wrapper_initcall = ptr;
}
EXPORT_SYMBOL(register_wrapper_initcall);

int call_wrapper_initcall(initcall_t fn) {
    int ret;
    if(!wrapper_initcall)
        return fn();
    ret = wrapper_initcall(fn);
    return ret;
}

/****************** exitcall wrapper setup ************/
void (*wrapper_exitcall) (void (*fn) (void)) = NULL;
void register_wrapper_exitcall(void * ptr) {
    dbg_pr("dikernel/set_wrap.c: register_wrapper_exitcall called.\n");
    wrapper_exitcall = ptr;
}
EXPORT_SYMBOL(register_wrapper_exitcall);

void call_wrapper_exitcall(void (*fn) (void)) {
    if(!wrapper_exitcall)
        fn();
    else
        wrapper_exitcall(fn);
}

/*************** Symbols to wrap definition ***************/

struct wrapper_info {
    char * mod_name;
    char * target_name;
    char * sym_to_change;
};

static struct wrapper_info wrapper_info[] = {
    {WRAPPER_MODULE,    "__kmalloc",    "wrapper___kmalloc" },
    {WRAPPER_MODULE,    "__aeabi_unwind_cpp_pr1",
        "wrapper___aeabi_unwind_cpp_pr1"},
};

/********** setting wrapper and utility **************/

void modify_sym_in_mod(char * mod_name, char * target_name, char * sym_to_change) {
    struct module * mod = NULL;
    const struct kernel_symbol * sym;

    mod = find_module(mod_name);

    if(mod) {
        dbg_pr("Found module %s.\n", mod_name);
        sym = find_symbol(sym_to_change, &mod, NULL, true, true);
        if(sym)
            modify_symbol(target_name, sym->value);
        else
            dbg_pr("dik/wrapper.c:setting_wrappers couldn't find %s symbol to "
                    "modify.\n", sym_to_change);
    }
    else
        dbg_pr("Didn't find '%s' among already loaded modules.\n", mod_name);
}

int wrapper_set = 0;    // boolean value
void setting_wrappers() {
    dbg_pr("!!!!!!!!!! setting_wrappers start !!!!!!!!!!\n");
    if(!request_module(SWITCHER_MODULE)) {
        wrapper_set = 1;    // WRAPPER_MODULE should be in DOMAIN_PUBLIC
        unsigned i;
        dbg_pr("setting_wrapper: %s module loaded.\n", WRAPPER_MODULE);

        /* Here we should change the domain ID of the Public Domain.
         */

        for(i = 0; i < ARRAY_SIZE(wrapper_info); ++i) {
            modify_sym_in_mod(wrapper_info[i].mod_name,
                wrapper_info[i].target_name,
                wrapper_info[i].sym_to_change);
        }
    } else
        dbg_pr("Couldn't load %s module.\n", WRAPPER_MODULE);
    dbg_pr("!!!!!!!!!! setting_wrappers end !!!!!!!!!!\n");
}

int get_wrapper_set(void) {
    return wrapper_set;
}
