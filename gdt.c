#include "gdt.h"

/* 6 entries: null, kcode, kdata, ucode, udata, tss */
static struct gdt_entry gdt[6];
static struct gdt_ptr   gdtr;

extern void gdt_flush(unsigned int gdtr_addr); /* asm loader */

void gdt_set_entry(int idx, unsigned int base, unsigned int limit,
                   unsigned char access, unsigned char granularity)
{
    gdt[idx].limit_low   = (unsigned short)(limit & 0xFFFF);
    gdt[idx].base_low    = (unsigned short)(base  & 0xFFFF);
    gdt[idx].base_middle = (unsigned char)((base  >> 16) & 0xFF);
    gdt[idx].access      = access;
    gdt[idx].granularity = (unsigned char)(((limit >> 16) & 0x0F) | (granularity & 0xF0));
    gdt[idx].base_high   = (unsigned char)((base  >> 24) & 0xFF);
}

void gdt_set_sys_entry(int idx, unsigned int base, unsigned int limit,
                       unsigned char access, unsigned char granularity)
{
    /* same packing; different access/type for system segments (e.g., TSS) */
    gdt_set_entry(idx, base, limit, access, granularity);
}

void gdt_init(void)
{
    const unsigned int base  = 0;
    const unsigned int limit = 0xFFFFF;    /* with granularity=4KiB => 4GiB-1 */
    const unsigned char GRAN = 0xCF;       /* G=1,D=1,AVL=0 plus top limit nibble */

    /* 0: null */
    gdt_set_entry(0, 0, 0, 0, 0);
    /* 1: kernel code (ring0) access=0x9A */
    gdt_set_entry(1, base, limit, 0x9A, GRAN);
    /* 2: kernel data (ring0) access=0x92 */
    gdt_set_entry(2, base, limit, 0x92, GRAN);
    /* 3: user code (ring3) access=0xFA */
    gdt_set_entry(3, base, limit, 0xFA, GRAN);
    /* 4: user data (ring3) access=0xF2 */
    gdt_set_entry(4, base, limit, 0xF2, GRAN);
    /* 5: TSS placeholder; real values filled by tss_init() */
    gdt_set_entry(5, 0, 0, 0, 0);

    gdtr.size    = (unsigned short)(sizeof(gdt) - 1);
    gdtr.address = (unsigned int)&gdt[0];

    gdt_flush((unsigned int)&gdtr);
}
