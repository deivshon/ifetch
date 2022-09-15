#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include "../hs/netutils.h"
#include "../hs/argutils.h"

#define CONFIG_PATH_SUFFIX ".config/ifetch/ifetchrc"

#define LOGO_ROWS_NUM 8
#define LOGO_LINE_LENGHT 64
#define MAX_PADDING 11

#define starts_with(str, prefix) !strncmp(str, prefix, strlen(prefix))

#define output_data(data, label, logo, logo_substitute, row_index, sep, logo_color, fields_color, values_color, sep_color)  \
printf("%s%s%s%*s%s%s%s%s %s\n", logo_color, row_index < logo->rows_used ? logo->row[row_index] : logo_substitute,          \
fields_color, (int) (MAX_PADDING - strlen(label)), "", label, sep_color, sep, values_color, data);

#define output_data_padded(data, logo, logo_substitute, row_index, logo_color, fields_color, values_color, sep_color)   \
printf("%s%s%s%*s%*s%s%s %s\n", logo_color, row_index < logo->rows_used ? logo->row[row_index] : logo_substitute,       \
fields_color, (int) MAX_PADDING, "", (int) strlen(sep), "", sep_color, values_color, data);

struct logo {
    char row[LOGO_ROWS_NUM][LOGO_LINE_LENGHT];
    unsigned int rows_used;
};

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

int main(int argc, char **argv) {
    char *home_dir = getpwuid(getuid())->pw_dir;
    char config_path[MAX_PATH_LENGTH];
    sprintf(config_path, "%s/%s", home_dir, CONFIG_PATH_SUFFIX);

    char *logo_color = BWHITE;
    char *fields_color = BCYAN;
    char *values_color = BWHITE;
    char *sep_color = BWHITE;

    struct logo *assigned_logo;
    char logo_substitute[64];

    char sep[9] = ":";

    char interface[MAX_INTERFACE_LENGTH];
    interface[0] = '\0';
    int show_interface = 1;

    double rx = -1, tx = -1;
    char rx_mu[16], tx_mu[16];
    int show_rx = 1, show_tx = 1;

    char mac[18];
    int mac_present = 0;
    int show_mac = 1;

    char *ip_addr_4[MAX_IPV4_NUM];
    unsigned int ipv4_num = 0;
    int show_ip4 = 1;

    char *ip_addr_6[MAX_IPV6_NUM];
    unsigned int ipv6_num = 0;
    int show_ip6 = 1;

    char **args;
    int args_num;

    if(args_from_file(&args, &args_num, config_path)) {
        handle_args(args, args_num, 1, interface, &logo_color, &fields_color, \
                    &values_color, &sep_color, sep, &show_interface, &show_rx,\
                    &show_tx, &show_mac, &show_ip4, &show_ip6);
        free_args(args, args_num);
    }

    args = argv;
    args_num = argc;

    handle_args(args, args_num, 0, interface, &logo_color, &fields_color, \
                &values_color, &sep_color, sep, &show_interface, &show_rx,\
                &show_tx, &show_mac, &show_ip4, &show_ip6);

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

    if(show_rx) if(rx != -1) to_formatted_bytes(rx_mu, rx);
    if(show_tx) if(tx != -1) to_formatted_bytes(tx_mu, tx);
    if(show_mac) mac_present = get_mac(mac, interface);
    if(show_ip4) ipv4_num = get_ip(ip_addr_4, interface, IPv4);
    if(show_ip6) ipv6_num = get_ip(ip_addr_6, interface, IPv6);
    assign_logo(&assigned_logo, interface);
    get_logo_space(logo_substitute, assigned_logo);

    unsigned int row_index = 0;

    if(show_interface) {
        output_data(interface, "INTERFACE", assigned_logo, logo_substitute, row_index, sep, logo_color, fields_color, values_color, sep_color);
        row_index++;
    }

    if(show_mac && mac_present) {
        output_data(mac, "MAC", assigned_logo, logo_substitute, row_index, sep, logo_color, fields_color, values_color, sep_color);
        row_index++;
    }

    if(show_ip4 && ipv4_num > 0) {
        output_data(ip_addr_4[0], "IPv4", assigned_logo, logo_substitute, row_index, sep, logo_color, fields_color, values_color, sep_color);
        free(ip_addr_4[0]);
        row_index++;
        for(unsigned int i = 1; i < ipv4_num; i++) {
            output_data_padded(ip_addr_4[i], assigned_logo, logo_substitute, row_index, logo_color, fields_color, values_color, sep_color);
            row_index++;
            free(ip_addr_4[i]);
        }
    }

    if(show_ip6 && ipv6_num > 0) {
        output_data(ip_addr_6[0], "IPv6", assigned_logo, logo_substitute, row_index, sep, logo_color, fields_color, values_color, sep_color);
        row_index++;
        free(ip_addr_6[0]);
        for(unsigned int i = 1; i < ipv6_num; i++) {
            output_data_padded(ip_addr_6[i], assigned_logo, logo_substitute, row_index, logo_color, fields_color, values_color, sep_color);
            row_index++;
            free(ip_addr_6[i]);
        }
    }

    if(show_rx && rx != -1) {
        output_data(rx_mu, "RX", assigned_logo, logo_substitute, row_index, sep, logo_color, fields_color, values_color, sep_color);
        row_index++;
    }
    if(show_tx && tx != -1) {
        output_data(tx_mu, "TX", assigned_logo, logo_substitute, row_index, sep, logo_color, fields_color, values_color, sep_color);
        row_index++;
    }

    while(row_index < assigned_logo->rows_used) {
        printf("%s%s\n", logo_color, assigned_logo->row[row_index]);
        row_index++;

    }

    printf("%s\n", WHITE);
}
