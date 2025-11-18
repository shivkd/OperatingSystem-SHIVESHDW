#include "usermode.h"

static unsigned char user_stack[4096];

void enter_user_mode(void (*entry)(void)) __attribute__((noreturn));
void enter_user_mode(void (*entry)(void))
{
    unsigned int usp = (unsigned int)(user_stack + sizeof(user_stack));
    __asm__ __volatile__ (
        "cli\n\t"
        "movw $0x23, %%ax\n\t"   /* UDATA_SEL */
        "movw %%ax, %%ds\n\t"
        "movw %%ax, %%es\n\t"
        "movw %%ax, %%fs\n\t"
        "movw %%ax, %%gs\n\t"
        "pushl $0x23\n\t"        /* SS(user) */
        "pushl %0\n\t"           /* ESP(user) */
        "pushfl\n\t"
        "orl $0x200,(%%esp)\n\t" /* set IF in saved EFLAGS */
        "pushl $0x1B\n\t"        /* CS(user) */
        "pushl %1\n\t"           /* EIP(user) */
        "iret\n\t"
        :
        : "r"(usp), "r"(entry)
        : "ax", "memory"
    );
    for(;;);
}

void user_entry(void) __attribute__((noreturn));
void user_entry(void)
{
    /* simple visible proof you’re in ring3: just halt repeatedly */
    __asm__ __volatile__("hlt");
    for(;;);
}
