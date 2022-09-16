#ifndef ARGUTILS
#define ARGUTILS

#include "ifetch.h"

#define MAX_ARGS 128
#define MAX_ARG_SIZE 16

#define BRED    "\033[1m\033[31m" // Code: 'R'
#define BGREEN  "\033[1m\033[32m" // Code: 'G'
#define BYELLOW "\033[1m\033[33m" // Code: 'Y'
#define BBLUE   "\033[1m\033[34m" // Code: 'B'
#define BMAG    "\033[1m\033[35m" // Code: 'M'
#define BCYAN   "\033[1m\033[36m" // Code: 'C'
#define BWHITE  "\033[1m\033[37m" // Code: 'W'
#define RED     "\033[0m\033[31m" // Code: 'r'
#define GREEN   "\033[0m\033[32m" // Code: 'g'
#define YELLOW  "\033[0m\033[33m" // Code: 'y'
#define BLUE    "\033[0m\033[34m" // Code: 'b'
#define MAG     "\033[0m\033[35m" // Code: 'm'
#define CYAN    "\033[0m\033[36m" // Code: 'c'
#define WHITE   "\033[0m\033[37m" // Code: 'w'

void handle_args(char **argv, int argc, int from_config,    \
                 char *interface, char **logo_color,        \
                 char **fields_color, char **values_color,  \
                 char **sep_color, char *sep, int *show_ip4,\
                 int *show_ip6, struct data_item items[]);

int assign_color(char **dest, char code);

int args_from_file(char ***argv, int *argc, char *file_path);

void free_args(char **argv, int argc);

#endif
