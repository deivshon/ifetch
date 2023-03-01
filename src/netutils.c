#include "../hs/netutils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <netdb.h>
#include <ifaddrs.h>

static int line_from_file(char *dest, char *path) {
    FILE *fs = fopen(path, "r");
    if(fs == NULL) return 0;

    char line[1024];
    if(!fgets(line, 1024, fs)) return 0;

    fclose(fs);

    line[strcspn(line, "\n")] = '\0';

    strcpy(dest, line);
    return 1;
}

static double double_from_file(double *dest, char *path) {
    FILE *fs = fopen(path, "r");
    if(fs == NULL) return 0;

    char number_str[32];
    number_str[0] = '\0';

    if(!fgets(number_str, 32, fs)) return 0;
    if(!strcmp(number_str, "\n") || !strcmp(number_str, "")) { 
        // If the file is empty, the function failed
        fclose(fs);
        return 0;
    }
    fclose(fs);

    (*dest) = strtod(number_str, NULL);
    return 1;
}

double get_bytes(double *dest, char *interface, enum transmission_type t) {
    char file_path[MAX_PATH_LENGTH];
    if(t == RX)
        sprintf(file_path, "%s/%s/%s", INTERFACES_PATH, interface, "/statistics/rx_bytes");
    else if(t == TX)
        sprintf(file_path, "%s/%s/%s", INTERFACES_PATH, interface, "/statistics/tx_bytes");

    if(!double_from_file(dest, file_path)) return 0;
    return 1;
}

int to_formatted_bytes(char *dest, double bytes) {
    char *suffixes[7] = {"B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB"};

    double approx_bytes = bytes;
    unsigned int divisions = 0;

    while(approx_bytes > 1e3 && divisions <= 7) {
        approx_bytes /= 1024;
        divisions++;
    }

    if(divisions == 0)
        sprintf(dest, "%.0lf %s", approx_bytes, suffixes[divisions]);
    else
        sprintf(dest, "%.2lf %s", approx_bytes, suffixes[divisions]);

    return 1;
}

int get_mac(char *dest, char *interface) {
    char mac_file_path[MAX_PATH_LENGTH];
    sprintf(mac_file_path, "%s/%s/%s", INTERFACES_PATH, interface, "address");

    if(!line_from_file(dest, mac_file_path)) return 0;

    return strcmp("", dest);
}

int get_ip(char **dest, char *interface_name, enum ipv ip_version) {
    struct ifaddrs *interface, *interface_head;
    if(getifaddrs(&interface) == -1) return 0;
    interface_head = interface;

    char current_ip[MAX_IP_LENGTH];
    unsigned int c = 0;

    size_t info_size = ip_version == IPv4 ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
    unsigned int family = ip_version == IPv4 ? AF_INET : AF_INET6;

    while(interface != NULL && c < MAX_IPVX_NUM) {
        if (interface->ifa_addr == NULL) {
            interface = interface->ifa_next;
            continue; 
        }

        int gni_res = !getnameinfo(interface->ifa_addr, info_size, current_ip, MAX_IP_LENGTH, NULL, 0, NI_NUMERICHOST);

        if(gni_res && strcmp(interface->ifa_name, interface_name) == 0 && interface->ifa_addr->sa_family == family) {
            if((*dest) == NULL) {
                (*dest) = malloc(sizeof(char) * strlen(current_ip) + 1);
                strcpy((*dest), current_ip);
            }
            else {
                // Memory for: previous content + current IP string + memory for newline
                (*dest) = realloc((*dest), sizeof(char) * strlen(*dest)
                       + sizeof(char) * (strlen(current_ip) + 1)
                       + 1);
                strcat((*dest), "\n");
                strcat((*dest), current_ip);
            }
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

    char cur_opstate_path[MAX_PATH_LENGTH];
    char cur_opstate[8];

    double cur_rx_bytes;
    double cur_tx_bytes;
    while((entry = readdir(interfaces)) != NULL) {
        if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        sprintf(cur_opstate_path, "%s/%s/%s", INTERFACES_PATH, entry->d_name, "operstate");

        FILE *opstate_file = fopen(cur_opstate_path, "r");
        if(opstate_file == NULL) { // If the operstate file is missing, continue
            continue;
        }
        if(!fgets(cur_opstate, 8, opstate_file)) continue;
        fclose(opstate_file);

        if(strcmp(cur_opstate, "up\n")) // If interface is not up, continue
            continue;

        if(!get_bytes(&cur_rx_bytes, entry->d_name, RX) || \
           !get_bytes(&cur_tx_bytes, entry->d_name, TX)) continue;

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
