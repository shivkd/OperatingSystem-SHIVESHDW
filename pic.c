#include "pic.h"
#include "io.h"
/* Ports */
#define PIC1_CMD   0x20
#define PIC1_DATA  0x21
#define PIC2_CMD   0xA0
#define PIC2_DATA  0xA1

/* ICW control bits */
#define ICW1_INIT  0x10
#define ICW1_ICW4  0x01
#define ICW4_8086  0x01

/* Interrupt ranges after remap: */
#define PIC1_START_INTERRUPT 0x20
#define PIC2_START_INTERRUPT 0x28
#define PIC2_END_INTERRUPT   (PIC2_START_INTERRUPT + 7)

#define PIC_ACK 0x20

void pic_remap(unsigned char offset1, unsigned char offset2){
    unsigned char a1, a2;

    //save masks
    a1 = inb(PIC1_DATA);
    a2 = inb(PIC2_DATA);
    // start init

    outb(PIC1_CMD, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_CMD, ICW1_INIT | ICW1_ICW4);

     /* Set vector offset */
    outb(PIC1_DATA, offset1);
    outb(PIC2_DATA, offset2);


    //tell pics about their witing  
    outb(PIC1_DATA, 4); //pic2 at irq2
    outb(PIC2_DATA, 2); //cascade indietty
    //set 8066 mode
    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);
    //resotre the masks
    /* Restore masks */
    outb(PIC1_DATA, a1);
    outb(PIC2_DATA, a2);

}

//called after hanlding the harward irq
void pic_acknowledge(unsigned int interrupt){
    if (interrupt < PIC1_START_INTERRUPT || interrupt > PIC2_END_INTERRUPT){
        return;
    }
    if (interrupt < PIC2_START_INTERRUPT){
        // from pic1
        outb(PIC1_CMD, PIC_ACK);
    }
    else{
        outb(PIC2_CMD, PIC_ACK);
    }

}