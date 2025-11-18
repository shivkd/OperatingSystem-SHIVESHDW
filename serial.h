#ifndef INCLUDE_SERIAL_H
#define INCLUDE_SERIAL_H

/* Base COM1 port */
#define SERIAL_COM1_BASE                0x3F8  /* COM1 base port */

/* All ports are relative to the data port */
#define SERIAL_DATA_PORT(base)          (base)
#define SERIAL_FIFO_COMMAND_PORT(base)  (base + 2)
#define SERIAL_LINE_COMMAND_PORT(base)  (base + 3)
#define SERIAL_MODEM_COMMAND_PORT(base) (base + 4)
#define SERIAL_LINE_STATUS_PORT(base)   (base + 5)

/* Line command values */
#define SERIAL_LINE_ENABLE_DLAB         0x80   /* enable divisor access */

/* Public API */
void serial_init(void);

void serial_write_char(unsigned short com, unsigned char c);

int  serial_write(unsigned short com, const char *buf, unsigned int len);

#endif
