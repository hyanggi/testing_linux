#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/if_arp.h>
#include <linux/llc.h>

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

int run_with_socket_opened(int sockfd, const char *if_name, unsigned char local_sap, const unsigned char mac_addr[], unsigned char remote_sap, int service_type)
{
	if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, if_name, (socklen_t)(strlen(if_name) + 1)) < 0) {
		perror("Binding socket to interface failed");
		return -1;
	}
	struct sockaddr_llc local_addr = {
		.sllc_family = AF_LLC,
		.sllc_arphrd = ARPHRD_ETHER,
		.sllc_sap = local_sap,
	};
	if (bind(sockfd, (struct sockaddr *)&local_addr, sizeof local_addr) < 0) {
		perror("Binding socket failed");
		return -1;
	}

	struct sockaddr_llc addr = {
		.sllc_family = AF_LLC,
		.sllc_arphrd = ARPHRD_ETHER,
		.sllc_sap = remote_sap,
		.sllc_mac = {mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]}
	};

	char all_a_s[1000];
	memset(all_a_s, 'a', sizeof all_a_s);
	if (service_type == 2) { // Connect and send stream
		if (connect(sockfd, (struct sockaddr *)&addr, sizeof addr) != 0) {
			perror("Connecting failed");
			return -1;
		}
		for (int i = 0; i<5; ++i)
			if (stream_send(sockfd, "12", 2, 0) < 0)
				return -1;
		if (stream_send(sockfd, all_a_s, sizeof all_a_s, 0) < 0)
			return -1;
	}
	else { // Send frames without connecting
		for (int i = 0; i<5; ++i)
			if (sendto(sockfd, "12", 2, 0, (struct sockaddr *)&addr, sizeof addr) < 0) {
				perror("Sending failed");
				return -1;
			}
		if (sendto(sockfd, all_a_s, sizeof all_a_s, 0, (struct sockaddr *)&addr, sizeof addr) < 0) {
			perror("Sending failed");
			return -1;
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	if (argc < 6) {
		fprintf(stderr, "Usage: %s sending_interface sending_SAP destination_MAC_address destination_SAP service_type\n", argv[0]);
		return EXIT_FAILURE;
	}
	int service_type = atoi(argv[5]);
	if (service_type != 1 && service_type != 2) {
		fprintf(stderr, "Service type must be 1 or 2\n");
		return EXIT_FAILURE;
	}
	// Linux only supports LLC type 1 (unacknowledged connectionless) and type 2 (connection-oriented),
	// and does not support LLC type 3 (acknowledged connectionless).
	// Also, the Linux support for LLC type 2 sees data as a byte stream, and does not preserve message boundaries,
	// even though LLC type 2 is not designed this way.

	unsigned char local_sap;
	if (sscanf(argv[2], "%hhx", &local_sap) != 1) {
		fprintf(stderr, "Unrecognized sending_SAP.\n");
		return EXIT_FAILURE;
	}

	unsigned char mac_addr[6];
	if (sscanf(argv[3], "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", mac_addr, mac_addr+1, mac_addr+2, mac_addr+3, mac_addr+4, mac_addr+5) != 6) {
		fprintf(stderr, "Unrecognized MAC address.\n");
		return EXIT_FAILURE;
	}

	unsigned char remote_sap;
	if (sscanf(argv[4], "%hhx", &remote_sap) != 1) {
		fprintf(stderr, "Unrecognized destination_SAP.\n");
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

	if (run_with_socket_opened(sockfd, argv[1], local_sap, mac_addr, remote_sap, service_type) < 0) {
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
