/*
 * Copyright (c) 2023 Antti Nykänen <aon@iki.fi>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

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
void serial_mirror_control(serial_port_t *, serial_port_t *);

#endif /* _TTYMIRROR_SERIAL_H */
