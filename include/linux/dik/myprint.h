#ifndef __MYPRINT__
#define __MYPRINT__

#ifdef DEBUG
#define dbg_pr(fmt, ...)    \
    printk(fmt, ##__VA_ARGS__)
#else
#define dbg_pr(fmt, ...)    \
    no_printk(fmt, ##__VA_ARGS__)
#endif

#endif
