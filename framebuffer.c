#include "framebuffer.h"
#include "io.h"

#define FB_WIDTH   80
#define FB_HEIGHT  25

#define VGA_MEM    ((unsigned short*)0xB8000)

/* Default colors */
static unsigned char fb_fg = 0x00;
static unsigned char fb_bg = 0x0A;

/* Cursor in character coordinates */
static unsigned int cursor_x = 0;
static unsigned int cursor_y = 0;
void fb_set_color(unsigned char fg, unsigned char bg)
{
    fb_fg = bg;
    fb_bg = fg;
}



/* Write a character cell at (x,y) with given colors */
static void fb_put_cell_xy(unsigned int x, unsigned int y,
                           char c, unsigned char fg, unsigned char bg)
{
    unsigned short attr = ((fg & 0x0F) << 4) | (bg & 0x0F);
    VGA_MEM[y * FB_WIDTH + x] = ((unsigned short)attr << 8) | (unsigned char)c;
}

/* fb_write_cell, kept for compatibility: index i = y*FB_WIDTH + x */
void fb_write_cell(unsigned int i, char c,
                   unsigned char fg, unsigned char bg)
{
    unsigned int x = i % FB_WIDTH;
    unsigned int y = i / FB_WIDTH;
    if (y >= FB_HEIGHT) return;  /* ignore out-of-range */
    fb_put_cell_xy(x, y, c, fg, bg);
}

/* Move hardware cursor */
void fb_move_cursor(unsigned short pos)
{
    outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    outb(FB_DATA_PORT,    (unsigned char)((pos >> 8) & 0x00FF));
    outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    outb(FB_DATA_PORT,    (unsigned char)(pos & 0x00FF));
}

/* Scroll screen up by one line if we’ve gone past the last row */
static void fb_scroll(void)
{
    if (cursor_y < FB_HEIGHT)
        return;

    /* move rows 1..24 to rows 0..23 */
    for (unsigned int row = 1; row < FB_HEIGHT; row++) {
        unsigned int src = row * FB_WIDTH;
        unsigned int dst = (row - 1) * FB_WIDTH;
        for (unsigned int col = 0; col < FB_WIDTH; col++) {
            VGA_MEM[dst + col] = VGA_MEM[src + col];
        }
    }

    /* clear last row */
    unsigned short blank = (((fb_fg & 0x0F) << 4) | (fb_bg & 0x0F));
    blank = (blank << 8) | ' ';
    for (unsigned int col = 0; col < FB_WIDTH; col++) {
        VGA_MEM[(FB_HEIGHT - 1) * FB_WIDTH + col] = blank;
    }

    cursor_y = FB_HEIGHT - 1;
}

/* Clear screen and reset cursor */
void fb_clear(void)
{
    unsigned short blank = (((fb_fg & 0x0F) << 4) | (fb_bg & 0x0F));
    blank = (blank << 8) | ' ';

    for (unsigned int i = 0; i < FB_WIDTH * FB_HEIGHT; i++) {
        VGA_MEM[i] = blank;
    }

    cursor_x = 0;
    cursor_y = 0;
    fb_move_cursor(0);
}

/* Internal: print one character with scrolling */
static void fb_put_char(char c)
{
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        fb_scroll();
    } else {
        fb_put_cell_xy(cursor_x, cursor_y, c, fb_fg, fb_bg);
        cursor_x++;
        if (cursor_x >= FB_WIDTH) {
            cursor_x = 0;
            cursor_y++;
            fb_scroll();
        }
    }

    unsigned short pos = (unsigned short)(cursor_y * FB_WIDTH + cursor_x);
    fb_move_cursor(pos);
}

/* Public write API */
int fb_write(const char *buf, unsigned int len)
{
    for (unsigned int i = 0; i < len; i++) {
        fb_put_char(buf[i]);
    }
    return (int)len;
}
