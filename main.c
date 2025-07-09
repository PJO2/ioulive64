// -----------------------
// ioulive64 - 2025 - GPLv2
// refactorization of the project ioulive86 
// https://github.com/jlgaddis/ioulive86
// -----------------------

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "ioulive64.h"

int verbosity=0;

void print_usage(void) {
    printf("Usage: ioulive64 [-v [-v]] -i <interface> [-n NETMAP] <instance ID>\n");
}


// -----------------
// Main
int main (int argc, char *argv[] )
{
	char netmap_path[255];
    char intf_name[20];
	unsigned int instance=0;
    struct S_netmap_unit peer;
	int raw_fd; // socket for ethernet snooping
    int eth_id; // id of the ehernet interface

    // parse command line
	printf("%s\n\n", argv[0]);
    if (parse_args(argc, argv, intf_name, netmap_path, &instance) < 0) {
        exit(EXIT_FAILURE);
    }

    // parse netmap file and extract peer data
    if (parse_netmap(netmap_path, instance, &peer) < 0) {
        exit(EXIT_FAILURE);
    }

    // Create a raw Ethernet socket for packet capture/transmission.
    // This is a privileged operation that requires CAP_NET_RAW capability.
    // Make sure the binary has the appropriate capability set:
    //    sudo setcap cap_net_raw=eip ./ioulive64
    if ( (raw_fd = create_raw_socket (intf_name, &eth_id)) == -1 ) {
		fprintf(stderr, "Error creating Ethernet socket on '%s': %s\n", intf_name, strerror(errno));
        printf ("  Invalid Ethernet device specified or insufficent privileges\n");
        printf ("  Make sure that %s program has received *cap_net_raw* privileges\n", argv[0]);
        printf ("  Please consider running follwing command\n");
        printf ("          sudo setcap cap_net_raw=eip ./%s\n", argv[0]);
        printf ("  And start %s program again in user mode\n", argv[0]);
		exit(EXIT_FAILURE);
    }

    // Ensure /tmp/netio directory exists
    char dir_path[128];
    snprintf(dir_path, sizeof(dir_path), "/tmp/netio%u", getuid());
    if (mkdir(dir_path, 0755) < 0 && errno != EEXIST) {
        perror("mkdir: can not create directory for sockets");
        return -1;
    }

    // start the threads for handling data
    pthread_t eth_tid, iou_tid;
    struct S_ioulive_args args = { raw_fd, eth_id,  instance,  peer };

    if (    pthread_create(&eth_tid, NULL, iou_to_ethernet, &args) != 0
         && pthread_create(&iou_tid, NULL, ethernet_to_iou, &args) != 0 ) {
        perror("pthread_create (eth_tid)");
        exit(EXIT_FAILURE);
    }
    sleep(3);
    printf ("Hit Ctrl-C to stop process\n");
    // And wait for Ctrl-C....
    pthread_join(eth_tid, NULL);
    pthread_join(iou_tid, NULL);

    close (raw_fd);

} // main



