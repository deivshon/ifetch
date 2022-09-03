#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#define INTERFACES_PATH "/sys/class/net"

#define MAX_PATH_LENGTH 4096
#define MAX_FILENAME_LENGTH 256

char *ethernet_logo = "\
┌───────────────┐\n\
│   ┌───────┐   │\n\
│ ┌─┘       └─┐ │\n\
│ │           │ │\n\
│ │ │ │ │ │ │ │ │\n\
│ │ │ │ │ │ │ │ │\n\
│ └─┴─┴─┴─┴─┴─┘ │\n\
└───────────────┘\n";

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

int get_max_interface(char *dest, double *dest_rx_bytes, double *dest_tx_bytes) {
    int res = 0;
    double current_max = -1;

    DIR *interfaces = opendir(INTERFACES_PATH);
    struct dirent *entry;

    char cur_if_path[MAX_PATH_LENGTH];
    char cur_opstate_path[MAX_PATH_LENGTH];
    char cur_opstate[8];

    char cur_rxb_path[MAX_PATH_LENGTH];
    char cur_txb_path[MAX_PATH_LENGTH];

    double cur_rx_bytes;
    double cur_tx_bytes;
    while(entry = readdir(interfaces)) {
        if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        sprintf(cur_if_path, "%s/%s", INTERFACES_PATH, entry->d_name);
        sprintf(cur_opstate_path, "%s/%s", cur_if_path, "operstate");

        FILE *opstate_file = fopen(cur_opstate_path, "r");
        if(opstate_file == NULL) { // If the operstate file is missing, continue
            continue;
        }
        fgets(cur_opstate, 8, opstate_file);
        if(strcmp(cur_opstate, "up\n")) // If interface is not up, continue
            continue;

        sprintf(cur_rxb_path, "%s/%s", cur_if_path, "statistics/rx_bytes");
        sprintf(cur_txb_path, "%s/%s", cur_if_path, "statistics/tx_bytes");
        cur_rx_bytes = double_from_file(cur_rxb_path);
        cur_tx_bytes = double_from_file(cur_txb_path);

        if(cur_rx_bytes + cur_tx_bytes > current_max) {
            current_max = cur_rx_bytes + cur_tx_bytes;
            strcpy(dest, entry->d_name);
            *dest_rx_bytes = cur_rx_bytes;
            *dest_tx_bytes = cur_tx_bytes;
            res = 1;
        }
    }

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

int main() {
    char interface[32];
    char rx_mu[16], tx_mu[16];
    double rx = -1, tx = -1;
    char mac[18];

    int interface_available = get_max_interface(interface, &rx, &tx);
    if(!interface_available) return 1;
    to_formatted_bytes(rx_mu, rx);
    to_formatted_bytes(tx_mu, tx);
    get_mac(mac, interface);
    printf("\033[1m\033[36mINTERFACE\033[1m\033[37m: %s\n", interface);
    printf("\033[1m\033[36m       RX\033[1m\033[37m: %s\n", rx_mu);
    printf("\033[1m\033[36m       TX\033[1m\033[37m: %s\n", tx_mu);
    printf("\033[1m\033[36m      MAC\033[1m\033[37m: %s\n", mac);
    printf("\033[0m\033[37m");
}
