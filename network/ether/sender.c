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

int run_with_socket_opened(int sockfd, const char *if_name, const unsigned char dst_addr[], uint16_t protocol)
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

	struct sockaddr_ll addr = {
		.sll_family = AF_PACKET,
		.sll_protocol = htons(protocol),
		.sll_ifindex = ifindex,
		.sll_hatype = 0,
		.sll_pkttype = 0,
		.sll_halen = 6,
		.sll_addr = {dst_addr[0], dst_addr[1], dst_addr[2], dst_addr[3], dst_addr[4], dst_addr[5], 0, 0}
	};

	char all_a_s[1000];
	memset(all_a_s, 'a', sizeof all_a_s);
	for (int i = 0; i<5; ++i)
		if (sendto(sockfd, "12", 2, 0, (struct sockaddr *)&addr, sizeof addr) < 0) {
			perror("Sending failed");
			return -1;
		}
	if (sendto(sockfd, all_a_s, sizeof all_a_s, 0, (struct sockaddr *)&addr, sizeof addr) < 0) {
		perror("Sending failed");
		return -1;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	if (argc < 4) {
		fprintf(stderr, "Usage: %s sending_interface destination_MAC_address protocol\n", argv[0]);
		fprintf(stderr, "       protocol 1 for sending IEEE 802.3 frames\n");
		fprintf(stderr, "       protocol >= 600 for EtherType values\n");
		return EXIT_FAILURE;
	}

	uint16_t protocol;
	if (sscanf(argv[3], "%" SCNx16, &protocol) != 1) {
		fprintf(stderr, "Unrecognized protocol.\n");
		return EXIT_FAILURE;
	}

	unsigned char dst_addr[6];
	if (sscanf(argv[2], "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", dst_addr, dst_addr+1, dst_addr+2, dst_addr+3, dst_addr+4, dst_addr+5) != 6) {
		fprintf(stderr, "Unrecognized MAC address.\n");
		return EXIT_FAILURE;
	}

	int sockfd = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_ALL));
	if (sockfd < 0) {
		perror("Opening socket failed");
		return EXIT_FAILURE;
	}

	if (run_with_socket_opened(sockfd, argv[1], dst_addr, protocol) < 0) {
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
