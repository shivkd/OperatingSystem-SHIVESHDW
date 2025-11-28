#include "interrupts.h"
#include "io.h"
#include "pic.h"
#include "keyboard.h"
#include "framebuffer.h"
#include "serial.h"
#include "proc.h"
#include "syscall.h"




/*IDt entry*/
struct idt_entry{
    unsigned short offset_low; //handler bits 0-15r
    unsigned short selector; //cpde segment selector
    unsigned char zero;
    unsigned char flags; // type and attriubtyte
    unsigned short offset_high; // handelr bits 16-31
}__attribute__((packed));

// idt pointer for lidt
struct idt_ptr{
    unsigned short limit;
    unsigned int base;

}__attribute__((packed));

static struct idt_entry idt[256];
static struct idt_ptr    idt_desc;

/* Assembly function in interrupts_asm.s */
extern void load_idt(unsigned int idt_ptr_address);

/* Assembly stubs for each interrupt we care about */
extern void interrupt_handler_0(void);
extern void interrupt_handler_1(void);
extern void interrupt_handler_2(void);
extern void interrupt_handler_3(void);
extern void interrupt_handler_4(void);
extern void interrupt_handler_5(void);
extern void interrupt_handler_6(void);
extern void interrupt_handler_7(void);
extern void interrupt_handler_8(void);
extern void interrupt_handler_9(void);
extern void interrupt_handler_10(void);
extern void interrupt_handler_11(void);
extern void interrupt_handler_12(void);
extern void interrupt_handler_13(void);
extern void interrupt_handler_14(void);
extern void interrupt_handler_15(void);
extern void interrupt_handler_16(void);
extern void interrupt_handler_17(void);
extern void interrupt_handler_18(void);
extern void interrupt_handler_19(void);
extern void interrupt_handler_20(void);
extern void interrupt_handler_21(void);
extern void interrupt_handler_22(void);
extern void interrupt_handler_23(void);
extern void interrupt_handler_24(void);
extern void interrupt_handler_25(void);
extern void interrupt_handler_26(void);
extern void interrupt_handler_27(void);
extern void interrupt_handler_28(void);
extern void interrupt_handler_29(void);
extern void interrupt_handler_30(void);
extern void interrupt_handler_31(void);

/* IRQs after PIC remap: 0x20–0x2F */
extern void interrupt_handler_32(void);  /* timer */
extern void interrupt_handler_33(void);  /* keyboard */
extern void interrupt_handler_34(void);
extern void interrupt_handler_35(void);
extern void interrupt_handler_36(void);
extern void interrupt_handler_37(void);
extern void interrupt_handler_38(void);
extern void interrupt_handler_39(void);
extern void interrupt_handler_40(void);
extern void interrupt_handler_41(void);
extern void interrupt_handler_42(void);
extern void interrupt_handler_43(void);
extern void interrupt_handler_44(void);
extern void interrupt_handler_45(void);
extern void interrupt_handler_46(void);
extern void interrupt_handler_47(void);

extern void syscall_entry(void);

// fill on eidt entry for a 32 bit interrupt/trap gate in ring 0
static void idt_set_entry(int num, unsigned int base, unsigned char flags){
    idt[num].offset_low = (unsigned short)(base & 0xFFFF);
    idt[num].selector = 0x08; //kernel code segment
    idt[num].zero = 0; 
    idt[num].flags = flags; //ring 0 32 bit intterupt
    idt[num].offset_high = (unsigned short)((base >> 16) & 0xFFFF);


}
static void idt_clear(void) {
    int i;
    for (i = 0; i < 256; i++) {
        idt[i].offset_low  = 0;
        idt[i].selector    = 0;
        idt[i].zero        = 0;
        idt[i].flags       = 0;   // NOT present
        idt[i].offset_high = 0;
    }
}

