#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include "../hs/netutils.h"
#include "../hs/argutils.h"
#include "../hs/ifetch.h"

#define CONFIG_PATH_SUFFIX ".config/ifetch/ifetchrc"

void print_data(struct data_item *item, struct logo *assigned_logo, char *logo_substitute, unsigned int *row_index, char *sep, int max_padding) {
    char *buf = strtok(item->data, "\n");
    int i = 0;
    while(buf != NULL) {
        if(i == 0) {
            printf("%s%s%s%*s%s%s%s%s%s\n", item->logo_color, (*row_index) < assigned_logo->rows_used ? assigned_logo->row[(*row_index)] : logo_substitute, item->field_color, (int) (max_padding - strlen(item->label)), "", item->label, item->sep_color, sep, item->value_color, buf);
        }
        else {
            printf("%s%s%s%*s%*s%s%s%s\n", item->logo_color, (*row_index) < assigned_logo->rows_used ? assigned_logo->row[(*row_index)] : logo_substitute, item->field_color, (int) max_padding, "", (int) strlen(sep), "", item->sep_color, item->value_color, buf);
        }
        i++;
        (*row_index)++;
        buf = strtok(NULL, "\n");
    }
}

struct logo ethernet_logo = {{
    "+---------------+",
    "|   +-------+   |",
    "| +--       --+ |",
    "| |           | |",
    "| | | | | | | | |",
    "| | | | | | | | |",
    "| +-----------+ |",
    "+---------------+"
}, 8};

struct logo wifi_logo = {{
    "   ___________   ",
    "  /           \\  ",
    " /  _________  \\ ",
    "/  /         \\  \\",
    "  /  _______  \\  ",
    "    /       \\    ",
    "       ...       ",
    "       ...       "
}, 8};

struct logo default_logo = {{
    "            +---+",
    "            |   |",
    "        +---+   |",
    "        |   |   |",
    "    +---+   |   |",
    "    |   |   |   |",
    "+---+   |   |   |",
    "|___|___|___|___|"
}, 8};

void get_logo_space(char *dest, struct logo *assigned_logo) {
    sprintf(dest, "%*s", (int) strlen(assigned_logo->row[assigned_logo->rows_used - 1]), "");
}

void assign_logo(struct logo **dest, char *interface) {
    if(strlen(interface) < 3) {
        *dest = &default_logo;
        return;
    }
    if(starts_with(interface, "wlp") || starts_with(interface, "wlan")) {
        *dest = &wifi_logo;
        return;
    }
    if(starts_with(interface, "eth") || starts_with(interface, "enp")) {
        *dest = &ethernet_logo;
        return;
    }
    *dest = &default_logo;
    return;
}

void init_data_items(struct data_item items[]) {
    for(int i = 0; i < FIELDS_NUM; i++) {
        if(i != IP4_INDEX && i != IP6_INDEX)
            items[i].data = malloc(sizeof(char) * MAX_DATA_LENGTH);
        else
            items[i].data = NULL;
        items[i].show = 1;
        items[i].sep = " >> ";

        items[i].logo_color = BGREEN;
        items[i].field_color = BWHITE;
        items[i].sep_color = BGREEN;
        items[i].value_color = BWHITE;
    }

    strcpy(items[IF_INDEX].label, "INTERFACE");
    items[IF_INDEX].data[0] = '\0';

    strcpy(items[MAC_INDEX].label, "MAC");
    strcpy(items[RX_INDEX].label, "RX");
    strcpy(items[IP4_INDEX].label, "IPv4");
    strcpy(items[IP6_INDEX].label, "IPv6");
    strcpy(items[TX_INDEX].label, "TX");

    strcpy(items[IF_INDEX].arg_name, "-if");
    strcpy(items[MAC_INDEX].arg_name, "-mac");
    strcpy(items[IP4_INDEX].arg_name, "-ip4");
    strcpy(items[IP6_INDEX].arg_name, "-ip6");
    strcpy(items[RX_INDEX].arg_name, "-rx");
    strcpy(items[TX_INDEX].arg_name, "-tx");
}

