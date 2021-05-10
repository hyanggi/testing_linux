#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include "common.h"

int stream_send(int sockfd, const char *buf, size_t len, int flags)
{
	size_t len_sent = 0;
	while (len_sent < len) {
		ssize_t res = send(sockfd, buf + len_sent, len - len_sent, flags);
		if (res < 0) {
			perror("Sending failed");
			return -1;
		}
		else if (res == 0) {
			fprintf(stderr, "Error: send returned 0\n");
			return -1;
		}
		else
			len_sent += (size_t)res;
	}
	return 0;
}

int run_with_socket_opened(int sockfd, const struct addrinfo *addr)
{
	char all_a_s[50000];
	memset(all_a_s, 'a', sizeof all_a_s);
	if (addr->ai_protocol == IPPROTO_TCP) {
		// Connect and send byte-stream
		if (connect(sockfd, addr->ai_addr, addr->ai_addrlen) != 0) {
			perror("Connecting failed");
			return -1;
		}
		for (int i = 0; i<100; ++i)
			if (stream_send(sockfd, "12", 2, 0) < 0)
				return -1;
		if (stream_send(sockfd, all_a_s, sizeof all_a_s, 0) < 0)
			return -1;
	}
	else if (addr->ai_protocol == IPPROTO_DCCP || (addr->ai_protocol == IPPROTO_SCTP && addr->ai_socktype == SOCK_STREAM)) {
		// Connect and send datagrams
		if (connect(sockfd, addr->ai_addr, addr->ai_addrlen) != 0) {
			perror("Connecting failed");
			return -1;
		}
		for (int i = 0; i<5; ++i)
			if (send(sockfd, "12", 2, 0) < 0) {
				perror("Sending failed");
				return -1;
			}
		if (send(sockfd, all_a_s, sizeof all_a_s, 0) < 0) {
			perror("Sending failed");
			return -1;
		}
		if (addr->ai_protocol == IPPROTO_DCCP)
			usleep(10000); // Wait for data to be successfully transmitted before we close the connection
	}
	else {
		// send datagrams without connecting
		for (int i = 0; i<5; ++i)
			if (sendto(sockfd, "12", 2, 0, addr->ai_addr, addr->ai_addrlen) < 0) {
				perror("Sending failed");
				return -1;
			}
		if (sendto(sockfd, all_a_s, sizeof all_a_s, 0, addr->ai_addr, addr->ai_addrlen) < 0) {
			perror("Sending failed");
			return -1;
		}
	}
	return 0;
}

int run_with_addrinfo(const struct addrinfo *addr)
{
	// Open socket
	int sockfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
	if (sockfd < 0) {
		perror("Opening socket failed");
		return -1;
	}

	if (run_with_socket_opened(sockfd, addr) < 0) {
		if (close(sockfd) < 0)
			perror("Closing socket failed");
		return -1;
	}

	if (close(sockfd) < 0) {
		perror("Closing socket failed");
		return -1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	if (argc < 4) {
		fprintf(stderr, "Usage: %s server protocol port\n", argv[0]);
		return EXIT_FAILURE;
	}
	int socktype;
	int protocol;
	if (determine_protocol(argv[2], &socktype, &protocol) < 0)
		return EXIT_FAILURE;

	struct addrinfo hints = {
		.ai_flags = 0,
		.ai_family = AF_UNSPEC,
		.ai_socktype = socktype,
		.ai_protocol = protocol,
		.ai_addrlen = 0,
		.ai_addr = NULL,
		.ai_canonname = NULL,
		.ai_next = NULL,
	};
	struct addrinfo *result;
	int errcode = getaddrinfo(argv[1], argv[3], &hints, &result);
	if (errcode != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(errcode));
		return EXIT_FAILURE;
	}

	if (run_with_addrinfo(result) < 0) {
		freeaddrinfo(result);
		return EXIT_FAILURE;
	}

	freeaddrinfo(result);
	return EXIT_SUCCESS;
}