void interrupts_init(void)
{
     idt_clear();
    /* 2) Set handlers for CPU exceptions (0–31) */
    idt_set_entry(0,  (unsigned int)interrupt_handler_0, 0x8E);
    idt_set_entry(1,  (unsigned int)interrupt_handler_1, 0x8E);
    idt_set_entry(2,  (unsigned int)interrupt_handler_2, 0x8E);
    idt_set_entry(3,  (unsigned int)interrupt_handler_3, 0x8E);
    idt_set_entry(4,  (unsigned int)interrupt_handler_4, 0x8E);
    idt_set_entry(5,  (unsigned int)interrupt_handler_5, 0x8E);
    idt_set_entry(6,  (unsigned int)interrupt_handler_6, 0x8E);
    idt_set_entry(7,  (unsigned int)interrupt_handler_7, 0x8E);
    idt_set_entry(8,  (unsigned int)interrupt_handler_8, 0x8E);
    idt_set_entry(9,  (unsigned int)interrupt_handler_9, 0x8E);
    idt_set_entry(10, (unsigned int)interrupt_handler_10, 0x8E);
    idt_set_entry(11, (unsigned int)interrupt_handler_11, 0x8E);
    idt_set_entry(12, (unsigned int)interrupt_handler_12, 0x8E);
    idt_set_entry(13, (unsigned int)interrupt_handler_13, 0x8E);
    idt_set_entry(14, (unsigned int)interrupt_handler_14, 0x8E);
    idt_set_entry(15, (unsigned int)interrupt_handler_15, 0x8E);
    idt_set_entry(16, (unsigned int)interrupt_handler_16, 0x8E);
    idt_set_entry(17, (unsigned int)interrupt_handler_17, 0x8E);
    idt_set_entry(18, (unsigned int)interrupt_handler_18, 0x8E);
    idt_set_entry(19, (unsigned int)interrupt_handler_19, 0x8E);
    idt_set_entry(20, (unsigned int)interrupt_handler_20, 0x8E);
    idt_set_entry(21, (unsigned int)interrupt_handler_21, 0x8E);
    idt_set_entry(22, (unsigned int)interrupt_handler_22, 0x8E);
    idt_set_entry(23, (unsigned int)interrupt_handler_23, 0x8E);
    idt_set_entry(24, (unsigned int)interrupt_handler_24, 0x8E);
    idt_set_entry(25, (unsigned int)interrupt_handler_25, 0x8E);
    idt_set_entry(26, (unsigned int)interrupt_handler_26, 0x8E);
    idt_set_entry(27, (unsigned int)interrupt_handler_27, 0x8E);
    idt_set_entry(28, (unsigned int)interrupt_handler_28, 0x8E);
    idt_set_entry(29, (unsigned int)interrupt_handler_29, 0x8E);
    idt_set_entry(30, (unsigned int)interrupt_handler_30, 0x8E);
    idt_set_entry(31, (unsigned int)interrupt_handler_31, 0x8E);

    /* 3) IRQs 0–15 (remapped to 0x20–0x2F) */
    idt_set_entry(32, (unsigned int)interrupt_handler_32, 0x8E);
    idt_set_entry(33, (unsigned int)interrupt_handler_33, 0x8E);
    idt_set_entry(34, (unsigned int)interrupt_handler_34, 0x8E);
    idt_set_entry(35, (unsigned int)interrupt_handler_35, 0x8E);
    idt_set_entry(36, (unsigned int)interrupt_handler_36, 0x8E);
    idt_set_entry(37, (unsigned int)interrupt_handler_37, 0x8E);
    idt_set_entry(38, (unsigned int)interrupt_handler_38, 0x8E);
    idt_set_entry(39, (unsigned int)interrupt_handler_39, 0x8E);
    idt_set_entry(40, (unsigned int)interrupt_handler_40, 0x8E);
    idt_set_entry(41, (unsigned int)interrupt_handler_41, 0x8E);
    idt_set_entry(42, (unsigned int)interrupt_handler_42, 0x8E);
    idt_set_entry(43, (unsigned int)interrupt_handler_43, 0x8E);
    idt_set_entry(44, (unsigned int)interrupt_handler_44, 0x8E);
    idt_set_entry(45, (unsigned int)interrupt_handler_45, 0x8E);
    idt_set_entry(46, (unsigned int)interrupt_handler_46, 0x8E);
    idt_set_entry(47, (unsigned int)interrupt_handler_47, 0x8E);
    //syscall entry into 0x80, dpl = 3, so user mode can oinvoke
    idt_set_entry(0x80, (unsigned int)syscall_entry, 0xEE);

    /* 4) Build IDT descriptor */
    idt_desc.limit = sizeof(idt) - 1;
    idt_desc.base  = (unsigned int)&idt[0];

    /* 5) Remap PIC so IRQs start at 0x20 */
    pic_remap(0x20, 0x28);

    /* 6) Load IDT into CPU */
    load_idt((unsigned int)&idt_desc);
}
/* Simple wrapper around cli/sti */
void interrupts_enable(void)
{
    __asm__ __volatile__("sti");
}

void interrupts_disable(void)
{
    __asm__ __volatile__("cli");
}
void interrupt_handler(struct regs *r, unsigned int interrupt)
{
    /* CPU exceptions 0–31 */
    if (interrupt < 32) {
        fb_write("CPU exception\n", 14);
        for (;;)
            ;
    }

    /* Timer IRQ (IRQ0 => interrupt 0x20) */
    if (interrupt == 32) {
        scheduler_tick(r);
        pic_acknowledge(interrupt);
        return;
    }

    /* Keyboard IRQ (IRQ1 => interrupt 0x21) */
    if (interrupt == 33) {
        unsigned char sc = read_scan_code();

        /* Dump scancode to serial in hex, just to see something. */
        static const char hex[] = "0123456789ABCDEF";
        char buf[3];
        buf[0] = hex[(sc >> 4) & 0x0F];
        buf[1] = hex[sc & 0x0F];
        buf[2] = '\n';
        serial_write(SERIAL_COM1_BASE, buf, 3);

        pic_acknowledge(interrupt);
        return;
    }

    /* Other hardware IRQs */
    if (interrupt >= 32 && interrupt <= 47) {
        pic_acknowledge(interrupt);
        return;
    }
}
