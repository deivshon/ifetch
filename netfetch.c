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

void get_max_interface(char *dest) {
    long current_max = 0;

    char *ifs_path = "/sys/class/net";
    DIR *interfaces = opendir(ifs_path);

    struct dirent *entry;

    while(entry = readdir(interfaces)) {
        if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        char cur_if[MAX_PATH_LENGTH];
        char cur_opstate[MAX_PATH_LENGTH];
        sprintf(cur_if, "%s/%s", ifs_path, entry->d_name);
        sprintf(cur_opstate, "%s/%s", cur_if, "operstate");

        FILE *opstate_file = fopen(cur_opstate, "r");
        if(opstate_file == NULL) {
            printf("%s: no operstate\n");
            continue;
        }
        char opstate[8];
        fgets(opstate, 8, opstate_file);
        if(strcmp(opstate, "up\n")) // If interface is not up, continue
            continue;

        char rxb_path[MAX_PATH_LENGTH];
        char txb_path[MAX_PATH_LENGTH];
        sprintf(rxb_path, "%s/%s", cur_if, "statistics/rx_bytes");
        sprintf(txb_path, "%s/%s", cur_if, "statistics/tx_bytes");
        long cur_rx_bytes = long_from_file(rxb_path);
        long cur_tx_bytes = long_from_file(txb_path);

        if(cur_rx_bytes + cur_tx_bytes > current_max) {
            current_max = cur_rx_bytes + cur_tx_bytes;
            strcpy(dest, entry->d_name);
        }
    }
}

int main() {

}
