// -----------------------
// ioulive64 2025 - GPLv2
// Parse args + NETMAP file
// -----------------------

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "ioulive64.h"


// -----------------
// Parsing arguments
int parse_args(int argc, char *argv[], char *intf, char *netmap_path, unsigned int *instance) {
    int opt;
    int interface_set = 0;

    strcpy(netmap_path, "NETMAP");  // default value

    while ((opt = getopt(argc, argv, "i:n:v")) != -1) {
        switch (opt) {
            case 'i':
                strncpy(intf, optarg, 19);
                intf[19] = '\0';
                interface_set = 1;
                break;
            case 'n':
                strncpy(netmap_path, optarg, 254);
                netmap_path[254] = '\0';
                break;
			case 'v':
			    verbosity++;
				break;
            default:
                print_usage();
                return -1;
        }
    }

    if (!interface_set || optind >= argc) {
        print_usage();
        return -1;
    }

    *instance = atoi(argv[optind]);
    if (*instance == 0) {
        fprintf(stderr, "Invalid instance ID: %s\n", argv[optind]);
        print_usage();
        return -1;
    }

   if (verbosity>=VERY_VERBOSE) {
	   printf("Netmap path = %s\n", netmap_path);
	   printf("ioulive Instance = %d\n\n", *instance);
   }
return 0;
} // parse_args


// -----------------
// Pase Netmap
// now find the ioulive86 instance ID either at the left or right of the meshing specification
// -----------------

int parse_netmap(const char *netmap_path, unsigned int live_instance, struct S_netmap_unit *iou) {
    FILE *infp = fopen(netmap_path, "rt");
    char linein[256];				   // current line
    char left[127], right[127];	       // split in left and right columns
	struct S_netmap_unit src, dst;     // slpit in fields
    int line_num = 1;

    if (!infp) {
        printf("Cannot open %s\n\n", netmap_path);
        return -1;
    }

	iou->instance = 0;

    while (    fgets(linein, sizeof(linein), infp)     // not end of file
            && iou->instance==0 )                      // instance not found
	{
        // Skip empty, too short or comment lines
        if (linein[0] == '\n' || linein[0] == '#' || strlen(linein)<4) {
            line_num++;
            continue;
        }

		// split line into left and right and extract instance id, slots and interfaces
        if (     sscanf(linein, "%127s %127s", left, right) != 2
		     ||  sscanf(left,   "%d:%d/%d", &src.instance, &src.slot, &src.port) != 3
             ||  sscanf(right,  "%d:%d/%d", &dst.instance, &dst.slot, &dst.port) != 3)
        {
            printf("Error: skipping Line %d: too long or incorrect NETMAP format\n", line_num);
			continue;
		}
		if (verbosity>=DEBUG)
		{
			printf("NETMAP line %3d: (%s) %4d:%d/%d <=> (%s) %4d:%d/%d\n", 
				                line_num,
				                left,  src.instance, src.slot, src.port,
								right, dst.instance, dst.slot, dst.port);
		}

		// Search for our instance ID
		if ( src.instance==live_instance ) *iou=dst ;
	    if ( dst.instance==live_instance ) *iou=src ;

        line_num++;
    } // parse next line

    fclose(infp);
    if (iou->instance==0)	{
  	    printf("Error: netmap file %s parsed, but live instance %d not found\n\n",
			    netmap_path, live_instance);
		return -1;
	}
    if (verbosity>=VERBOSE) {
        printf("Peer found line %d: Peer Instance = %3d, Peer Interface = %3d, IOULIVE Instance = %3d\n",
	            line_num-1, iou->instance, iou->slot+iou->port*16, live_instance);
	}
	return 0;
} // parse_netmap




