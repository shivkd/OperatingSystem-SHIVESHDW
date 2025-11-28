//define fucntions ot be used by out bitwise frame allcocater
#ifndef INCLUDE_FRAME_ALLOCH_H
#define INCLUDE_FRAME_ALLOCH_H

void frame_alloc_init(unsigned int kernel_end_phys);

//Return physical addres of kb frame or o on fail
unsigned int alloc_frame(void);

//fre teh frame at the [hys address
void free_frame(unsigned int phys_addr);
#endif