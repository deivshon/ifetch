#include "../hs/argutils.h"
#include "../hs/netutils.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void handle_args(char **argv, int argc, int from_config,    \
                 char *interface, char **logo_color,        \
                 char **fields_color, char **values_color,  \
                 char **sep_color, char *sep,               \
                 int *ascii_strict)
{
    char *error_premise = from_config ? "Error in config file\n" : "";
    int ai = 1;
    while(ai < argc) {
        // Case for interface
        if(argv[ai][0] != '-' || strcmp("-i", argv[ai]) == 0) {
            if(argv[ai][0] == '-') {
                if(ai + 1 >= argc) {
                    printf("%sYou must provide an interface after the \"-i\" option\n", error_premise);
                    exit(EXIT_FAILURE);
                }
                ai++;
            }

            argv[ai][strcspn(argv[ai], " ")] = '\0';
            strcpy(interface, argv[ai]);
            if(!interface_exists(interface)) {
                printf("%sNo interface named \"%s\" exists\n", error_premise, interface);
                exit(EXIT_FAILURE);
            }
        }
        // Other options
        else if(strcmp("-fc", argv[ai]) == 0) {
            handle_color_argument(fields_color, &ai, argc, argv, error_premise);
        }
        else if(strcmp("-vc", argv[ai]) == 0) {
            handle_color_argument(values_color, &ai, argc, argv, error_premise);
        }
        else if(strcmp("-sc", argv[ai]) == 0) {
            handle_color_argument(sep_color, &ai, argc, argv, error_premise);
        }
        else if(strcmp("-lc", argv[ai]) == 0) {
            handle_color_argument(logo_color, &ai, argc, argv, error_premise);
        }
        else if(strcmp("-ns", argv[ai]) == 0) {
            strcpy(sep, "");
        }
        else if(strcmp("-s", argv[ai]) == 0) {
            if(ai + 1 >= argc) {
                printf("%sYou must provide a separator to use after the \"-s\" option\n", error_premise);
                exit(EXIT_FAILURE);
            }

            ai++;
            if(strlen(argv[ai]) >= 9) {
                printf("%s\"%s\" is not a valid separator\nThe separator must be at maximum 8 characters long\n", error_premise, argv[ai]);
                exit(EXIT_FAILURE);
            }
            strcpy(sep, argv[ai]);
        }
        else if(strcmp("-ascii", argv[ai]) == 0) {
            (*ascii_strict) = 1;
        }
        else {
            printf("%sUncrecognized argument: \"%s\"\n", error_premise, argv[ai]);
            exit(EXIT_FAILURE);
        }
        ai++;
    }
}

void handle_color_argument(char **dest, int *ai, int argc,  \
                           char **argv, char *error_premise)
{
    if((*ai) + 1 >= argc) {
        printf("%sYou must provide a color after the \"%s\" option\n", error_premise, argv[*ai]);
        exit(EXIT_FAILURE);
    }
    
    (*ai)++;
    argv[*ai][strcspn(argv[*ai], " ")] = '\0';
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

    int i = 1;
    char buf[MAX_ARG_SIZE * 2 + 1];
    char *buf_split;
    int len_add;

    while(i < MAX_ARGS) {
        if(!fgets(buf, MAX_ARG_SIZE * 2 + 1, fs)) break;
        if(buf[0] == '#') continue;
        buf[strcspn(buf, "\n")] = '\0';

        buf_split = strtok(buf, "=");
        if(strlen(buf_split) > MAX_ARG_SIZE) exit_config_error(buf_split);

        len_add = buf_split[0] != '-' ? 1 : 0;

        (*argv)[i] = malloc(sizeof(char) * (strlen(buf_split) + 1 + len_add));
        if(buf_split[0] != '-') 
            sprintf((*argv)[i], "%s%s", "-", buf_split);
        else
            strcpy((*argv)[i], buf_split);
        i++;

        buf_split = strtok(NULL, "=");
        if(buf_split == NULL) continue;
        if(strlen(buf_split) > MAX_ARG_SIZE) exit_config_error(buf_split);
    
        (*argv)[i] = malloc(sizeof(char) * (strlen(buf_split) + 1));
        strcpy((*argv)[i], buf_split);
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
