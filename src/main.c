#include <errno.h>
#include <libgen.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "serial.h"

#define POLL_TIMEOUT 25

static void print_help()
{
	printf("Usage: ttymirror [options]\n\n");
	printf("Options:\n");
	printf("  -s <source_port>   Path to the source serial port (required)\n");
	printf("  -m <mirror_port>   Path to the mirror serial port (required)\n");
	printf("  -b <baudrate>      Baud rate for both ports (default: 9600)\n");
	printf("  -d <databits>      Number of data bits for both ports (5-8, default: 8)\n");
	printf("  -h                 Display this help message\n");
	printf("  -p <stopbits>      Number of stop bits for both ports (1 or 2, default: 1)\n");
	printf("  -y <parity>        Parity for both ports (n=none, e=even, o=odd, default: n)\n\n");
	printf("Example:\n");
	printf("  ttymirror -s /dev/ttyS0 -m /dev/ttyS1 -b 19200 -d 8 -p 1 -y n\n\n");
}

static serial_port_t source = {
	.port_name = NULL,
	.fd = -1,
	.baudrate = B9600,
	.databits = 8,
	.stopbits = 1,
	.parity = NONE
};

static serial_port_t mirror = {
	.port_name = NULL,
	.fd = -1,
	.baudrate = B9600,
	.databits = 8,
	.stopbits = 1,
	.parity = NONE
};

static void cleanup(int signal) {
	if (source.fd != -1) {
		(void)tcsetattr(source.fd, TCSANOW, &source.old_termios);
		(void)close(source.fd);
	}

	if (mirror.fd != -1) {
		(void)tcsetattr(mirror.fd, TCSANOW, &mirror.old_termios);
		(void)close(mirror.fd);
	}

	exit(signal);
}


int main(int argc, char **argv) {
	int c;
	speed_t baudrate;
	int databits, stopbits;

	struct pollfd poll_fds[2];
	int poll_rv;

	while ((c = getopt(argc, argv, "b:d:hm:p:s:y:")) != -1) {
		switch (c) {
			case 'b':
				baudrate = serial_validate_baudrate(
						atoi(optarg));
				if (baudrate == B0) {
					fprintf(stderr, "Invalid baudrate\n");
					exit(EXIT_FAILURE);
				}

				source.baudrate = baudrate;
				mirror.baudrate = baudrate;

				break;
			case 'd':
				databits = atoi(optarg);
				if (databits < 5 || databits > 8) {
					fprintf(stderr, "Invalid databits\n");
					exit(EXIT_FAILURE);
				}

				source.databits = databits;
				mirror.databits = databits;

				break;
			case 'h':
				print_help();
				exit(EXIT_SUCCESS);
			case 'm':
				mirror.port_name = optarg;
				break;
			case 'p':
				stopbits = atoi(optarg);
				if (stopbits != 1 && stopbits != 2) {
					fprintf(stderr, "Invalid stopbits\n");
					exit(EXIT_FAILURE);
				}

				source.stopbits = stopbits;
				mirror.stopbits = stopbits;

				break;
			case 's':
				source.port_name = optarg;
				break;
			case 'y':
				if (strcmp(optarg, "n") == 0) {
					source.parity = NONE;
					mirror.parity = NONE;
				} else if (strcmp(optarg, "e") == 0) {
					source.parity = EVEN;
					mirror.parity = EVEN;
				} else if (strcmp(optarg, "o") == 0) {
					source.parity = ODD;
					mirror.parity = ODD;
				} else {
					fprintf(stderr, "Invalid parity\n");
					exit(EXIT_FAILURE);
				}
				break;
			case '?':
			default:
				print_help();
				exit(EXIT_FAILURE);
		}

	}

	if (source.port_name == NULL || mirror.port_name == NULL) {
		print_help();
		exit(EXIT_FAILURE);
	}

	source.fd = serial_open(source.port_name);
	if (source.fd == -1) {
		exit(EXIT_FAILURE);
	}

	mirror.fd = serial_open(mirror.port_name);
	if (mirror.fd == -1) {
		exit(EXIT_FAILURE);
	}

	(void)signal(SIGINT, cleanup);
	(void)signal(SIGTERM, cleanup);

	serial_set_options(&source);
	serial_set_options(&mirror);

	poll_fds[0].fd = source.fd;
	poll_fds[0].events = POLLIN;
	poll_fds[1].fd = mirror.fd;
	poll_fds[1].events = POLLIN;

	while (true) {
		poll_rv = poll(poll_fds, 2, POLL_TIMEOUT);
		if (poll_rv < 0) {
			fprintf(stderr, "Error polling: %s", strerror(errno));
			exit(EXIT_FAILURE);
		}

		if (poll_fds[0].revents & POLLIN) {
			serial_mirror_data(&source, &mirror);
		}

		if (poll_fds[1].revents & POLLIN) {
			serial_mirror_data(&mirror, &source);
		}
	}

	cleanup(0);

	return 0;
}
