#include "frame_alloc.h"

// frame size 4096 bytes (4 KiB)
#define FRAME_SIZE      4096
#define MAX_PHYS_MEM    (64 * 1024 * 1024)   // 64 MB
#define NUM_FRAMES      (MAX_PHYS_MEM / FRAME_SIZE)

// bitmap: 1 bit per frame
static unsigned int frames_bitmap[NUM_FRAMES / 32];

// local memset (no libc)
static void *memset_local(void *dst, int value, unsigned int count){
    unsigned char *d = (unsigned char *)dst;
    unsigned char v = (unsigned char)value;
    unsigned int i; 
    for (i = 0; i < count; i++) d[i] = v;
    return dst;
}

// bit helpers
static void set_frame(unsigned int frame){
    frames_bitmap[frame / 32] |= (1U << (frame % 32));
}

static void clear_frame(unsigned int frame){
    frames_bitmap[frame / 32] &= ~(1U << (frame % 32));
}

static int test_frame(unsigned int frame){
    return (frames_bitmap[frame / 32] & (1U << (frame % 32))) != 0;
}

void frame_alloc_init(unsigned int kernel_end_phys){
    // mark all frames as free initially
    memset_local(frames_bitmap, 0, sizeof(frames_bitmap));

    // reserve frame 0
    set_frame(0);

    // reserve all frames up to kernel_end_phys
    unsigned int end = kernel_end_phys;
    if (end & (FRAME_SIZE - 1)) {
        end = (end & ~(FRAME_SIZE - 1)) + FRAME_SIZE;
    }

    unsigned int frame = 0;
    while (frame * FRAME_SIZE < end && frame < NUM_FRAMES) {
        set_frame(frame);
        frame++;
    }
}

unsigned int alloc_frame(void)
{
    unsigned int frame;

    for (frame = 0; frame < NUM_FRAMES; frame++) {
        if (!test_frame(frame)) {
            set_frame(frame);
            return frame * FRAME_SIZE;
        }
    }

    return 0;
}

void free_frame(unsigned int phys_addr){
    if (phys_addr == 0) return;

    unsigned int frame = phys_addr / FRAME_SIZE;
    if (frame >= NUM_FRAMES) return;

    clear_frame(frame);
}
