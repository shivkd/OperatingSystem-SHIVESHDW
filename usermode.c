#include "usermode.h"
#include "syscall.h"

static unsigned int u_strlen(const char *s)
{
    unsigned int n = 0;
    while (s[n] != '\0') n++;
    return n;
}

//user mode syscall wrappers these run in ring3 and trigger int
static int sys_write(const char *s, unsigned int len)
{
    int ret;
    __asm__ __volatile__(
        "int $0x80"
        :   "=a"(ret)
        :   "a"(SYS_WRITE), //eax = syscall number
            "b"(s), //ebx = buf
            "c"(len) //ecx = len
        :   "memory"    

    );
    return ret;
}
static void sys_exit(int code)
{
    __asm__ __volatile__(
        "int $0x80"
        :
        : "a"(SYS_EXIT),   /* eax = syscall number */
          "b"(code)        /* ebx = exit code */
        : "memory"
    );
    for(;;);  /* should not return */
}
static void sys_yield(void)
{
    __asm__ __volatile__(
        "int $0x80"
        :
        : "a"(SYS_YIELD)
        : "memory"
    );
}


static int sys_spawn(void (*entry)(void))
{
    int ret;
    __asm__ __volatile__(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYS_SPAWN),
          "b"(entry)       // EBX = entry point
        : "memory"
    );
    return ret;
}

void user_proc1(void)
{
    /* Spawn second process exactly once */
    sys_spawn(user_proc2);

    const char banner[] =
        "\n"
        "+--------------------------------------+\n"
        "|   SHIVESH-OS USER PROCESS #1        |\n"
        "|   MULTITASKING DEMO                 |\n"
        "|   Author: SHIVESH DWIVEDI           |\n"
        "+--------------------------------------+\n\n";

    sys_write(banner, u_strlen(banner));

    const char line[] = "[P1] running in user mode...\n";

    for (;;) {
        sys_write(line, sizeof(line) - 1);

        /* crude delay: burn some cycles so output is readable */
        for (volatile unsigned int i = 0; i < 2000000U; i++)
            ;

        sys_yield();   /* let other processes run */
    }
}

/* Process 2: simple spinner line so you can see time-slicing */
void user_proc2(void)
{
    const char prefix[] = "[P2] spinner ";
    const char spins[]  = "|/-\\";
    unsigned int idx = 0;
    char buf[32];

    for (;;) {
        /* build one line: "[P2] spinner X\n" */
        unsigned int pos = 0;
        for (; pos < sizeof(prefix) - 1; pos++) {
            buf[pos] = prefix[pos];
        }
        buf[pos++] = spins[idx & 3];
        buf[pos++] = '\n';

        sys_write(buf, pos);

        idx++;

        /* burn some cycles to slow down */
        for (volatile unsigned int i = 0; i < 1000000U; i++)
            ;

        sys_yield();
    }
}


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
    const char msg[] = "This is syscall implemented by SHIVESH DWIVEDI\n";
    sys_write(msg, sizeof(msg)-1);
    sys_exit(0);
    for(;;);
}
