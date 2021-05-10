#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

int run_with_socket_opened(int sockfd, const struct addrinfo *addr)
{
	char all_a_s[50000];
	memset(all_a_s, 'a', sizeof all_a_s);
	for (int i = 0; i<5; ++i)
		if (sendto(sockfd, "12", 2, 0, addr->ai_addr, addr->ai_addrlen) < 0) {
			perror("Sending failed");
			return -1;
		}
	if (sendto(sockfd, all_a_s, sizeof all_a_s, 0, addr->ai_addr, addr->ai_addrlen) < 0) {
		perror("Sending failed");
		return -1;
	}
	return 0;
}

int run_with_addrinfo(const struct addrinfo *addr)
{
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
	if (argc < 3) {
		fprintf(stderr, "Usage: %s hostname ip_protocol_number\n", argv[0]);
		return EXIT_FAILURE;
	}

	struct addrinfo hints = {
		.ai_flags = 0,
		.ai_family = AF_UNSPEC,
		.ai_socktype = SOCK_RAW,
		.ai_protocol = atoi(argv[2]),
		.ai_addrlen = 0,
		.ai_addr = NULL,
		.ai_canonname = NULL,
		.ai_next = NULL,
	};
	struct addrinfo *result;
	int errcode = getaddrinfo(argv[1], NULL, &hints, &result);
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
