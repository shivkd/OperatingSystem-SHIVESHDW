#include "serial.h"
#include "io.h"

/* 4.3.2: configure baud rate (115200 / divisor) */
static void serial_configure_baud_rate(unsigned short com,
                                       unsigned short divisor)
{
    /* Enable DLAB so we can set divisor.  */
    outb(SERIAL_LINE_COMMAND_PORT(com), SERIAL_LINE_ENABLE_DLAB);

    /* Book version writes both bytes to SERIAL_DATA_PORT(com).
     * Correct 16550 behavior expects low at base, high at base+1.
     * This version uses the proper mapping.
     */
    outb(SERIAL_DATA_PORT(com),      (unsigned char)(divisor & 0x00FF));       /* low */
    outb(SERIAL_DATA_PORT(com) + 1,  (unsigned char)((divisor >> 8) & 0x00FF));/* high */

    /* Clear DLAB again by configuring line later (serial_configure_line). */
}

/* 4.3.2: configure line (8N1) */
static void serial_configure_line(unsigned short com)
{
    /* Bit:     | 7 | 6 | 5 4 3 | 2 | 1 0 |
     * Content: | d | b | prty  | s | dl  |
     * Value:   | 0 | 0 | 0 0 0 | 0 | 1 1 | = 0x03
     * -> 8 data bits, no parity, one stop bit. 
     */
    outb(SERIAL_LINE_COMMAND_PORT(com), 0x03);
}

/* 4.3.3: configure FIFO/buffers */
static void serial_configure_buffers(unsigned short com)
{
    /* 0xC7:
     *  - enable FIFO
     *  - clear RX/TX FIFOs
     *  - use 14-byte threshold. 
     */
    outb(SERIAL_FIFO_COMMAND_PORT(com), 0xC7);
}

/* 4.3.4: configure modem control */
static void serial_configure_modem(unsigned short com)
{
    /* 0x03: RTS = 1, DTR = 1, others 0 (no interrupts etc.)  */
    outb(SERIAL_MODEM_COMMAND_PORT(com), 0x03);
}

/* 4.3.5: check if transmit FIFO is empty */
static int serial_is_transmit_fifo_empty(unsigned short com)
{
    /* Bit 5 of line status: 1 -> transmit FIFO empty.  */
    return inb(SERIAL_LINE_STATUS_PORT(com)) & 0x20;
}

/* 4.3.5: write a single character (busy-wait) */
void serial_write_char(unsigned short com, unsigned char c)
{
    /* Spin until previous transmission finished */
    while (!serial_is_transmit_fifo_empty(com))
        ;

    outb(SERIAL_DATA_PORT(com), c);
}

/* Driver init: configure everything in one call */
void serial_init(void)
{
    /* Use divisor 3 -> 115200 / 3 = 38400 baud (example from book) */
    serial_configure_baud_rate(SERIAL_COM1_BASE, 3);
    serial_configure_line(SERIAL_COM1_BASE);
    serial_configure_buffers(SERIAL_COM1_BASE);
    serial_configure_modem(SERIAL_COM1_BASE);
}

/* 4.3.7: write() - like fb_write but for serial port */
int serial_write(unsigned short com, const char *buf, unsigned int len)
{
    unsigned int i;
    for (i = 0; i < len; i++) {
        serial_write_char(com, (unsigned char)buf[i]);
    }
    return (int)len;
}
