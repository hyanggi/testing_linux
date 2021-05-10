#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netpacket/packet.h>
#include <linux/if_ether.h>

void print_message(const char *message, ssize_t length)
{
	printf("Here is the message (%zd):\n", length);
	for (int i=0; i<length && i<500; ++i)
		if (message[i] >= 0x20 && message[i] < 0x7f)
			printf("%c", message[i]);
		else
			printf("\\%02hhx", message[i]);
	if (length > 500)
		printf("... %zd", length);
	printf("\n");
}

void print_address(const struct sockaddr_ll *addr)
{
	const unsigned char *mac_addr = addr->sll_addr;
	printf("%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx.\n", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
}

int run_with_socket_opened(int sockfd, const char *if_name, uint16_t protocol)
{
	struct ifreq ifr;
	if (strlen(if_name) + 1 > sizeof ifr.ifr_name) {
		fprintf(stderr, "Interface name too long.\n");
		return -1;
	}
	strcpy(ifr.ifr_name, if_name);
	if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1) {
		perror("Unrecognized interface name");
		return -1;
	}
	int ifindex = ifr.ifr_ifindex;

	for (int i=0; i<6;) {
		struct sockaddr_ll sender_addr;
		socklen_t sender_addr_len = sizeof sender_addr;
		char buffer[1500];

		ssize_t length = recvfrom(sockfd, buffer, sizeof buffer, 0, (struct sockaddr *)&sender_addr, &sender_addr_len);
		if (length < 0) {
			perror("Receiving failed");
			return -1;
		}
		if (sender_addr.sll_ifindex != ifindex)
			continue;
		if (protocol != ETH_P_ALL)
			if (sender_addr.sll_protocol != htons(protocol))
				continue;

		printf("Received from ");
		print_address(&sender_addr);
		print_message(buffer, length);
		++i;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "Usage: %s interface protocol\n", argv[0]);
		fprintf(stderr, "       protocol 3 for any incoming frames\n");
		fprintf(stderr, "       protocol 4 for incoming IEEE 802.3 frames\n");
		fprintf(stderr, "       protocol >= 600 for EtherType values\n");
		return EXIT_FAILURE;
	}

	uint16_t protocol;
	if (sscanf(argv[2], "%" SCNx16, &protocol) != 1) {
		fprintf(stderr, "Unrecognized protocol.\n");
		return EXIT_FAILURE;
	}

	int sockfd = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_ALL));
	if (sockfd < 0) {
		perror("Opening socket failed");
		return EXIT_FAILURE;
	}

	if (run_with_socket_opened(sockfd, argv[1], protocol) < 0) {
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
