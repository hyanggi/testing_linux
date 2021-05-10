#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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

void print_address(const struct sockaddr_in6 *addr)
{
	char addr_text[INET6_ADDRSTRLEN];
	const char erro_text[] = "(unknown address)";
	if (inet_ntop(AF_INET6, &(addr->sin6_addr), addr_text, INET6_ADDRSTRLEN) == NULL)
		printf("%s. Port %" PRIu16 ".\n", erro_text, ntohs(addr->sin6_port));
	else
		printf("%s. Port %" PRIu16 ".\n", addr_text, ntohs(addr->sin6_port));
}

int run_with_socket_connected(int connected_sockfd, const struct sockaddr_in6 *client_addr)
{
	printf("Connection from ");
	print_address(client_addr);

	while (true) {
		char buffer[2000000];
		ssize_t length = recv(connected_sockfd, buffer, sizeof buffer, 0);
		if (length < 0) {
			perror("Receiving failed");
			return -1;
		}
		if (length == 0)
			return 0;
		print_message(buffer, length);
	}
}

int run_with_socket_opened(int sockfd, uint16_t port, int socktype, int protocol)
{
	// Bind socket
	struct sockaddr_in6 serv_addr = {
		.sin6_family = AF_INET6,
		.sin6_port = htons(port),
		.sin6_flowinfo = 0,
		.sin6_addr = IN6ADDR_ANY_INIT,
		.sin6_scope_id = 0,
	};
	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof serv_addr) < 0) {
		perror("Binding socket failed");
		return -1;
	}

	// Listen on socket
	if (protocol == IPPROTO_TCP || protocol == IPPROTO_DCCP || protocol == IPPROTO_SCTP)
		if (listen(sockfd, 5) < 0) {
			perror("Listening on socket failed");
			return -1;
		}

	if (protocol == IPPROTO_TCP || protocol == IPPROTO_DCCP || (protocol == IPPROTO_SCTP && socktype == SOCK_STREAM)) {
		// Accept connection and receive
		struct sockaddr_in6 client_addr;
		socklen_t client_addr_len = sizeof client_addr;

		int connected_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
		if (connected_sockfd < 0) {
			perror("Accepting connection failed");
			return -1;
		}

		if (run_with_socket_connected(connected_sockfd, &client_addr) < 0) {
			if (close(connected_sockfd) < 0)
				perror("Closing socket failed");
			return -1;
		}

		if (close(connected_sockfd) < 0) {
			perror("Closing socket failed");
			return -1;
		}
	}
	else {
		// Receive without accepting connection
		for (int i=0; i<6; ++i) {
			struct sockaddr_in6 client_addr;
			socklen_t client_addr_len = sizeof client_addr;
			char buffer[2000000];

			ssize_t length = recvfrom(sockfd, buffer, sizeof buffer, 0, (struct sockaddr *)&client_addr, &client_addr_len);
			if (length < 0) {
				perror("Receiving failed");
				return -1;
			}

			printf("Received from ");
			print_address(&client_addr);
			print_message(buffer, length);
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "Usage: %s protocol port\n", argv[0]);
		return EXIT_FAILURE;
	}
	int socktype;
	int protocol;
	if (determine_protocol(argv[1], &socktype, &protocol) < 0)
		return EXIT_FAILURE;

	// Open socket
	int sockfd = socket(AF_INET6, socktype, protocol);
	if (sockfd < 0) {
		perror("Opening socket failed");
		return EXIT_FAILURE;
	}

	if (run_with_socket_opened(sockfd, (uint16_t)atoi(argv[2]), socktype, protocol) < 0) {
		if (close(sockfd) < 0)
			perror("Closing socket failed");
		return EXIT_FAILURE;
	}

	if (close(sockfd) < 0) {
		perror("Closing socket failed");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
