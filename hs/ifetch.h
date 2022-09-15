#ifndef IFETCH
#define IFETCH

#define LOGO_ROWS_NUM 8
#define LOGO_LINE_LENGHT 64
#define MAX_DATA_LENGTH 64
#define MAX_LABEL_LENGTH 16
#define MAX_SEP_LENGTH 8

struct logo {
    char row[LOGO_ROWS_NUM][LOGO_LINE_LENGHT];
    unsigned int rows_used;
};

struct data_item {
    char data[MAX_DATA_LENGTH];
    char label[MAX_LABEL_LENGTH];
    int label_chosen;
    int show;

    int exists;
};

#endif
