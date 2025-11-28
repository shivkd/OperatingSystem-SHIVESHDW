#include "framebuffer.h"
#include "serial.h"
#include "gdt.h"
#include "interrupts.h"
#include "tss.h"
#include "usermode.h"
#include "paging.h"
#include "frame_alloc.h"
#include "io.h"
#include "fs.h"
#include "pit.h"
#include "proc.h"

extern unsigned int kernel_end;

/* Ring 0 stack used by TSS when we enter the kernel from user mode */
static unsigned char kstack[4096];

/* Simple strlen (no libc) */
static unsigned int kstrlen(const char *s)
{
    unsigned int n = 0;
    while (s[n] != '\0') n++;
    return n;
}

/* Tiny printk to framebuffer */
static void kputs(const char *s)
{
    fb_write(s, kstrlen(s));
}

/* Your ASCII banner */
static const char *shivesh_logo[] = {
"  #####  #     # ### #     # #######  #####  ",
" #     # #     #  #  #     # #     # #     # ",
" #       #     #  #  #     # #     # #       ",
"  #####  #######  #  #     # #     #  #####  ",
"       # #     #  #   #   #  #     #       # ",
" #     # #     #  #    # #   #     # #     # ",
"  #####  #     # ###    #    #######  #####  ",
"                                             ",
0
};

/* Poll the keyboard controller until Enter (scancode 0x1C) is pressed.
 * Interrupts can be enabled or disabled; this uses port I/O directly.
 */
static void wait_for_enter_poll(void)
{
    const unsigned short KBD_STATUS = 0x64;
    const unsigned short KBD_DATA   = 0x60;
    const unsigned char  KBD_OBF    = 0x01;   /* Output buffer full */

    for (;;) {
        unsigned char status = inb(KBD_STATUS);
        if (!(status & KBD_OBF))
            continue;           /* no key yet */

        unsigned char sc = inb(KBD_DATA);
        if (sc == 0x1C)        /* Enter make code */
            break;
    }
}

void kmain(void)
{
    fb_clear();

    kputs("ShiveshOS booting...\n\n");

    kputs("[BOOT] Setting up GDT...\n");
    gdt_init();
    kputs("[ OK ] GDT ready\n\n");

    kputs("[BOOT] Initializing frame allocator...\n");
    frame_alloc_init((unsigned int)&kernel_end);
    kputs("[ OK ] Frame allocator ready\n\n");

    kputs("[BOOT] Enabling paging (identity map 16MB)...\n");
    paging_init();
    kputs("[ OK ] Paging enabled\n\n");

    kputs("[BOOT] Initializing process table...\n");
    proc_init();
    kputs("[ OK ] Process table ready\n\n");

    kputs("[BOOT] Setting up TSS...\n");
    tss_init(KDATA_SEL, kstack + sizeof kstack);
    tss_load(TSS_SEL);
    kputs("[ OK ] TSS loaded\n\n");

    kputs("[BOOT] Setting up IDT & PIC...\n");
    interrupts_init();
    kputs("[ OK ] Interrupts initialized\n\n");

    serial_init();   /* debug output over COM1 if needed */

    kputs("[BOOT] Initializing initfs...\n");
    fs_init();
    kputs("[ OK ] initfs loaded\n\n");

    kputs("[BOOT] Programming PIT @ 50 Hz...\n");
    pit_init(50);
    kputs("[ OK ] PIT ready (preemptive ticks)\n\n");

    /* Show that /bin/init exists and is readable */
    struct fs_obj *init_obj = fs_lookup("/bin/init");
    if (!init_obj) {
        kputs("[ERR ] no /bin/init in initfs\n");
        for(;;);
    }

    char buf[64];
    int n = fs_read(init_obj, buf, 0, sizeof(buf) - 1);
    if (n < 0) {
        kputs("[ERR ] read error from /bin/init\n");
        for(;;);
    }
    buf[n] = '\0';

    kputs("[INFO] /bin/init first bytes:\n");
    fb_write(buf, n);
    kputs("\n\n");

    /* Show your logo */
    const char **p = shivesh_logo;
    while (*p) {
        fb_write(*p, kstrlen(*p));
        fb_write("\n", 1);
        p++;
    }

    kputs("\n[BOOT COMPLETE] Features:\n");
    kputs("  - GDT with user/kernel segments + TSS\n");
    kputs("  - Identity-mapped paging 16MB\n");
    kputs("  - Frame allocator (bitmap-based)\n");
    kputs("  - initfs (simple read-only FS)\n");
    kputs("  - Syscalls via int 0x80\n");
    kputs("  - Preemptive multitasking via PIT + scheduler\n\n");

    kputs("Press ENTER to start 2-process demo...\n");

    interrupts_enable();     /* allow keyboard IRQs if you use them elsewhere */
    wait_for_enter_poll();   /* but this works even just by polling ports */
    interrupts_disable();    /* optional; we'll re-enable in user mode anyway */

    fb_clear();

    kputs("Starting 2-process preemptive demo (P1/P2)...\n\n");

    /* Drop into user mode; user_proc1 will sys_spawn(user_proc2) */
    enter_user_mode(user_proc1);

    for (;;);
}
