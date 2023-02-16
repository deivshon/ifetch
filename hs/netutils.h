#ifndef NETUTILS
#define NETUTILS

#define INTERFACE_SIZE 32
#define MAX_IPVX_NUM 8
#define MAX_IP_LENGTH 128

#define MAX_PATH_LENGTH 4096
#define MAX_FILENAME_LENGTH 256
#define MAX_INTERFACE_LENGTH 64

#define INTERFACES_PATH "/sys/class/net"

enum transmission_type {TX, RX};
enum ipv {IPv4, IPv6};

double get_bytes(double *dest, char *interface, enum transmission_type t);

int to_formatted_bytes(char *dest, double bytes);

int get_mac(char *dest, char *interface);

int get_ip(char **dest, char *interface_name, enum ipv ip_version);

int get_max_interface(char *dest, double *dest_rx_bytes, double *dest_tx_bytes);

int interface_exists(char *interface);

#endif
