#ifndef IFETCH
#define IFETCH

#include "netutils.h"

#define LOGO_ROWS_NUM 32
#define LOGO_LINE_LENGTH 32
#define MAX_DATA_LENGTH 64
#define MAX_LABEL_LENGTH 16
#define MAX_SEP_LENGTH 9

#define IF_INDEX    0
#define MAC_INDEX   1
#define IP4_INDEX   2
#define IP6_INDEX   3
#define RX_INDEX    4
#define TX_INDEX    5

#define FIELDS_NUM  6

#define starts_with(str, prefix) !strncmp(str, prefix, strlen(prefix))

struct logo {
    char row[LOGO_ROWS_NUM][LOGO_LINE_LENGTH];
    unsigned int rows_used;
};

struct data_item {
    char *data;
    char label[MAX_LABEL_LENGTH];
    int show;

    char sep[MAX_SEP_LENGTH];
    char *logo_color;
    char *field_color;
    char *sep_color;
    char *value_color;

    int instances;

    char arg_name[8];
};

int set_logo(struct logo *dest, char *logo_name, char *home_dir);

#endif
