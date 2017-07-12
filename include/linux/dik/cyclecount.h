#ifndef _CYCLECOUNT_H
#define _CYCLECOUNT_H

#define get_cyclecount(t)   \
        asm volatile ("MRC p15, 0, %0, c9, c13, 0\t\n": "=r"(t))
        
void init_perfcounters(void);

extern unsigned int *cc_init_f_pt;
#define get_cc_init()   \
    get_cyclecount(*cc_init_f_pt)

#endif
