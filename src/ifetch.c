#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include "../hs/netutils.h"
#include "../hs/argutils.h"
#include "../hs/ifetch.h"

#define CONFIG_PATH_SUFFIX ".config/ifetch/ifetchrc"

#define output_data(data, label, logo, logo_substitute, row_index, sep, logo_color, fields_color, values_color, sep_color, max_padding)  \
printf("%s%s%s%*s%s%s%s%s %s\n", logo_color, row_index < logo->rows_used ? logo->row[row_index] : logo_substitute,          \
fields_color, (int) (max_padding - strlen(label)), "", label, sep_color, sep, values_color, data);

#define output_data_padded(data, logo, logo_substitute, row_index, logo_color, fields_color, values_color, sep_color, max_padding)   \
printf("%s%s%s%*s%*s%s%s %s\n", logo_color, row_index < logo->rows_used ? logo->row[row_index] : logo_substitute,       \
fields_color, (int) max_padding, "", (int) strlen(sep), "", sep_color, values_color, data);

#define print_ips(ips, num, label, logo, logo_substitute, row_index, sep, logo_color, fields_color, values_color, sep_color, max_padding)\
    if(num > 0) {\
        output_data(ips[0], label, logo, logo_substitute, row_index, sep, logo_color, fields_color, values_color, sep_color, max_padding);\
        row_index++;\
        for(unsigned int i = 1; i < num; i++) {\
            output_data_padded(ips[i], logo, logo_substitute, row_index, logo_color, fields_color, values_color, sep_color, max_padding);\
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

    strcpy(items[IF_INDEX].label, "INTERFACE");
    items[IF_INDEX].data[0] = '\0';

    strcpy(items[MAC_INDEX].label, "MAC");
    strcpy(items[RX_INDEX].label, "RX");
    strcpy(items[TX_INDEX].label, "TX");

    strcpy(items[IF_INDEX].arg_name, "-if");
    strcpy(items[MAC_INDEX].arg_name, "-mac");
    strcpy(items[RX_INDEX].arg_name, "-rx");
    strcpy(items[TX_INDEX].arg_name, "-tx");
}

void init_ip_item(struct ip_item *ip) {
    ip->show = 1;
    ip->ips_num = 0;
}

void free_ips(char **ips, unsigned int num) {
    for(unsigned int i = 0; i < num; i++) free(ips[i]);
}

unsigned int get_max_padding(struct data_item items[], char *ip4_label, \
                    int ip4_num, int show_ip4, char *ip6_label,         \
                    int ip6_num, int show_ip6) {
    int max = 0;
    int cur_len;
    cur_len = strlen(ip4_label);
    if(cur_len > max && show_ip4 && ip4_num > 0) max = cur_len;
    cur_len = strlen(ip6_label);
    if(cur_len > max && show_ip6 && ip6_num > 0) max = cur_len;

    for(int i = 0; i < FIELDS_NUM; i++) {
        if(items[i].show && items[i].exists) {
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

    char *logo_color = BWHITE;
    char *fields_color = BCYAN;
    char *values_color = BWHITE;
    char *sep_color = BWHITE;

    struct logo *assigned_logo;
    char logo_substitute[64];
    unsigned int max_padding; 
    unsigned int logo_fields_distance = 2;
    unsigned int min_padding = 0;

    char sep[9] = ":";

    double rx = -1, tx = -1;
    
    struct ip_item ip4;
    strcpy(ip4.label, "IPv4");

    struct ip_item ip6;
    strcpy(ip6.label, "IPv6");

    init_ip_item(&ip4);
    init_ip_item(&ip6);

    char **args;
    int args_num;

    if(args_from_file(&args, &args_num, config_path)) {
        handle_args(args, args_num, 1, data[IF_INDEX].data, &logo_color, &fields_color, \
                    &values_color, &sep_color, sep, &ip4, &ip6, data,         \
                    &logo_fields_distance, &min_padding);
        free_args(args, args_num);
    }

    args = argv;
    args_num = argc;

    handle_args(args, args_num, 0, data[IF_INDEX].data, &logo_color, &fields_color, \
                &values_color, &sep_color, sep, &ip4, &ip6, data,         \
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

    data[IF_INDEX].exists = 1;
    data[RX_INDEX].exists = rx != -1;
    data[TX_INDEX].exists = tx != -1;

    if(data[RX_INDEX].show && data[RX_INDEX].exists) to_formatted_bytes(data[RX_INDEX].data, rx);
    if(data[TX_INDEX].show && data[TX_INDEX].exists) to_formatted_bytes(data[TX_INDEX].data, tx);
    if(data[MAC_INDEX].show) data[MAC_INDEX].exists = get_mac(data[MAC_INDEX].data, data[IF_INDEX].data);
    if(ip4.show) ip4.ips_num = get_ip(ip4.data, data[IF_INDEX].data, IPv4);
    if(ip6.show) ip6.ips_num = get_ip(ip6.data, data[IF_INDEX].data, IPv6);
    assign_logo(&assigned_logo, data[IF_INDEX].data);
    get_logo_space(logo_substitute, assigned_logo);

    max_padding = get_max_padding(data, ip4.label, ip4.ips_num, ip4.show, ip6.label, ip6.ips_num, ip6.show) + logo_fields_distance;
    if(max_padding < min_padding) max_padding = min_padding;

    unsigned int row_index = 0;

    for(unsigned int i = 0; i <= MAC_INDEX; i++) {
        if(data[i].show && data[i].exists) {
            output_data(data[i].data, data[i].label, assigned_logo, logo_substitute, row_index, sep, logo_color, fields_color, values_color, sep_color, max_padding);
            row_index++;
        }
    }

    if(ip4.show) {
        print_ips(ip4.data, ip4.ips_num, ip4.label, assigned_logo, logo_substitute, row_index, sep, logo_color, fields_color, values_color, sep_color, max_padding);
        free_ips(ip4.data, ip4.ips_num);
    }

    if(ip6.show) {
        print_ips(ip6.data, ip6.ips_num, ip6.label, assigned_logo, logo_substitute, row_index, sep, logo_color, fields_color, values_color, sep_color, max_padding);
        free_ips(ip6.data, ip6.ips_num);
    }

    for(unsigned int i = RX_INDEX; i < FIELDS_NUM; i++) {
        if(data[i].show && data[i].exists) {
            output_data(data[i].data, data[i].label, assigned_logo, logo_substitute, row_index, sep, logo_color, fields_color, values_color, sep_color, max_padding);
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
