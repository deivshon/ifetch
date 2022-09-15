#include "../hs/argutils.h"
#include "../hs/netutils.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define spacing_chars " \t\n\v\f\r"

static void step_arg_next(char **argv, int argc, int *ai,   \
                          char *error_premise) {
    if((*ai) + 1 >= argc) {
        printf("%sYou must provide an argument after the \"%s\" option\n", error_premise, argv[*ai]);
        exit(EXIT_FAILURE);
    }
    
    (*ai)++;
}

static void handle_color_argument(char **dest, int *ai, char **argv,    \
                                  char *error_premise)
{
    if(strlen(argv[*ai]) > 1) {
        printf("%s\"%s\" is not a valid color code\n", error_premise, argv[*ai]);
        exit(EXIT_FAILURE);
    }

    int assigned = assign_color(dest, argv[*ai][0]);
    if(!assigned) {
        printf("%s\"%s\" is not a valid color code\n", error_premise, argv[*ai]);
        exit(EXIT_FAILURE);
    }
}

static void handle_show_argument(int *dest, int *ai, char **argv,   \
                                 char *error_premise)
{
    if(!strcmp("s", argv[*ai]) || !strcmp("show", argv[*ai]))
        (*dest) = 1;
    else if(!strcmp("h", argv[*ai]) || !strcmp("hide", argv[*ai]))
        (*dest) = 0;
    else {
        printf("%s\"%s\" is not a valid argument for the \"%s\" option.\nValid arguments: s | show | h | hide\n", error_premise, argv[*ai], argv[(*ai) - 1]);
        exit(EXIT_FAILURE);
    }
}

void handle_args(char **argv, int argc, int from_config,    \
                 char *interface, char **logo_color,        \
                 char **fields_color, char **values_color,  \
                 char **sep_color, char *sep,               \
                 int *show_interface, int *show_rx,         \
                 int *show_tx, int *show_mac, int *show_ip4,\
                 int *show_ip6)
{
    char *error_premise = from_config ? "Error in config file\n" : "";
    int ai = 1;
    while(ai < argc) {
        // Case for interface
        if(argv[ai][0] != '-' || strcmp("-ifn", argv[ai]) == 0) {
            if(argv[ai][0] == '-') {
                step_arg_next(argv, argc, &ai, error_premise);
            }

            argv[ai][strcspn(argv[ai], " ")] = '\0';
            strcpy(interface, argv[ai]);
            if(!interface_exists(interface)) {
                printf("%sNo interface named \"%s\" exists\n", error_premise, interface);
                exit(EXIT_FAILURE);
            }
        }
        // Other options
        else if(!strcmp("-fc", argv[ai])) {
            step_arg_next(argv, argc, &ai, error_premise);
            handle_color_argument(fields_color, &ai, argv, error_premise);
        }
        else if(!strcmp("-vc", argv[ai])) {
            step_arg_next(argv, argc, &ai, error_premise);
            handle_color_argument(values_color, &ai, argv, error_premise);
        }
        else if(!strcmp("-sc", argv[ai])) {
            step_arg_next(argv, argc, &ai, error_premise);
            handle_color_argument(sep_color, &ai, argv, error_premise);
        }
        else if(!strcmp("-lc", argv[ai])) {
            step_arg_next(argv, argc, &ai, error_premise);
            handle_color_argument(logo_color, &ai, argv, error_premise);
        }
        else if(!strcmp("-ns", argv[ai])) {
            strcpy(sep, "");
        }
        else if(!strcmp("-s", argv[ai])) {
            step_arg_next(argv, argc, &ai, error_premise);

            if(strlen(argv[ai]) >= 9) {
                printf("%s\"%s\" is not a valid separator\nThe separator must be at maximum 8 characters long\n", error_premise, argv[ai]);
                exit(EXIT_FAILURE);
            }
            strcpy(sep, argv[ai]);
        }
        else if(!strcmp("-if", argv[ai])) {
            step_arg_next(argv, argc, &ai, error_premise);
            handle_show_argument(show_interface, &ai, argv, error_premise);
        }
        else if(!strcmp("-mac", argv[ai])) {
            step_arg_next(argv, argc, &ai, error_premise);
            handle_show_argument(show_mac, &ai, argv, error_premise);
        }
        else if(!strcmp("-ip4", argv[ai])) {
            step_arg_next(argv, argc, &ai, error_premise);
            handle_show_argument(show_ip4, &ai, argv, error_premise);
        }
        else if(!strcmp("-ip6", argv[ai])) {
            step_arg_next(argv, argc, &ai, error_premise);
            handle_show_argument(show_ip6, &ai, argv, error_premise);
        }
        else if(!strcmp("-rx", argv[ai])) {
            step_arg_next(argv, argc, &ai, error_premise);
            handle_show_argument(show_rx, &ai, argv, error_premise);
        }
        else if(!strcmp("-tx", argv[ai])) {
            step_arg_next(argv, argc, &ai, error_premise);
            handle_show_argument(show_tx, &ai, argv, error_premise);
        }
        else {
            printf("%sUncrecognized argument: \"%s\"\n", error_premise, argv[ai]);
            exit(EXIT_FAILURE);
        }
        ai++;
    }
}

