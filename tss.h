#ifndef INCLUDE_TSS_H
#define INCLUDE_TSS_H

struct tss {
    unsigned int  prev;
    unsigned int  esp0;  unsigned int ss0;
    unsigned int  esp1;  unsigned int ss1;
    unsigned int  esp2;  unsigned int ss2;
    unsigned int  cr3;   unsigned int eip;    unsigned int eflags;
    unsigned int  eax;   unsigned int ecx;    unsigned int edx;    unsigned int ebx;
    unsigned int  esp;   unsigned int ebp;    unsigned int esi;    unsigned int edi;
    unsigned short es, _es;
    unsigned short cs, _cs;
    unsigned short ss, _ss;
    unsigned short ds, _ds;
    unsigned short fs, _fs;
    unsigned short gs, _gs;
    unsigned short ldt, _ldt;
    unsigned short trace, iopb;
} __attribute__((packed));

void tss_init(unsigned short kdata_sel, void *kernel_stack_top);
void tss_set_kernel_stack(void *kernel_stack_top);
void tss_load(unsigned short tss_selector);

#endif
