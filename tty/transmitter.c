#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "common.h"

int stream_write(int fd, const char *buf, size_t len)
{
	size_t len_written = 0;
	while (len_written < len) {
		ssize_t res = write(fd, buf + len_written, len - len_written);
		if (res < 0) {
			perror("Writing failed");
			return -1;
		}
		else if (res == 0) {
			fprintf(stderr, "Error: write returned 0\n");
			return -1;
		}
		else
			len_written += (size_t)res;
	}
	return 0;
}

int run_with_port_opened(int port)
{
	if (setup_port(port) < 0)
		return -1;

	char all_a_s[50000];
	memset(all_a_s, 'a', sizeof all_a_s);
	for (int i = 0; i<100; ++i)
		if (stream_write(port, "12", 2) < 0)
			return -1;
	if (stream_write(port, all_a_s, sizeof all_a_s) < 0)
		return -1;

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
