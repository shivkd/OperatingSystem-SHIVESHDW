#include "proc.h"
#include "framebuffer.h"
#include "tss.h"
#include "framebuffer.h"
#include "gdt.h"
#include "syscall.h"

static struct process proc_table[MAX_PROCS];

struct process *current_proc = 0;

void scheduler_tick(struct regs *r)
{
    // First time we ever schedule: turn current regs into proc0
    if (!current_proc) {
        struct process *p = proc_create_from_regs(r);
        if (!p) {
            fb_write("NO PROC SLOT\n", sizeof("NO PROC SLOT\n") - 1);
            for (;;);
        }
        current_proc = p;
        return;
    }

    // Save current process context
    current_proc->ctx = *r;

    // Pick next runnable process
    struct process *next = proc_next_runnable();

    if (next && next != current_proc) {
        // Switch: load next context into this regs frame
        *r = next->ctx;
        current_proc = next;
        //update tss kernel stack to this process lernel stack
        tss_set_kernel_stack(current_proc->kstack + KSTACK_SIZE);
    }
    // If no other runnable, we just resume same process
}

//init all process lsots as unssed and clear
void proc_init(void){
    int i;
    for (i = 0; i < MAX_PROCS; i++){
        proc_table[i].pid = i;
        proc_table[i].state = PROC_UNUSED;

    }
    current_proc = 0; 

}

//find a free slot in proc_table and mark it runable
static struct process *proc_slot_alloc(void){
    int i;
    for(i = 0; i <MAX_PROCS; i++){
        if (proc_table[i].state == PROC_UNUSED){
            proc_table[i].state = PROC_RUNNABLE;
            return &proc_table[i];
        }
    }
    return 0;
}
//create a process from current register fram
//r is the reg sitting kernerl stack for sycall 
struct process *proc_create_from_regs(struct regs *r)
{
    struct process *p = proc_slot_alloc();
    if (!p) return 0;
     p->ctx = *r; //full copy
     return p;

}
//very simple round round, start from currentproc->pid, scan forward
// through the table wrapping around
//return the first runnable process
//if none found, return currentproc
struct process *proc_next_runnable(void)
{
    if(!current_proc){
        return 0; 
    }
    int start = current_proc->pid;
    int i;
    for(i =1; i <= MAX_PROCS; i++){
        int idx = (start + i) % MAX_PROCS;
        if(proc_table[idx].state == PROC_RUNNABLE){
            return &proc_table[idx];
        }
    }
    return current_proc; 
}
//create a brand new user process with its own user +kernel stack
struct process *proc_spawn_user(void (*entry)(void))
{
    struct process *p = proc_slot_alloc();
    if (!p) return 0;
    //zero the context
    int i; 
    unsigned int *ctx = (unsigned int *)&p->ctx;
    for (i = 0; i < (int)(sizeof(p->ctx) / sizeof(unsigned int)); i++){
        ctx[i] = 0;
    }
    unsigned int usp = (unsigned int)(p->ustack + USTACK_SIZE);

    /// General registers (8 words) – all zero is fine
    p->ctx.edi       = 0;
    p->ctx.esi       = 0;
    p->ctx.ebp       = 0;
    p->ctx.esp_dummy = 0;     // placeholder for old ESP from pushad
    p->ctx.ebx       = 0;
    p->ctx.edx       = 0;
    p->ctx.ecx       = 0;
    p->ctx.eax       = 0;

    // Hardware interrupt frame fields to be consumed by IRET
    p->ctx.eip     = (unsigned int)entry;  // where to start in user mode
    p->ctx.cs      = UCODE_SEL;           // 0x1B
    p->ctx.eflags  = 0x202;               // IF=1
    p->ctx.useresp = usp;                 // user-mode ESP
    p->ctx.ss      = UDATA_SEL;           // 0x23

    return p;
}


