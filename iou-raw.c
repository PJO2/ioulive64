// -------------
// ioulive64 - 2025 - GPLv2
// system operations
// -------------

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <netinet/if_ether.h>
#include <linux/if.h>
#include <linux/if_packet.h>

#include "ioulive64.h"

// This file holds the functions that deal with low-level packet I/O


// Initialize an Ethernet raw socket
int create_raw_socket(const char*name, int *dev_id)
{
	struct sockaddr_ll sa;
	struct packet_mreq mreq;
	int sock;
	struct ifreq if_req;
	int ioctl_fd;

    // Get interface index of Ethernet device
	memset((void *) &if_req, 0, sizeof(if_req));
	strncpy(if_req.ifr_name, name, sizeof(if_req.ifr_name) - 1);
	// create dummy fd
	if ((ioctl_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "raw_get_dev_index: socket: %s\n", strerror(errno));
		return (-1);
	}
	if (ioctl(ioctl_fd, SIOCGIFINDEX, &if_req) < 0) {
		fprintf(stderr, "raw_get_dev_index: SIOCGIFINDEX: %s\n", strerror(errno));
		close(ioctl_fd);
		return (-1);
	}
	close(ioctl_fd);

	// now create real privileged socket
	if ((sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1) {
		fprintf(stderr, "create_raw_socket: socket: %s\n", strerror(errno));
		return (-1);
	}

	memset(&sa, 0, sizeof(struct sockaddr_ll));
	sa.sll_family = AF_PACKET;
	sa.sll_protocol = htons(ETH_P_ALL);
	sa.sll_hatype = ARPHRD_ETHER;
	sa.sll_halen = ETH_ALEN;
	sa.sll_ifindex = if_req.ifr_ifindex;

	memset(&mreq, 0, sizeof(mreq));
	mreq.mr_ifindex = sa.sll_ifindex;
	mreq.mr_type = PACKET_MR_PROMISC;

	if (bind(sock, (struct sockaddr *) &sa, sizeof(struct sockaddr_ll)) == -1) {
		fprintf(stderr, "raw_init_socket: bind: %s\n", strerror(errno));
		close (sock);
		return (-1);
	}
	if (setsockopt(sock, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) == -1) {
		fprintf(stderr, "raw_init_socket: setsockopt: %s\n", strerror(errno));
		close (sock);
		return (-1);
	}
	if (verbosity>=VERY_VERBOSE) {
		printf ("raw socket %d opened for ethernet interface id %d\n", sock, if_req.ifr_ifindex);
	}
	// need to return device_id
	*dev_id = if_req.ifr_ifindex;
	return (sock);
} // create_raw_socket


// Send an Ethernet frame to the Ethernet device
ssize_t raw_send(int sock, int dev_id, char *buffer, size_t len)
{
	struct sockaddr_ll sa;

	memset(&sa, 0, sizeof(struct sockaddr_ll));
	sa.sll_family = AF_PACKET;
	sa.sll_protocol = htons(ETH_P_ALL);
	sa.sll_hatype = ARPHRD_ETHER;
	sa.sll_halen = ETH_ALEN;
	sa.sll_ifindex = dev_id;

	return( sendto(sock, buffer, len, 0, (struct sockaddr *) &sa, sizeof(sa)));
}

// Receive an Ethernet frame from device
ssize_t raw_recv(int sock, char *buffer, size_t len)
{
	return( recv(sock, buffer, len, 0));
}



