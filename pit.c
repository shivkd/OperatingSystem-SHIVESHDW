//pit,c channel 0 setup for periodc interupt time
#include "pit.h"
#include "io.h"

#define PIT_CH0_PORT 0x40
#define PIT_CMD_PORT 0x43
#define PIT_INPUT_HZ 1193182u //PIT base freq
void pit_init(unsigned int hz)
{
    if(hz == 0){
        return;
    }
    unsigned int divisor = PIT_INPUT_HZ/ hz;
    //chan 0 access mode 11
    //mode 3 square wave
    outb(PIT_CMD_PORT, 0x36);
    //send low byte then high byte of a divsor to chanel 0
    outb(PIT_CH0_PORT, (unsigned char)(divisor & 0xFF));
    outb(PIT_CH0_PORT, (unsigned)((divisor >> 8) & 0xFF));
    
}