void free_data_items(struct data_item items[]) {
    for(int i = 0; i < FIELDS_NUM; i++) free(items[i].data);
}

unsigned int get_max_padding(struct data_item items[]) {
    int max = 0;
    int cur_len;

    for(int i = 0; i < FIELDS_NUM; i++) {
        if(items[i].show && items[i].instances) {
            cur_len = strlen(items[i].label);
            if(cur_len > max) max = cur_len;
        }
    }
    return max;
}

int main(int argc, char **argv) {
    char *home_dir = getpwuid(getuid())->pw_dir;
    char config_path[MAX_PATH_LENGTH];
    sprintf(config_path, "%s/%s", home_dir, CONFIG_PATH_SUFFIX);

    struct data_item data[FIELDS_NUM];
    init_data_items(data);

    struct logo *assigned_logo;
    char logo_substitute[64];
    unsigned int max_padding; 
    unsigned int logo_fields_distance = 2;
    unsigned int min_padding = 0;

    char *remaining_logo_color = BGREEN;

    char sep[9] = " >> ";

    double rx = -1, tx = -1;

    char **args;
    int args_num;

    if(args_from_file(&args, &args_num, config_path)) {
        handle_args(args, args_num, 1, data[IF_INDEX].data, sep, data,  \
                    &logo_fields_distance, &min_padding);
        free_args(args, args_num);
    }

    args = argv;
    args_num = argc;

    handle_args(args, args_num, 0, data[IF_INDEX].data, sep, data,  \
                &logo_fields_distance, &min_padding);

    if(strlen(data[IF_INDEX].data) == 0) {
        int interface_available = get_max_interface(data[IF_INDEX].data, &rx, &tx);
        if(!interface_available) {
            printf("No interface available\n");
            exit(EXIT_FAILURE);
        }
    }
    else {
        get_bytes(&rx, data[IF_INDEX].data, RX);
        get_bytes(&tx, data[IF_INDEX].data, TX);
    }

    data[IF_INDEX].instances = 1;
    data[RX_INDEX].instances = rx != -1;
    data[TX_INDEX].instances = tx != -1;

    if(data[RX_INDEX].show && data[RX_INDEX].instances) to_formatted_bytes(data[RX_INDEX].data, rx);
    if(data[TX_INDEX].show && data[TX_INDEX].instances) to_formatted_bytes(data[TX_INDEX].data, tx);
    if(data[MAC_INDEX].show) data[MAC_INDEX].instances = get_mac(data[MAC_INDEX].data, data[IF_INDEX].data);
    if(data[IP4_INDEX].show) data[IP4_INDEX].instances = get_ip(&(data[IP4_INDEX].data), data[IF_INDEX].data, IPv4);
    if(data[IP6_INDEX].show) data[IP6_INDEX].instances = get_ip(&(data[IP6_INDEX].data), data[IF_INDEX].data, IPv6);
    assign_logo(&assigned_logo, data[IF_INDEX].data);
    get_logo_space(logo_substitute, assigned_logo);

    max_padding = get_max_padding(data) + logo_fields_distance;
    if(max_padding < min_padding) max_padding = min_padding;

    unsigned int row_index = 0;

    for(unsigned int i = 0; i < FIELDS_NUM; i++) {
        if(data[i].show && data[i].instances) {
            print_data(&(data[i]), assigned_logo, logo_substitute, &row_index, sep, max_padding);
            remaining_logo_color = data[i].logo_color;
        }
    }

    while(row_index < assigned_logo->rows_used) {
        printf("%s%s\n", remaining_logo_color, assigned_logo->row[row_index]);
        row_index++;

    }

    free_data_items(data);
    printf("%s\n", WHITE);
    exit(EXIT_SUCCESS);
}
