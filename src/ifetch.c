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

#define starts_with(str, prefix) !strncmp(str, prefix, strlen(prefix))

struct logo {
    char row[ROWS_NUM][64];
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
    "+---------------+",
    "|   +-------+   |",
    "| +--       --+ |",
    "| |           | |",
    "| | | | | | | | |",
    "| | | | | | | | |",
    "| +-----------+ |",
    "+---------------+"
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

struct logo default_logo = {
    "            +---+",
    "            |   |",
    "        +---+   |",
    "        |   |   |",
    "    +---+   |   |",
    "    |   |   |   |",
    "+---+   |   |   |",
    "|___|___|___|___|"
};

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
    char padding[10] = "  ";

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
                    &values_color, &sep_color, sep, padding, &ascii_strict);
        free_args(args, args_num);
    }

    args = argv;
    args_num = argc;

    handle_args(args, args_num, 0, interface, &logo_color, &fields_color, \
                &values_color, &sep_color, sep, padding, &ascii_strict);

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

    printf("%s%s%s  INTERFACE%s%s%s %s\n", logo_color, assigned_logo->row[row_index], fields_color, sep_color, sep, values_color, interface);
    row_index++;

    if(mac_present) {
        printf("%s%s%s        MAC%s%s%s %s\n", logo_color, assigned_logo->row[row_index], fields_color, sep_color, sep, values_color, mac);
        row_index++;
    }

    for(int i = 0; i < ipv4_num; i++) {
        if(i == 0)
            printf("%s%s%s       IPv4%s%s%s %s\n", logo_color, assigned_logo->row[row_index], fields_color, sep_color, sep, values_color, ip_addr_4[i]);
        else
            printf("%s%s%s           %s%s%s\n", logo_color, assigned_logo->row[row_index], fields_color, values_color, padding, ip_addr_4[i]);
        free(ip_addr_4[i]);
        row_index++;
    }

    for(int i = 0; i < ipv6_num; i++) {
        if(i == 0)
            printf("%s%s%s       IPv6%s%s%s %s\n", logo_color, assigned_logo->row[row_index], fields_color, sep_color, sep, values_color, ip_addr_6[i]);
        else
            printf("%s%s%s           %s%s%s\n", logo_color, assigned_logo->row[row_index], fields_color, values_color, padding, ip_addr_6[i]);
        free(ip_addr_6[i]);
        row_index++;
    }

    if(rx != -1) {
        printf("%s%s%s         RX%s%s%s %s\n", logo_color, assigned_logo->row[row_index], fields_color, sep_color, sep, values_color, rx_mu);
        row_index++;
    }
    if(tx != -1) {
        printf("%s%s%s         TX%s%s%s %s\n", logo_color, assigned_logo->row[row_index], fields_color, sep_color, sep, values_color, tx_mu);
        row_index++;
    }

    while(row_index < ROWS_NUM) {
        printf("%s%s\n", logo_color, assigned_logo->row[row_index]);
        row_index++;

    }

    printf("%s\n", WHITE);
}
