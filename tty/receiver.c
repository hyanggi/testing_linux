#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include "common.h"

void print_message(const char *message, ssize_t length)
{
	printf("Here is the message (%zd):\n", length);
	for (int i=0; i<length && i<500; ++i)
		printf("%c", message[i]);
	if (length > 500)
		printf("... %zd", length);
	printf("\n");
}

int run_with_port_opened(int port)
{
	if (setup_port(port) < 0)
		return -1;

	while (true) {
		char buffer[2000000];
		ssize_t length = read(port, buffer, sizeof buffer);
		if (length < 0) {
			perror("Receiving failed");
			return -1;
		}
		if (length == 0)
			return 0;
		print_message(buffer, length);
	}

	return 0;
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s serial_port\n", argv[0]);
		return EXIT_FAILURE;
	}

	int port = open(argv[1], O_RDWR | O_NOCTTY);
	if (port < 0) {
		perror("Opening serial device failed");
		return EXIT_FAILURE;
	}

	if (run_with_port_opened(port) < 0) {
		if (close(port) < 0)
			perror("Closing device failed");
		return EXIT_FAILURE;
	}

	if (close(port) < 0) {
		perror("Closing device failed");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
