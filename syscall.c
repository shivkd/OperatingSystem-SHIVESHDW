//decode syscall number, call correct kernel function
#include "syscall.h"
#include "framebuffer.h"
#include "serial.h"
#include "proc.h"
// eax - syscall number ,ebx, edx = arguments
static int sys_write_impl(struct regs *r){
    const char *buf = (const char *)r->ebx;
    unsigned int len = r->ecx;
    fb_write(buf,len);
    return (int)len;


}
static int sys_exit_impl(struct regs *r){
    (void)r; 
    if(!current_proc){
        fb_write("EXIT: WITH NO CURRENT PROC\n", 30);
        for(;;);
    }
    //set proccess death
    current_proc->state = PROC_ZOMBIE;
    //find another process to run
    struct process *next = proc_next_runnable();
    if (!next || next == current_proc){
        //nothign left to run
        fb_write("No more runnable processes\n", 28);
        for(;;);
    }
    //context switch overwirte kernel stack regs with next context
    *r = next->ctx;
    current_proc = next;
    return 0;
}
static int sys_yield_impl(struct regs *r)
{
  //utilize the shceduluere
  scheduler_tick(r);
  return 0;
}
static int sys_spawn_impl(struct regs *r){
    void (*entry)(void) = (void (*)(void))r->ebx;
    struct process *p = proc_spawn_user(entry);
    if (!p)
        return -1;

    // Process is now RUNNABLE; scheduler_tick will start picking it.
    return p->pid;


}
//r->eax is sycall number
//return value from this function goes back to suer in eax
int syscall_dispatch(struct regs *r){
    unsigned int num = r->eax;
    switch(num){
    case SYS_WRITE:
        return sys_write_impl(r);
    case SYS_EXIT:
        return sys_exit_impl(r);
    case SYS_YIELD:
        return sys_yield_impl(r);
    case SYS_SPAWN:
        return sys_spawn_impl(r);
    default:
        fb_write("UNKNOWN SYSCALL\n", 17);
        return -1;
    
    }
}