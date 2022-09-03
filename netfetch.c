#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#define MAX_PATH_LENGTH 4096
#define MAX_FILENAME_LENGTH 256

long long_from_file(char *path) {
    FILE *fs = fopen(path, "r");

    char number_str[20];
    fgets(number_str, 20, fs);

    fclose(fs);

    return atol(number_str);
}

int get_max_interface(char *dest, long *dest_rx_bytes, long *dest_tx_bytes) {
    int res = 0;
    long current_max = -1;

    char *ifs_path = "/sys/class/net";
    DIR *interfaces = opendir(ifs_path);
    struct dirent *entry;

    char cur_if_path[MAX_PATH_LENGTH];
    char cur_opstate_path[MAX_PATH_LENGTH];
    char cur_opstate[8];

    char cur_rxb_path[MAX_PATH_LENGTH];
    char cur_txb_path[MAX_PATH_LENGTH];

    long cur_rx_bytes;
    long cur_tx_bytes;
    while(entry = readdir(interfaces)) {
        if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        sprintf(cur_if_path, "%s/%s", ifs_path, entry->d_name);
        sprintf(cur_opstate_path, "%s/%s", cur_if_path, "operstate");

        FILE *opstate_file = fopen(cur_opstate_path, "r");
        if(opstate_file == NULL) {
            printf("%s: no operstate\n");
            continue;
        }
        fgets(cur_opstate, 8, opstate_file);
        if(strcmp(cur_opstate, "up\n")) // If interface is not up, continue
            continue;

        sprintf(cur_rxb_path, "%s/%s", cur_if_path, "statistics/rx_bytes");
        sprintf(cur_txb_path, "%s/%s", cur_if_path, "statistics/tx_bytes");
        cur_rx_bytes = long_from_file(cur_rxb_path);
        cur_tx_bytes = long_from_file(cur_txb_path);

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

int main() {
    char interface[32];
    long rx = -1, tx = -1;

    get_max_interface(interface, &rx, &tx);
    printf("%s\nrx: %ld\ntx: %ld\n", interface, rx, tx);
}
