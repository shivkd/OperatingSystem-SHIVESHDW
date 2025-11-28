//paging .h
#ifndef INCLUDE_PAGING_H
#define INCLUDE_PAGING_H

#define PAGE_SIZE 4096
#define NUM_ENTRIES 1024

//flag bits
#define PAGE_PRESENT 0x001
#define PAGE_RW  0x002
#define PAGE_USER 0x004 //0 = supervieors, 1= user
//init entry points
void paging_init(void);

#endif
