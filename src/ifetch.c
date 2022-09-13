#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include "../hs/netutils.h"
#include "../hs/argutils.h"

#define CONFIG_PATH_SUFFIX ".config/ifetch/ifetchrc"

#define ROWS_NUM 8
#define MAX_PADDING 11

#define starts_with(str, prefix) !strncmp(str, prefix, strlen(prefix))

#define output_data(data, label, logo, row_index, sep, logo_color, fields_color, values_color, sep_color)   \
printf("%s%s%s%*s%s%s%s%s %s\n", logo_color, logo->row[row_index], fields_color, MAX_PADDING - strlen(label), "", label, sep_color, sep, values_color, data);

#define output_data_padded(data, logo, row_index, logo_color, fields_color, values_color, sep_color)   \
printf("%s%s%s%*s%*s%s%s %s\n", logo_color, logo->row[row_index], fields_color, MAX_PADDING, "", strlen(sep), "", sep_color, values_color, data);

struct logo {
    char row[ROWS_NUM][64];
};

struct logo ethernet_logo = {{
    "┌───────────────┐",
    "│   ┌───────┐   │",
    "│ ┌─┘       └─┐ │",
    "│ │           │ │",
    "│ │ │ │ │ │ │ │ │",
    "│ │ │ │ │ │ │ │ │",
    "│ └─┴─┴─┴─┴─┴─┘ │",
    "└───────────────┘"
}};

struct logo ethernet_logo_alt = {{
    "+---------------+",
    "|   +-------+   |",
    "| +--       --+ |",
    "| |           | |",
    "| | | | | | | | |",
    "| | | | | | | | |",
    "| +-----------+ |",
    "+---------------+"
}};

struct logo wifi_logo = {{
    "   ___________   ",
    "  /           \\  ",
    " /  _________  \\ ",
    "/  /         \\  \\",
    "  /  _______  \\  ",
    "    /       \\    ",
    "       ...       ",
    "       ...       "
}};

struct logo default_logo = {{
    "            +---+",
    "            |   |",
    "        +---+   |",
    "        |   |   |",
    "    +---+   |   |",
    "    |   |   |   |",
    "+---+   |   |   |",
    "|___|___|___|___|"
}};

void assign_logo(struct logo **dest, char *interface, int ascii_strict) {
    if(strlen(interface) < 3) {
        *dest = &default_logo;
        return;
    }
    if(starts_with(interface, "wlp") || starts_with(interface, "wlan")) {
        *dest = &wifi_logo;
        return;
    }
    if(starts_with(interface, "eth") || starts_with(interface, "enp")) {
        *dest = ascii_strict ? &ethernet_logo_alt : &ethernet_logo;
        return;
    }
    *dest = &default_logo;
    return;
}

int main(int argc, char **argv) {
    char *home_dir = getpwuid(getuid())->pw_dir;
    char config_path[MAX_PATH_LENGTH];
    sprintf(config_path, "%s/%s", home_dir, CONFIG_PATH_SUFFIX);

    char *logo_color = BWHITE;
    char *fields_color = BCYAN;
    char *values_color = BWHITE;
    char *sep_color = BWHITE;

    struct logo *assigned_logo;
    int ascii_strict = 0;

    char sep[9] = ":";

    char interface[MAX_INTERFACE_LENGTH];
    interface[0] = '\0';

    double rx = -1, tx = -1;
    char rx_mu[16], tx_mu[16];

    char mac[18];
    int mac_present;

    char *ip_addr_4[MAX_IPV4_NUM];
    int ipv4_num = 0;

    char *ip_addr_6[MAX_IPV6_NUM];
    int ipv6_num = 0;    

    char **args;
    int args_num;

    if(args_from_file(&args, &args_num, config_path)) {
        handle_args(args, args_num, 1, interface, &logo_color, &fields_color, \
                    &values_color, &sep_color, sep, &ascii_strict);
        free_args(args, args_num);
    }

    args = argv;
    args_num = argc;

    handle_args(args, args_num, 0, interface, &logo_color, &fields_color, \
                &values_color, &sep_color, sep, &ascii_strict);

    if(strlen(interface) == 0) {
        int interface_available = get_max_interface(interface, &rx, &tx);
        if(!interface_available) {
            printf("No interface available\n");
            exit(EXIT_FAILURE);
        }
    }
    else {
        get_bytes(&rx, interface, RX);
        get_bytes(&tx, interface, TX);
    }

    if(rx != -1) to_formatted_bytes(rx_mu, rx);
    if(tx != -1) to_formatted_bytes(tx_mu, tx);
    mac_present = get_mac(mac, interface);
    ipv4_num = get_ip(ip_addr_4, 1024, interface, IPv4);
    ipv6_num = get_ip(ip_addr_6, 1024, interface, IPv6);
    assign_logo(&assigned_logo, interface, ascii_strict);

    int row_index = 0;

    output_data(interface, "INTERFACE", assigned_logo, row_index, sep, logo_color, fields_color, values_color, sep_color);
    row_index++;

    if(mac_present) {
        output_data(mac, "MAC", assigned_logo, row_index, sep, logo_color, fields_color, values_color, sep_color);
        row_index++;
    }

    if(ipv4_num > 0) {
        output_data(ip_addr_4[0], "IPv4", assigned_logo, row_index, sep, logo_color, fields_color, values_color, sep_color);
        free(ip_addr_4[0]);
        row_index++;
        for(int i = 1; i < ipv4_num; i++) {
            output_data_padded(ip_addr_4[i], assigned_logo, row_index, logo_color, fields_color, values_color, sep_color);
            row_index++;
            free(ip_addr_4[i]);
        }
    }

    if(ipv6_num > 0) {
        output_data(ip_addr_6[0], "IPv6", assigned_logo, row_index, sep, logo_color, fields_color, values_color, sep_color);
        row_index++;
        free(ip_addr_6[0]);
        for(int i = 1; i < ipv6_num; i++) {
            output_data_padded(ip_addr_6[i], assigned_logo, row_index, logo_color, fields_color, values_color, sep_color);
            row_index++;
            free(ip_addr_6[i]);
        }
    }

    if(rx != -1) {
        output_data(rx_mu, "RX", assigned_logo, row_index, sep, logo_color, fields_color, values_color, sep_color);
        row_index++;
    }
    if(tx != -1) {
        output_data(tx_mu, "TX", assigned_logo, row_index, sep, logo_color, fields_color, values_color, sep_color);
        row_index++;
    }

    while(row_index < ROWS_NUM) {
        printf("%s%s\n", logo_color, assigned_logo->row[row_index]);
        row_index++;

    }

    printf("%s\n", WHITE);
}
