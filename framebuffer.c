#include "framebuffer.h"
#include "io.h"

#define FB_WIDTH  80
#define FB_HEIGHT 25

/* Memory-mapped VGA text framebuffer base.  */
static volatile char *fb = (volatile char *)0x000B8000;

/* Current cursor position in character cells (0 .. FB_WIDTH*FB_HEIGHT-1) */
static unsigned int fb_pos = 0;

/* Default colors */
static unsigned char fb_fg = FB_LIGHT_GREY;
static unsigned char fb_bg = FB_BLACK;

/* fb_write_cell:
 * Writes character c and colors at cell index i (not byte offset).
 */
void fb_write_cell(unsigned int i, char c,
                   unsigned char fg, unsigned char bg)
{
    unsigned int offset = i * 2;        /* 2 bytes per cell: char + attr */
    fb[offset]     = c;
    fb[offset + 1] = ((fg & 0x0F) << 4) | (bg & 0x0F);
}

/* fb_move_cursor:
 * Moves hardware cursor to character position pos. 
 */
void fb_move_cursor(unsigned short pos)
{
    outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    outb(FB_DATA_PORT,    (unsigned char)((pos >> 8) & 0x00FF));
    outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    outb(FB_DATA_PORT,    (unsigned char)(pos & 0x00FF));
}

/* Clear screen to spaces with current colors and reset cursor. */
void fb_clear(void)
{
    unsigned int i;
    for (i = 0; i < FB_WIDTH * FB_HEIGHT; i++) {
        fb_write_cell(i, ' ', fb_fg, fb_bg);
    }

    fb_pos = 0;
    fb_move_cursor(fb_pos);
}

/* Internal helper: write single character, handle newline and cursor. */
static void fb_put_char(char c)
{
    if (c == '\n') {
        /* Move to start of next line */
        fb_pos += FB_WIDTH - (fb_pos % FB_WIDTH);
    } else {
        fb_write_cell(fb_pos, c, fb_fg, fb_bg);
        fb_pos++;
    }

    /* No scrolling yet: wrap to top if we overflow */
    if (fb_pos >= FB_WIDTH * FB_HEIGHT) {
        fb_pos = 0;
    }

    fb_move_cursor((unsigned short)fb_pos);
}

/* fb_write:
 * Writes len bytes from buf to the screen, updates cursor, returns len. 
 */
int fb_write(const char *buf, unsigned int len)
{
    unsigned int i;
    for (i = 0; i < len; i++) {
        fb_put_char(buf[i]);
    }
    return (int)len;
}