int assign_color(char **dest, char code) {
    int assigned = 1;
    switch(code) {
        case 'R':
            *dest = BRED;
            break;
        case 'G':
            *dest = BGREEN;
            break;
        case 'Y':
            *dest = BYELLOW;
            break;
        case 'B':
            *dest = BBLUE;
            break;
        case 'M':
            *dest = BMAG;
            break;
        case 'C':
            *dest = BCYAN;
            break;
        case 'W':
            *dest = BWHITE;
            break;
        case 'r':
            *dest = RED;
            break;
        case 'g':
            *dest = GREEN;
            break;
        case 'y':
            *dest = YELLOW;
            break;
        case 'b':
            *dest = BLUE;
            break;
        case 'm':
            *dest = MAG;
            break;
        case 'c':
            *dest = CYAN;
            break;
        case 'w':
            *dest = WHITE;
            break;
        default:
            assigned = 0;
            break;
    }

    return assigned;
}

static void exit_config_error(char *wrong_arg) {
    printf("Error in config file\n\"%s\" is too long, arguments from config files can be at maximum %d characters long\n", wrong_arg, MAX_ARG_SIZE);
    exit(EXIT_FAILURE);
}

int args_from_file(char ***argv, int *argc, char *file_path) {
    FILE *fs = fopen(file_path, "r");
    if(fs == NULL) {
        return 0;
    }

    (*argv) = calloc(MAX_ARGS, sizeof(char *));

    char buf[MAX_ARG_SIZE * 2 + 1];
    char *buf_split;
    int start;
    char *arg;

    int len_add;
    int i = 1;
    while(i < MAX_ARGS) {
        if(!fgets(buf, MAX_ARG_SIZE * 2 + 1, fs)) break;

        buf_split = strtok(buf, "=");

        for(start = 0; isspace(buf_split[start]); start++);
        arg = buf_split + sizeof(char) * start;
        arg[strcspn(arg, spacing_chars)] = '\0';

        if(arg[0] == '#' || arg[0] == '\0') continue;
        if(strlen(arg) > MAX_ARG_SIZE) exit_config_error(arg);

        len_add = arg[0] != '-' ? 1 : 0;
        (*argv)[i] = malloc(sizeof(char) * (strlen(arg) + 1 + len_add));
        if(arg[0] != '-')
            sprintf((*argv)[i], "%s%s", "-", arg);
        else
            strcpy((*argv)[i], arg);
        i++;
        if(i >= MAX_ARGS) break;

        buf_split = strtok(NULL, "\n");
        if(buf_split == NULL) continue;

        for(start = 0; isspace(buf_split[start]); start++);
        if(buf_split[start] == '"') {
            for(; buf_split[start] == '"'; start++);
            arg = buf_split + sizeof(char) * start;

            for(start++; buf_split[start] != '"'; start++);
            buf_split[start] = '\0';
        }
        else {
            arg = buf_split + sizeof(char) * start;

            // If the argument was not quoted, trailing spaces should not be considered
            arg[strcspn(arg, spacing_chars)] = '\0';
        }

        if(strlen(arg) > MAX_ARG_SIZE) exit_config_error(arg);
        (*argv)[i] = malloc(sizeof(char) * (strlen(arg) + 1));
        strcpy((*argv)[i], arg);
        i++;
    }

    (*argc) = i;
    fclose(fs);
    return 1;
}

void free_args(char **argv, int argc) {
    for(int i = 0; i < argc; i++) {
        free(argv[i]);
    }
    free(argv);
}
