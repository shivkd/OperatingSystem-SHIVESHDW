//  minimal paging implementation
// one page directry aligned to 4kb
//four page tables 4kb to 0-16kb
#include "paging.h"
//4kb allgiment for cr3 and page tables
static unsigned int page_directory[NUM_ENTRIES]
        __attribute__((aligned(PAGE_SIZE)));

static unsigned int page_tables[4][NUM_ENTRIES]
        __attribute__((aligned(PAGE_SIZE)));

static void enable_paging(unsigned int pd_phys)
{
    /* pd_phys is physical because paging is off when this runs,
       and we identity-map the area where these arrays live. */
    __asm__ __volatile__ (
        "mov %0, %%cr3\n\t"        /* load page directory base */
        "mov %%cr0, %%eax\n\t"
        "or  $0x80000000, %%eax\n\t" /* set PG bit */
        "mov %%eax, %%cr0\n\t"
        :
        : "r"(pd_phys)
        : "eax", "memory"
    );
}

void paging_init(void){
    int i, t;
    //clear page directory
    for (i = 0; i < NUM_ENTRIES; i++){
        page_directory[i] = 0;

    }
    // identity map -16 via 4 page tables
    unsigned int phys = 0;
    for(t = 0; t < 4; t++ ){
        // fill page table t map 4 mb chucnks
        for (i = 0; i < NUM_ENTRIES; i++){
            // frame base| present | rw, supervisor
            page_tables[t][i] = (phys & 0xFFFFF000)
                                | PAGE_PRESENT
                                | PAGE_RW
                                | PAGE_USER;
            phys += PAGE_SIZE;
        }
        /* PDE entry: points to page_tables[t] */
        page_directory[t] =
            ((unsigned int)page_tables[t] & 0xFFFFF000)
            | PAGE_PRESENT
            | PAGE_RW  /* supervisor-only */
            | PAGE_USER; //add user mode
    }
    /* Enable paging using the physical address of page_directory */
    enable_paging((unsigned int)page_directory);


}



