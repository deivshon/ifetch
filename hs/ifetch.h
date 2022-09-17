#ifndef IFETCH
#define IFETCH

#include "netutils.h"

#define LOGO_ROWS_NUM 8
#define LOGO_LINE_LENGHT 64
#define MAX_DATA_LENGTH 64
#define MAX_LABEL_LENGTH 16
#define MAX_SEP_LENGTH 8

#define IF_INDEX    0
#define MAC_INDEX   1
#define RX_INDEX    2
#define TX_INDEX    3

#define FIELDS_NUM  4

#define starts_with(str, prefix) !strncmp(str, prefix, strlen(prefix))

struct logo {
    char row[LOGO_ROWS_NUM][LOGO_LINE_LENGHT];
    unsigned int rows_used;
};

struct data_item {
    char data[MAX_DATA_LENGTH];
    char label[MAX_LABEL_LENGTH];
    int show;

    int exists;

    char arg_name[8];
};

struct ip_item {
    char *data[MAX_IPVX_NUM];
    char label[MAX_LABEL_LENGTH];
    int show;

    unsigned int ips_num;

    char arg_name[8];
};

#endif
