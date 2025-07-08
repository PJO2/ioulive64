// -----------------------
// ioulive64 2025 - GPLv2
// data plane operations
// -----------------------

#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "ioulive64.h"


#define IOULIVE_INTF_ID 0
#define IOU_HEADER_LEN  8


// creates listening socket and let's peer write in it
int create_listening_skt(int instance) {
int skt;
struct sockaddr_un server;

    // Create unix socket path to /tmp/netioX where X is the user id
    // should be then same user than the iou instance
    memset(&server, 0, sizeof(server));
    server.sun_family = AF_UNIX;
    sprintf(server.sun_path, "/tmp/netio%u/%d", getuid(), instance);
    if (verbosity>=VERBOSE) {
       printf("Opening server socket on %s\n", server.sun_path);
    }
    // Open server socket (IOUlive instance)
    if ( (skt = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0 ) {
        perror("opening server socket");
        return -1;
    }
    unlink (server.sun_path);
    if (bind(skt, (struct sockaddr *)&server, sizeof(server)) < 0) {
   		close (skt);
        perror("binding server socket");
        return -1;
    }
    return skt;
} // create_listening_skt


// connect to peer socket after that we will write in it
int connect_peer_skt (int peer_instance) {
int skt;
struct sockaddr_un router;

    memset(&router, 0, sizeof(router));
    router.sun_family = AF_UNIX;
	sprintf(router.sun_path, "/tmp/netio%u/%d", getuid(), peer_instance);

    // Open client socket to router
    if ((skt = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0 ) {
        perror("opening client socket");
        return -1;
    }

    if (verbosity>=VERBOSE) {
       printf("Connecting to instance #%u using unix socket %s\n", peer_instance, router.sun_path);
    }
    while (connect(skt, (struct sockaddr *)&router, sizeof(router)) < 0) {
        sleep(2);
    }
    if (verbosity>=VERY_VERBOSE) {
       printf("Connected to instance #%u\n", peer_instance, router.sun_path);
    }
    return skt;
} // connect_peer_skt



// receive from IOU, send to Ethernet
void iou_to_ethernet_loop(int rcv_sock, int raw_fd, int raw_dev_id, const struct S_netmap_unit *peer) {
    char buf[1514+IOU_HEADER_LEN];

    if (verbosity>=VERY_VERBOSE) 
           tsprintf("Waiting on socket %d for IOU frame\n", rcv_sock);
    while (1) {
        struct sockaddr_un peer_skt;
        socklen_t peerlen = sizeof(peer_skt);

        ssize_t rval = recvfrom(rcv_sock, buf, sizeof(buf), 0, (struct sockaddr *)&peer_skt, &peerlen);

        if (rval < 0) {
            perror("reading frame from IOU instance");
        } else if (rval == 0) {
            fprintf(stderr, "IOU instance ended connection\n");
        } else {
            raw_send(raw_fd, raw_dev_id, &buf[IOU_HEADER_LEN], rval-IOU_HEADER_LEN);
        }
        if (verbosity>=DEBUG)
              tsprintf("transmitted %d bytes from instance %d to Ethernet\n", 
                        rval-IOU_HEADER_LEN, peer->instance);
    }
} // iou_to_ethernet_loop

// receive from Ethernet, send to IOU
void ethernet_to_iou_loop(int iou_sock, int raw_fd, int live_instance, const struct S_netmap_unit *peer) {
    char buf[1514+IOU_HEADER_LEN];

    if (verbosity>=VERY_VERBOSE) 
        tsprintf("Waiting for Ethernet frame on socket %d\n", raw_fd);
    while (1) {
        ssize_t rval = raw_recv(raw_fd, &buf[IOU_HEADER_LEN], sizeof(buf)-IOU_HEADER_LEN);

        if (rval < 0) {
            perror("reading raw Ethernet frame");
        } else if (rval == 0) {
            fprintf(stderr, "raw Ethernet connection ended\n");
        } else {
            // itf_id = peer->slot + peer->port*16;
            buf[0] = peer->instance >> 8;
            buf[1] = peer->instance & 0xFF;
            buf[2] = live_instance >> 8;
            buf[3] = live_instance & 0xFF;
            buf[4] = peer->slot + peer->port*16;
            buf[5] = IOULIVE_INTF_ID;
            buf[6] = 1;
            buf[7] = 0;

            if (write(iou_sock, buf, rval+IOU_HEADER_LEN) < 0) {
                fprintf(stderr, "IOU instance terminated: waiting for reconnection\n");
                close(iou_sock);
                break;
            }
        }
        if (verbosity>=DEBUG)
              tsprintf("transmitted %d bytes from Ethernet to instance %d\n", rval, peer->instance);
    }
} // ethernet_to_iou_loop



// wrappers to decaps args
void *iou_to_ethernet(void *arg) {
    struct S_ioulive_args *args = (struct S_ioulive_args *)arg;
    int rcv_skt;

    // Create Unix sockets to receive from and send to
    if ( (rcv_skt=create_listening_skt(args->ioulive_inst)) <=0 ) {
		printf ("Error: can create unix socket:\n");
		exit(EXIT_FAILURE);
    }
    iou_to_ethernet_loop(rcv_skt, args->raw_fd, args->raw_dev_id, &args->peer);
    return NULL;
}

void *ethernet_to_iou(void *arg) {
    struct S_ioulive_args *args = (struct S_ioulive_args *)arg;
    int snd_skt;
    // put into a loop, to allow teh lab to de stopped then restarted
    while (1)
    {
        if (  (snd_skt=connect_peer_skt(args->peer.instance)) <=0  ) {
            printf ("Error: can not connect to peer\n");
            exit(EXIT_FAILURE);
        }

        ethernet_to_iou_loop(snd_skt, args->raw_fd, args->ioulive_inst, &args->peer);
    }
    return NULL;
}
