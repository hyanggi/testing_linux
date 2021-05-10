#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <linux/if_arp.h>
#include <linux/llc.h>

void print_message(const char *message, ssize_t length)
{
	printf("Here is the message (%zd):\n", length);
	for (int i=0; i<length && i<500; ++i)
		switch (message[i]) {
			case 0:
				printf("\\0");
				break;
			default:
				printf("%c", message[i]);
		}
	if (length > 500)
		printf("... %zd", length);
	printf("\n");
}

void print_address(const struct sockaddr_llc *addr)
{
	const unsigned char *mac_addr = addr->sllc_mac;
	unsigned char sap = addr->sllc_sap;
	printf("%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx SAP:%hhx.\n", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5], sap);
}

int run_with_socket_connected(int connected_sockfd, const struct sockaddr_llc *addr)
{
	printf("Connection from ");
	print_address(addr);

	while (true) {
		char buffer[1500];
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

int run_with_socket_opened(int sockfd, const char *if_name, unsigned char sap, int service_type)
{
	if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, if_name, (socklen_t)(strlen(if_name) + 1)) < 0) {
		perror("Binding socket to interface failed");
		return -1;
	}
	struct sockaddr_llc local_addr = {
		.sllc_family = AF_LLC,
		.sllc_arphrd = ARPHRD_ETHER,
		.sllc_sap = sap,
	};
	if (bind(sockfd, (struct sockaddr *)&local_addr, sizeof local_addr) < 0) {
		perror("Binding socket failed");
		return -1;
	}

	if (service_type == 2) { // Accept connection and receive
		if (listen(sockfd, 5) < 0) {
			perror("Listening on socket failed");
			return -1;
		}

		struct sockaddr_llc sender_addr;
		socklen_t sender_addr_len = sizeof sender_addr;

		int connected_sockfd = accept(sockfd, (struct sockaddr *)&sender_addr, &sender_addr_len);
		if (connected_sockfd < 0) {
			perror("Accepting connection failed");
			return -1;
		}

		if (run_with_socket_connected(connected_sockfd, &sender_addr) < 0) {
			if (close(connected_sockfd) < 0)
				perror("Closing socket failed");
			return -1;
		}

		if (close(connected_sockfd) < 0) {
			perror("Closing socket failed");
			return -1;
		}
	}
	else { // Receive without accepting connection
		for (int i=0; i<6; ++i) {
			struct sockaddr_llc sender_addr;
			socklen_t sender_addr_len = sizeof sender_addr;
			char buffer[1500];

			ssize_t length = recvfrom(sockfd, buffer, sizeof buffer, 0, (struct sockaddr *)&sender_addr, &sender_addr_len);
			if (length < 0) {
				perror("Receiving failed");
				return -1;
			}

			printf("Received from ");
			print_address(&sender_addr);
			print_message(buffer, length);
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	if (argc < 4) {
		fprintf(stderr, "Usage: %s interface SAP service_type\n", argv[0]);
		return EXIT_FAILURE;
	}
	int service_type = atoi(argv[3]);
	if (service_type != 1 && service_type != 2) {
		fprintf(stderr, "Service type must be 1 or 2\n");
		return EXIT_FAILURE;
	}
	// Linux only supports LLC type 1 (unacknowledged connectionless) and type 2 (connection-oriented),
	// and does not support LLC type 3 (acknowledged connectionless).
	// Also, the Linux support for LLC type 2 sees data as a byte stream, and does not preserve message boundaries,
	// even though LLC type 2 is not designed this way.

	unsigned char sap;
	if (sscanf(argv[2], "%hhx", &sap) != 1) {
		fprintf(stderr, "Unrecognized SAP.\n");
		return EXIT_FAILURE;
	}

	int sockfd;
	if (service_type == 1)
		sockfd = socket(AF_LLC, SOCK_DGRAM, 0);
	else
		sockfd = socket(AF_LLC, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("Opening socket failed");
		return EXIT_FAILURE;
	}

	if (run_with_socket_opened(sockfd, argv[1], sap, service_type) < 0) {
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
