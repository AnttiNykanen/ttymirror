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

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "serial.h"

#define BUFFER_SIZE 4096
static char buffer[BUFFER_SIZE];

int serial_open(char *path)
{
	int fd = -1;

	fd = open(path, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
	if (fd == -1) {
		fprintf(stderr, "Error opening %s: %s\n", path,
			strerror(errno));
	}

	return fd;
}

int serial_set_options(serial_port_t *port)
{
	if (port == NULL) {
		return -1;
	}

	(void)tcgetattr(port->fd, &port->cur_termios);
	port->old_termios = port->cur_termios;

	/* Set baudrate */
	(void)cfsetispeed(&port->cur_termios, port->baudrate);
	(void)cfsetospeed(&port->cur_termios, port->baudrate);

	/* Set data bits */
	switch (port->databits) {
		case 5:
			port->cur_termios.c_cflag |= CS5;
			break;
		case 6:
			port->cur_termios.c_cflag |= CS6;
			break;
		case 7:
			port->cur_termios.c_cflag |= CS7;
			break;
		case 8:
			port->cur_termios.c_cflag |= CS8;
			break;
		default:
			return -1;
	}

	/* Set stop bits */
	if (port->stopbits == 2) {
		port->cur_termios.c_cflag |= CSTOPB;
	} else if (port->stopbits != 1) {
		return -1;
	}

	/* Set parity */
	if (port->parity != NONE) {
		port->cur_termios.c_cflag |= PARENB;
		if (port->parity == ODD) {
			port->cur_termios.c_cflag |= PARODD;
		}
	}

	/* Always set CLOCAL ("local line") and CREAD ("enable receiver") */
	port->cur_termios.c_cflag |= (CLOCAL | CREAD);

	port->cur_termios.c_lflag |= ~(ICANON | ECHO);
	port->cur_termios.c_cc[VMIN] = (cc_t)0;
	port->cur_termios.c_cc[VTIME] = (cc_t)0;

	(void)tcsetattr(port->fd, TCSANOW, &port->cur_termios);

	return 0;
}

speed_t serial_validate_baudrate(int baudrate)
{
	switch (baudrate) {
		case 50:
			return B50;
		case 75:
			return B75;
		case 110:
			return B110;
		case 134:
			return B134;
		case 150:
			return B150;
		case 200:
			return B200;
		case 300:
			return B300;
		case 600:
			return B600;
		case 1200:
			return B1200;
		case 1800:
			return B1800;
		case 2400:
			return B2400;
		case 4800:
			return B4800;
		case 9600:
			return B9600;
		case 19200:
			return B19200;
		case 38400:
			return B38400;
#ifdef B57600
		case 57600:
			return B57600;
#endif /* B57600 */
#ifdef B115200
		case 115200:
			return B115200;
#endif /* B115200 */
#ifdef B230400
		case 230400:
			return B230400;
#endif /* B230400 */
#ifdef B460800
		case 460800:
			return B460800;
#endif /* B460800 */
#ifdef B500000
		case 500000:
			return B500000;
#endif /* B500000 */
#ifdef B576000
		case 576000:
			return B576000;
#endif /* B576000 */
#ifdef B921600
		case 921600:
			return B921600;
#endif /* B921600 */
#ifdef B1000000
		case 1000000:
			return B1000000;
#endif /* B1000000 */
#ifdef B1152000
		case 1152000:
			return B1152000;
#endif /* B1152000 */
#ifdef B1500000
		case 1500000:
			return B1500000;
#endif /* B1500000 */
#ifdef B2000000
		case 2000000:
			return B2000000;
#endif /* B2000000 */
#ifdef B2500000
		case 2500000:
			return B2500000;
#endif /* B2500000 */
#ifdef B3000000
		case 3000000:
			return B3000000;
#endif /* B3000000 */
#ifdef B3500000
		case 3500000:
			return B3500000;
#endif /* B3500000 */
#ifdef B4000000
		case 4000000:
			return B4000000;
#endif /* B4000000 */
	}

	/* For our purposes B0 is invalid */
	return B0;
}

void serial_mirror_data(serial_port_t *src, serial_port_t *dst)
{
	ssize_t count;

	count = read(src->fd, buffer, BUFFER_SIZE);

	if (count > 0) {
		(void)write(dst->fd, buffer, (size_t)count);
		(void)tcdrain(dst->fd);
	} else if (count == -1) {
		if (errno != EAGAIN && errno != EWOULDBLOCK) {
			fprintf(stderr, "Error reading from %s: %s\n",
				src->port_name, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
}

void serial_mirror_control(serial_port_t *src, serial_port_t *dst)
{
	ioctl(src->fd, TIOCMGET, &src->status);

	if (src->status & TIOCM_CTS) {
		dst->status |= TIOCM_RTS;
	} else {
		dst->status &= ~TIOCM_RTS;
	}

	if (src->status & (TIOCM_DSR | TIOCM_CAR)) {
		dst->status |= TIOCM_DTR;
	} else {
		dst->status &= ~TIOCM_DTR;
	}

	ioctl(dst->fd, TIOCMSET, &dst->status);
}
