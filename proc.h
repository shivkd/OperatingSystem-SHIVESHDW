//describes the processes and the scheduler api
#ifndef INCLUDE_PROC_H
#define INCLUDE_PROC_H

#include "syscall.h"

#define KSTACK_SIZE 4096
#define USTACK_SIZE 4096

#define MAX_PROCS 8 // max processes
#define PROC_UNUSED 0 //slot not in use
#define PROC_RUNNABLE 1
#define PROC_ZOMBIE 2 // logically dead processes to not schedule

struct process{
    int pid; // simpled inedex 0-7
    int state; // on of the three macors
    struct regs ctx; //context switch registers
    unsigned char kstack[KSTACK_SIZE];
    unsigned char ustack[USTACK_SIZE];

};

extern struct process *current_proc;

void proc_init(void);
//turn the current regs frame into a new process slot
struct process *proc_create_from_regs(struct regs *r);
//find the next runnable process usign round roubin
struct process *proc_next_runnable(void);
//spawn a fresh user porcess with given entry point
struct process *proc_spawn_user(void (*entry)(void));
void scheduler_tick(struct regs *r);

#endif
