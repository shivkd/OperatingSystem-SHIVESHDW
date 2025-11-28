// share the register layout and syscall dispathcer prototype
#ifndef INCLUDE_SYSCALL_H
#define INCLUDE_SYSCALL_H
//pusad psuhes, eax, ecc, edx, ebx, esp, ebp, edi, edi
struct regs{
    unsigned int edi;
    unsigned int esi;
    unsigned int ebp;
    unsigned int esp_dummy;
    unsigned int ebx;
    unsigned int edx;
    unsigned int ecx;
    unsigned int eax;

    //then comes the hardware psuhed interrupt frame
    unsigned int eip; //reutrn user
    unsigned int cs; //(user, 1b)
    unsigned int eflags;
    unsigned int useresp; //user mode esp at that of interrupt
    unsigned int ss; //user mode ss 0x23
    
};
//kerneel side dispatcher from syscall_entry
int syscall_dispatch(struct regs *r);
//syscall numbers 
#define SYS_WRITE 1
#define SYS_EXIT 2
#define SYS_YIELD 3
#define SYS_SPAWN 4

#endif