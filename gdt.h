#ifndef INCLUDE_GDT_H
#define INCLUDE_GDT_H
#define KCODE_SEL 0x08 // gtd[1]
#define KDATA_SEL 0x10 // gdt2
#define UCODE_SEL 0x1B
#define UDATA_SEL 0x23 //4
#define TSS_SEL 0x28 //5

struct gdt_entry {
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char  base_middle;
    unsigned char  access;
    unsigned char  granularity;
    unsigned char  base_high;
} __attribute__((packed));

struct gdt_ptr {
    unsigned short size;
    unsigned int   address;
} __attribute__((packed));

/* publish helpers so TSS code can install its descriptor */
void gdt_set_entry(int idx, unsigned int base, unsigned int limit,
                   unsigned char access, unsigned char granularity);
void gdt_set_sys_entry(int idx, unsigned int base, unsigned int limit,
                       unsigned char access, unsigned char granularity);

void gdt_init(void);

#endif
