#ifndef INCLUDE_PIC_H
#define INCLUDE_PIC_H

/* Remap PIC1 and PIC2 to the given vector offsets. */
void pic_remap(unsigned char offset1, unsigned char offset2);

/* Acknowledge an interrupt from PIC1 or PIC2. */
void pic_acknowledge(unsigned int interrupt);

#endif
