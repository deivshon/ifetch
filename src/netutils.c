#include "../hs/netutils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <netdb.h>
#include <ifaddrs.h>

static void line_from_file(char *dest, char *path) {
    FILE *fs = fopen(path, "r");

    char line[1024];
    fgets(line, 1024, fs);

    fclose(fs);

    line[strcspn(line, "\n")] = '\0';

    strcpy(dest, line);
}

static double double_from_file(char *path) {
    FILE *fs = fopen(path, "r");

    char number_str[20];
    fgets(number_str, 20, fs);

    fclose(fs);

    return strtod(number_str, NULL);
}

double get_bytes(char *interface, enum transmission_type t) {
    char file_path[MAX_PATH_LENGTH];
    if(t == RX)
        sprintf(file_path, "%s/%s/%s", INTERFACES_PATH, interface, "/statistics/rx_bytes");
    else if(t == TX)
        sprintf(file_path, "%s/%s/%s", INTERFACES_PATH, interface, "/statistics/tx_bytes");

    return double_from_file(file_path);
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

void get_mac(char *dest, char *interface) {
    char mac_file_path[MAX_PATH_LENGTH];
    sprintf(mac_file_path, "%s/%s/%s", INTERFACES_PATH, interface, "address");

    line_from_file(dest, mac_file_path);
}

int get_ip(char **dest, unsigned int dest_size, char *interface_name, enum ipv ip_version) {
    struct ifaddrs *interface, *interface_head;
    if(getifaddrs(&interface) == -1) return 0;
    interface_head = interface;

    char current_ip[1024];
    int c = 0;

    size_t info_size = ip_version == IPv4 ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
    unsigned int family = ip_version == IPv4 ? AF_INET : AF_INET6;
    unsigned int max_ips = ip_version == IPv4 ? MAX_IPV4_NUM : MAX_IPV6_NUM;

    while(interface != NULL && c < max_ips) {
        if (interface->ifa_addr == NULL) {
            interface = interface->ifa_next;
            continue; 
        }

        int gni_res = !getnameinfo(interface->ifa_addr, info_size, current_ip, 1024, NULL, 0, NI_NUMERICHOST);

        if(gni_res && strcmp(interface->ifa_name, interface_name) == 0 && interface->ifa_addr->sa_family == family) {
            dest[c] = malloc(sizeof(char) * 1024);
            strcpy(dest[c], current_ip);
            c++;
        }

        interface = interface->ifa_next;
    }

    freeifaddrs(interface_head);
    return c;
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

int interface_exists(char *interface) {
    int result;

    char ifdir_path[MAX_PATH_LENGTH];
    sprintf(ifdir_path, "%s/%s", INTERFACES_PATH, interface);

    DIR *ifdir = opendir(ifdir_path);
    result = ifdir != NULL;
    closedir(ifdir);

    return result;
}