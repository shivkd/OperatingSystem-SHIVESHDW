#include "tss.h"
#include "gdt.h"
//#include <string.h>

static struct tss g_tss;
extern void tss_flush(unsigned short sel); /* asm ltr */

/* local memset to avoid libc */
static void *memset(void *dst, int value, unsigned int count) {
    unsigned char *d = (unsigned char *)dst;
    unsigned char v  = (unsigned char)value;
    for (unsigned int i = 0; i < count; i++) d[i] = v;
    return dst;
}


void tss_init(unsigned short kdata_sel, void *kernel_stack_top)
{
    memset(&g_tss, 0, sizeof(g_tss));
    g_tss.ss0  = kdata_sel;                          /* 0x10 */
    g_tss.esp0 = (unsigned int)kernel_stack_top;     /* top of kernel stack */
    g_tss.iopb = (unsigned short)sizeof(g_tss);      /* forbid user I/O */

    /* GDT[5] = 32-bit available TSS, access = 0x89 (P=1,DPL=0,S=0,Type=1001b) */
    gdt_set_sys_entry(5, (unsigned int)&g_tss, (unsigned int)sizeof(g_tss) - 1,
                      0x89, 0x00);
}

void tss_set_kernel_stack(void *kernel_stack_top)
{
    g_tss.esp0 = (unsigned int)kernel_stack_top;
}

void tss_load(unsigned short tss_selector)
{
    tss_flush(tss_selector); /* ltr */
}
