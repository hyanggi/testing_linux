#include <stdio.h>
#include <string.h>
#include <netdb.h>

int determine_protocol(const char protocol_str[], int *socktype, int *protocol)
{
	if (strcmp(protocol_str, "tcp") == 0) {
		*socktype = SOCK_STREAM;
		*protocol = IPPROTO_TCP;
	}
	else if (strcmp(protocol_str, "dccp") == 0) { // Datagram Congestion Control Protocol
		*socktype = SOCK_DCCP;
		*protocol = IPPROTO_DCCP;
	}
	else if (strcmp(protocol_str, "sctp11") == 0) { // one-to-one (TCP) style API of SCTP
		*socktype = SOCK_STREAM;
		*protocol = IPPROTO_SCTP;
	}
	else if (strcmp(protocol_str, "sctp1m") == 0) { // one-to-many style API of SCTP
		*socktype = SOCK_SEQPACKET;
		*protocol = IPPROTO_SCTP;
	}
	else if (strcmp(protocol_str, "udp") == 0) {
		*socktype = SOCK_DGRAM;
		*protocol = IPPROTO_UDP;
	}
	else if (strcmp(protocol_str, "udplite") == 0) {
		*socktype = SOCK_DGRAM;
		*protocol = IPPROTO_UDPLITE;
	}
	else {
		fprintf(stderr, "Protocol can only be tcp, dccp, sctp11, sctp1m, udp or udplite.\n");
		return -1;
	}
	return 0;
}
