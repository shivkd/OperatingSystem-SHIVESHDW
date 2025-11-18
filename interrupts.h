//public interface for interrupt setup and c level central handler
#ifndef INCLUDE_INTERRUPTS_H
#define INCLUDE_INTERRUPTS_H
// intiliaze idt and pic but d ont enable them yet
void interrupts_init(void);
//enable/disable cpu interrupts
void interrupts_enable(void);
void interrupts_disable(void);
// called from assembly for every interrupt
void interrupt_handler(unsigned int interrupt);
#endif