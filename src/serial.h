#ifndef _TTYMIRROR_SERIAL_H
#define _TTYMIRROR_SERIAL_H

#include <termios.h>

typedef enum {
    NONE,
    EVEN,
    ODD
} serial_parity_t;

typedef struct {
	char *port_name;
	int fd;
    struct termios cur_termios;
    struct termios old_termios;
    int status;
	speed_t baudrate;
	int databits;
	int stopbits;
	serial_parity_t parity;
} serial_port_t;

int serial_open(char *);
int serial_set_options(serial_port_t *);
speed_t serial_validate_baudrate(int);
void serial_mirror_data(serial_port_t *, serial_port_t *);
//void serial_mirror_status(serial_port_t *, serial_port_t *);

#endif /* _TTYMIRROR_SERIAL_H */
