#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void print_message(const char *message, ssize_t length)
{
	printf("Here is the message (%zd):\n", length);
	for (int i=0; i<length && i<500; ++i)
		printf("%c", message[i]);
	if (length > 500)
		printf("... %zd", length);
	printf("\n");
}

void print_ipv4_address(const struct sockaddr_in *addr)
{
	char addr_text[INET_ADDRSTRLEN];
	const char erro_text[] = "(unknown address)";
	if (inet_ntop(AF_INET, &(addr->sin_addr), addr_text, INET_ADDRSTRLEN) == NULL)
		printf("%s.\n", erro_text);
	else
		printf("%s.\n", addr_text);
}

void print_ipv6_address(const struct sockaddr_in6 *addr)
{
	char addr_text[INET6_ADDRSTRLEN];
	const char erro_text[] = "(unknown address)";
	if (inet_ntop(AF_INET6, &(addr->sin6_addr), addr_text, INET6_ADDRSTRLEN) == NULL)
		printf("%s.\n", erro_text);
	else
		printf("%s.\n", addr_text);
}

int run_with_ipv4_socket_opened(int sockfd)
{
	for (int i=0; i<6; ++i) {
		struct sockaddr_in sender_addr;
		socklen_t sender_addr_len = sizeof sender_addr;
		char buffer[2000000];

		ssize_t length = recvfrom(sockfd, buffer, sizeof buffer, 0, (struct sockaddr *)&sender_addr, &sender_addr_len);
		if (length < 0) {
			perror("Receiving failed");
			return -1;
		}

		printf("Received from ");
		print_ipv4_address(&sender_addr);
		int header_length = (buffer[0] & 0xF) * 4;
		print_message(buffer + header_length, length - header_length);
	}
	return 0;
}

int run_with_ipv6_socket_opened(int sockfd)
{
	for (int i=0; i<6; ++i) {
		struct sockaddr_in6 sender_addr;
		socklen_t sender_addr_len = sizeof sender_addr;
		char buffer[2000000];

		ssize_t length = recvfrom(sockfd, buffer, sizeof buffer, 0, (struct sockaddr *)&sender_addr, &sender_addr_len);
		if (length < 0) {
			perror("Receiving failed");
			return -1;
		}

		printf("Received from ");
		print_ipv6_address(&sender_addr);
		print_message(buffer, length);
	}
	return 0;
}

int main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "Usage: %s ipv4|ipv6 ip_protocol_number\n", argv[0]);
		return EXIT_FAILURE;
	}
	int ip_protocol_number = atoi(argv[2]);
	if (strcmp(argv[1], "ipv4") == 0) {
		int sockfd = socket(AF_INET, SOCK_RAW, ip_protocol_number);
		if (sockfd < 0) {
			perror("Opening socket failed");
			return EXIT_FAILURE;
		}

		if (run_with_ipv4_socket_opened(sockfd) < 0) {
			if (close(sockfd) < 0)
				perror("Closing socket failed");
			return EXIT_FAILURE;
		}

		if (close(sockfd) < 0) {
			perror("Closing socket failed");
			return EXIT_FAILURE;
		}
	}
	else if (strcmp(argv[1], "ipv6") == 0) {
		int sockfd = socket(AF_INET6, SOCK_RAW, ip_protocol_number);
		if (sockfd < 0) {
			perror("Opening socket failed");
			return EXIT_FAILURE;
		}

		if (run_with_ipv6_socket_opened(sockfd) < 0) {
			if (close(sockfd) < 0)
				perror("Closing socket failed");
			return EXIT_FAILURE;
		}

		if (close(sockfd) < 0) {
			perror("Closing socket failed");
			return EXIT_FAILURE;
		}
	}
	else {
		fprintf(stderr, "Usage: %s ipv4|ipv6 ip_protocol_number\n", argv[0]);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
