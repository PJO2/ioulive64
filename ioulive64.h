// -------------
// ioulive64 - 2025 - GPLv2
// -------------


// format of NETMAP file
struct S_netmap_unit
{
	int instance;
	int slot;
	int port;
};

// send params to threads
struct S_ioulive_args {
    int raw_fd;
    int raw_dev_id;
    int ioulive_inst;
    struct S_netmap_unit peer;
} ;

// verbosity
extern int verbosity;
enum { QUIET=0, VERBOSE, VERY_VERBOSE, DEBUG };

void print_usage(void);

// in utils.c
void tsprintf(const char *format, ...);

// in parse.c
int parse_args(int argc, char *argv[], char *intf, char *netmap_path, unsigned int *instance);
int parse_netmap(const char *netmap_path, unsigned int live_instance, struct S_netmap_unit *iou);

// in iou-raw.c
int create_raw_socket(const char*name, int *dev_id);
ssize_t raw_send(int sock, int dev_id, char *buffer, size_t len);
ssize_t raw_recv(int sock, char *buffer, size_t len);

// in ioulive.c
int create_listening_skt(int instance);
int connect_peer_skt (int peer_instance);
void *iou_to_ethernet(void *arg);
void *ethernet_to_iou(void *arg);

