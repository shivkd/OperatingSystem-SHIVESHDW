#include "framebuffer.h"
#include "serial.h"
#include "gdt.h"
#include "interrupts.h"
#include "tss.h"        // <-- add
#include "usermode.h"

static unsigned char kstack[4096];  // ring0 stack used on CPL3→CPL0 switches

void kmain(void)
{
    gdt_init();                                  // GDT with user segs and TSS descriptor @ 0x28
    tss_init(KDATA_SEL, kstack + sizeof kstack); // ss0=KDATA, esp0=top of kstack
    tss_load(TSS_SEL);                           // ltr

    interrupts_init();                           // IDT + PIC
    serial_init();

    fb_clear();
    fb_write("kernel: dropping to ring3...\n", 29);

    enter_user_mode(user_entry);                 // builds ring3 frame + iret
    for(;;);
}
