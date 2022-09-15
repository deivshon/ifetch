#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include "../hs/netutils.h"
#include "../hs/argutils.h"
#include "../hs/ifetch.h"

#define CONFIG_PATH_SUFFIX ".config/ifetch/ifetchrc"

#define MAX_PADDING 11

#define IF_INDEX    0
#define MAC_INDEX   1
#define RX_INDEX    2
#define TX_INDEX    3

#define FIELDS_NUM  4

#define starts_with(str, prefix) !strncmp(str, prefix, strlen(prefix))

#define output_data(data, label, logo, logo_substitute, row_index, sep, logo_color, fields_color, values_color, sep_color)  \
printf("%s%s%s%*s%s%s%s%s %s\n", logo_color, row_index < logo->rows_used ? logo->row[row_index] : logo_substitute,          \
fields_color, (int) (MAX_PADDING - strlen(label)), "", label, sep_color, sep, values_color, data);

#define output_data_padded(data, logo, logo_substitute, row_index, logo_color, fields_color, values_color, sep_color)   \
printf("%s%s%s%*s%*s%s%s %s\n", logo_color, row_index < logo->rows_used ? logo->row[row_index] : logo_substitute,       \
fields_color, (int) MAX_PADDING, "", (int) strlen(sep), "", sep_color, values_color, data);

#define print_ips(ips, num, label, logo, logo_substitute, row_index, sep, logo_color, fields_color, values_color, sep_color)\
    if(num > 0) {\
        output_data(ips[0], label, logo, logo_substitute, row_index, sep, logo_color, fields_color, values_color, sep_color);\
        row_index++;\
        for(unsigned int i = 1; i < num; i++) {\
            output_data_padded(ips[i], logo, logo_substitute, row_index, logo_color, fields_color, values_color, sep_color);\
            row_index++;\
        }\
    }\

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
        items[i].show = 1;
    }
}

void free_ips(char **ips, unsigned int num) {
    for(unsigned int i = 0; i < num; i++) free(ips[i]);
}

int main(int argc, char **argv) {
    char *home_dir = getpwuid(getuid())->pw_dir;
    char config_path[MAX_PATH_LENGTH];
    sprintf(config_path, "%s/%s", home_dir, CONFIG_PATH_SUFFIX);

    struct data_item data[FIELDS_NUM];
    init_data_items(data);

    strcpy(data[IF_INDEX].label, "INTERFACE");
    data[IF_INDEX].data[0] = '\0';

    strcpy(data[MAC_INDEX].label, "MAC");
    strcpy(data[RX_INDEX].label, "RX");
    strcpy(data[TX_INDEX].label, "TX");

    char *logo_color = BWHITE;
    char *fields_color = BCYAN;
    char *values_color = BWHITE;
    char *sep_color = BWHITE;

    struct logo *assigned_logo;
    char logo_substitute[64];

    char sep[9] = ":";

    double rx = -1, tx = -1;

    char *ip_addr_4[MAX_IPV4_NUM];
    unsigned int ipv4_num = 0;
    int show_ip4 = 1;

    char *ip_addr_6[MAX_IPV6_NUM];
    unsigned int ipv6_num = 0;
    int show_ip6 = 1;

    char **args;
    int args_num;

    if(args_from_file(&args, &args_num, config_path)) {
        handle_args(args, args_num, 1, data[IF_INDEX].data, &logo_color, &fields_color, \
                    &values_color, &sep_color, sep, &data[IF_INDEX].show,               \
                    &data[RX_INDEX].show, &data[TX_INDEX].show, &data[MAC_INDEX].show,  \
                    &show_ip4, &show_ip6);
        free_args(args, args_num);
    }

    args = argv;
    args_num = argc;

    handle_args(args, args_num, 0, data[IF_INDEX].data, &logo_color, &fields_color, \
                &values_color, &sep_color, sep, &data[IF_INDEX].show,               \
                &data[RX_INDEX].show, &data[TX_INDEX].show, &data[MAC_INDEX].show, &show_ip4,   \
                &show_ip6);

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

    data[IF_INDEX].exists = 1;
    data[RX_INDEX].exists = rx != -1;
    data[TX_INDEX].exists = tx != -1;

    if(data[RX_INDEX].show && data[RX_INDEX].exists) to_formatted_bytes(data[RX_INDEX].data, rx);
    if(data[TX_INDEX].show && data[TX_INDEX].exists) to_formatted_bytes(data[TX_INDEX].data, tx);
    if(data[MAC_INDEX].show) data[MAC_INDEX].exists = get_mac(data[MAC_INDEX].data, data[IF_INDEX].data);
    if(show_ip4) ipv4_num = get_ip(ip_addr_4, data[IF_INDEX].data, IPv4);
    if(show_ip6) ipv6_num = get_ip(ip_addr_6, data[IF_INDEX].data, IPv6);
    assign_logo(&assigned_logo, data[IF_INDEX].data);
    get_logo_space(logo_substitute, assigned_logo);

    unsigned int row_index = 0;

    for(unsigned int i = 0; i <= MAC_INDEX; i++) {
        if(data[i].show && data[i].exists) {
            output_data(data[i].data, data[i].label, assigned_logo, logo_substitute, row_index, sep, logo_color, fields_color, values_color, sep_color);
            row_index++;
        }
    }

    if(show_ip4) {
        print_ips(ip_addr_4, ipv4_num, "IPv4", assigned_logo, logo_substitute, row_index, sep, logo_color, fields_color, values_color, sep_color);
        free_ips(ip_addr_4, ipv4_num);
    }

    if(show_ip6) {
        print_ips(ip_addr_6, ipv6_num, "IPv6", assigned_logo, logo_substitute, row_index, sep, logo_color, fields_color, values_color, sep_color);
        free_ips(ip_addr_6, ipv6_num);
    }

    for(unsigned int i = RX_INDEX; i < FIELDS_NUM; i++) {
        if(data[i].show && data[i].exists) {
            output_data(data[i].data, data[i].label, assigned_logo, logo_substitute, row_index, sep, logo_color, fields_color, values_color, sep_color);
            row_index++;
        }
    }

    while(row_index < assigned_logo->rows_used) {
        printf("%s%s\n", logo_color, assigned_logo->row[row_index]);
        row_index++;

    }

    printf("%s\n", WHITE);
    exit(EXIT_SUCCESS);
}
