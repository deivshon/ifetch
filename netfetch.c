#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <netdb.h>
#include <ifaddrs.h>

#define INTERFACES_PATH "/sys/class/net"

#define MAX_PATH_LENGTH 4096
#define MAX_FILENAME_LENGTH 256

enum transmission_type {TX, RX};
enum ipv {IPv4, IPv6};

#define BRED    "\033[1m\033[31m"
#define BGREEN  "\033[1m\033[32m"
#define BYELLOW "\033[1m\033[33m"
#define BBLUE   "\033[1m\033[34m"
#define BMAG    "\033[1m\033[35m"
#define BCYAN   "\033[1m\033[36m"
#define BWHITE  "\033[1m\033[37m"
#define NORMAL  "\033[0m\033[37m"

struct logo {
    char row0[64];
    char row1[64];
    char row2[64];
    char row3[64];
    char row4[64];
    char row5[64];
    char row6[64];
    char row7[64];
};

struct logo ethernet_logo = {
    "┌───────────────┐",
    "│   ┌───────┐   │",
    "│ ┌─┘       └─┐ │",
    "│ │           │ │",
    "│ │ │ │ │ │ │ │ │",
    "│ │ │ │ │ │ │ │ │",
    "│ └─┴─┴─┴─┴─┴─┘ │",
    "└───────────────┘"
};

struct logo ethernet_logo_alt = {
    "|---------------|",
    "|   |-------|   |",
    "| |--       --| |",
    "| |           | |",
    "| | | | | | | | |",
    "| | | | | | | | |",
    "| ------------- |",
    "-----------------"
};

struct logo wifi_logo = {
    "   ___________   ",
    "  /           \\  ",
    " /  _________  \\ ",
    "/  /         \\  \\",
    "  /  _______  \\  ",
    "    /       \\    ",
    "       ...       ",
    "       ...       "
};

void line_from_file(char *dest, char *path) {
    FILE *fs = fopen(path, "r");

    char line[1024];
    fgets(line, 1024, fs);

    fclose(fs);

    line[strcspn(line, "\n")] = '\0';

    strcpy(dest, line);
}

double double_from_file(char *path) {
    FILE *fs = fopen(path, "r");

    char number_str[20];
    fgets(number_str, 20, fs);

    fclose(fs);

    return strtod(number_str, NULL);
}

void get_mac(char *dest, char *interface) {
    char mac_file_path[MAX_PATH_LENGTH];
    sprintf(mac_file_path, "%s/%s/%s", INTERFACES_PATH, interface, "address");

    line_from_file(dest, mac_file_path);
}

double get_bytes(char *interface, enum transmission_type t) {
    char file_path[MAX_PATH_LENGTH];
    if(t == RX)
        sprintf(file_path, "%s/%s/%s", INTERFACES_PATH, interface, "/statistics/rx_bytes");
    else if(t == TX)
        sprintf(file_path, "%s/%s/%s", INTERFACES_PATH, interface, "/statistics/tx_bytes");

    return double_from_file(file_path);
}

int get_max_interface(char *dest, double *dest_rx_bytes, double *dest_tx_bytes) {
    int res = 0;
    double current_max = -1;

    DIR *interfaces = opendir(INTERFACES_PATH);
    struct dirent *entry;

    char cur_if_path[MAX_PATH_LENGTH];
    char cur_opstate_path[MAX_PATH_LENGTH];
    char cur_opstate[8];

    double cur_rx_bytes;
    double cur_tx_bytes;
    while(entry = readdir(interfaces)) {
        if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        sprintf(cur_opstate_path, "%s/%s/%s", INTERFACES_PATH, entry->d_name, "operstate");

        FILE *opstate_file = fopen(cur_opstate_path, "r");
        if(opstate_file == NULL) { // If the operstate file is missing, continue
            continue;
        }
        fgets(cur_opstate, 8, opstate_file);
        fclose(opstate_file);

        if(strcmp(cur_opstate, "up\n")) // If interface is not up, continue
            continue;

        cur_rx_bytes = get_bytes(entry->d_name, RX);
        cur_tx_bytes = get_bytes(entry->d_name, TX);

        if(cur_rx_bytes + cur_tx_bytes > current_max) {
            current_max = cur_rx_bytes + cur_tx_bytes;
            strcpy(dest, entry->d_name);
            *dest_rx_bytes = cur_rx_bytes;
            *dest_tx_bytes = cur_tx_bytes;
            res = 1;
        }
    }
    closedir(interfaces);

    return res;
}

int to_formatted_bytes(char *dest, double bytes) {
    char *suffixes[7] = {"B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB"};

    double approx_bytes = bytes;
    int divisions = 0;

    while(approx_bytes > 1e3 && divisions <= 7) {
        approx_bytes /= 1024;
        divisions++;
    }

    sprintf(dest, "%3.3lf %s", approx_bytes, suffixes[divisions]);
    return 1;
}

int get_ip(char *dest, unsigned int dest_size, char *interface_name, enum ipv ip_version) {
    struct ifaddrs *interface;
    if(getifaddrs(&interface) == -1) return 0;

    size_t info_size = ip_version == IPv4 ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
    unsigned int family = ip_version == IPv4 ? AF_INET : AF_INET6;

    int interface_found = 0;
    while(interface != NULL) {
        if (interface->ifa_addr == NULL) {
            interface = interface->ifa_next;
            continue; 
        }

        int gni_res = !getnameinfo(interface->ifa_addr, info_size, dest, dest_size, NULL, 0, NI_NUMERICHOST);

        if(gni_res && strcmp(interface->ifa_name, interface_name) == 0 && interface->ifa_addr->sa_family == family) {
            interface_found = 1;
            break;
        }

        interface = interface->ifa_next;
    }

    if(!interface) dest[0] = '\0';

    return interface_found;
}

int main() {
    char interface[32];
    char rx_mu[16], tx_mu[16];
    double rx = -1, tx = -1;
    char mac[18];
    char ip_addr_4[1024];
    char ip_addr_6[1024];

    struct logo *assigned_logo = &wifi_logo;

    int interface_available = get_max_interface(interface, &rx, &tx);
    if(!interface_available) return 1;

    to_formatted_bytes(rx_mu, rx);
    to_formatted_bytes(tx_mu, tx);
    get_mac(mac, interface);
    get_ip(ip_addr_4, 1024, interface, IPv4);
    get_ip(ip_addr_6, 1024, interface, IPv6);

    printf("%s%s  %sINTERFACE%s: %s\n", BWHITE, assigned_logo->row0, BCYAN, BWHITE, interface);
    printf("%s%s  %s      MAC%s: %s\n", BWHITE, assigned_logo->row1, BCYAN, BWHITE, mac);
    printf("%s%s  %s     IPv4%s: %s\n", BWHITE, assigned_logo->row2, BCYAN, BWHITE, ip_addr_4);
    printf("%s%s  %s     IPv6%s: %s\n", BWHITE, assigned_logo->row3, BCYAN, BWHITE, ip_addr_6);
    printf("%s%s  %s       RX%s: %s\n", BWHITE, assigned_logo->row4, BCYAN, BWHITE, rx_mu);
    printf("%s%s  %s       TX%s: %s\n", BWHITE, assigned_logo->row5, BCYAN, BWHITE, tx_mu);
    printf("%s%s\n", BWHITE, assigned_logo->row6);
    printf("%s%s\n", BWHITE, assigned_logo->row7);
    printf("%s\n", NORMAL);
}
