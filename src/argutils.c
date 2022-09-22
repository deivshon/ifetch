#include "../hs/argutils.h"
#include "../hs/netutils.h"
#include "../hs/ifetch.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define spacing_chars " \t\n\v\f\r"
#define is_digit(c) (c >= 48 && c <= 57)

static void step_arg_next(char **argv, int argc, int *ai,   \
                          char *error_premise) {
    if((*ai) + 1 >= argc) {
        printf("%sYou must provide an argument after the \"%s\" option\n", error_premise, argv[*ai]);
        exit(EXIT_FAILURE);
    }
    
    (*ai)++;
}

static void unrecognized_argument(char *arg, char *error_premise) {
    printf("%sUnrecognized argument: \"%s\"\n", error_premise, arg);
    exit(EXIT_FAILURE);
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

static void handle_label_argument(char *dest, int *ai, char **argv, \
                                 char *error_premise)
{
    if(strlen(argv[*ai]) >= MAX_LABEL_LENGTH) {
        printf("%s\"%s\" is not a valid label. Labels have a maximum length of %d\n", error_premise, argv[*ai], MAX_LABEL_LENGTH);
        exit(EXIT_FAILURE);
    }

    strcpy(dest, argv[*ai]);
}

static void handle_sep_argument(char *dest, char *sep, char *error_premise) {
    if(strlen(sep) >= MAX_SEP_LENGTH) {
        printf("%s\"%s\" is not a valid separator\nThe separator must be at maximum %d characters long\n", error_premise, sep, MAX_SEP_LENGTH - 1);
        exit(EXIT_FAILURE);
    }

    strcpy(dest, sep);
}

static void handle_data_argument(char **argv, int argc,             \
                                 struct data_item *data, int *ai,   \
                                 char *error_premise)
{
    // Show argument case
    if(strlen(argv[*ai]) == strlen(data->arg_name)) {
        step_arg_next(argv, argc, ai, error_premise);
        handle_show_argument(&(data->show), ai, argv, error_premise);
        return;
    }

    char *sub_arg = argv[*ai] + sizeof(char) * strlen(data->arg_name);
    if(!strcmp("l", sub_arg)) {
        step_arg_next(argv, argc, ai, error_premise);
        handle_label_argument(data->label, ai, argv, error_premise);
    }
    else if(!strcmp("loc", sub_arg)) {
        step_arg_next(argv, argc, ai, error_premise);
        handle_color_argument(&(data->logo_color), ai, argv, error_premise);
    }
    else if(!strcmp("fc", sub_arg)) {
        step_arg_next(argv, argc, ai, error_premise);
        handle_color_argument(&(data->field_color), ai, argv, error_premise);
    }
    else if(!strcmp("s", sub_arg)) {
        step_arg_next(argv, argc, ai, error_premise);
        handle_sep_argument(data->sep, argv[*ai], error_premise);
    }
    else if(!strcmp("ns", sub_arg)) {
        handle_sep_argument(data->sep, " ", error_premise);
    }
    else if(!strcmp("sc", sub_arg)) {
        step_arg_next(argv, argc, ai, error_premise);
        handle_color_argument(&(data->sep_color), ai, argv, error_premise);
    }
    else if(!strcmp("vc", sub_arg)) {
        step_arg_next(argv, argc, ai, error_premise);
        handle_color_argument(&(data->value_color), ai, argv, error_premise);
    }
    else unrecognized_argument(argv[*ai], error_premise);
}

static int data_arg_index(int *dest, char *arg, struct data_item items[]) {
    for(int i = 0; i < FIELDS_NUM; i++) {
        if(starts_with(arg, items[i].arg_name)) {
            (*dest) = i;
            return 1;
        }
    }

    return 0;
}

static int is_number(char *str) {
    for(unsigned int i = 0; i < strlen(str); i++) {
        if(!is_digit(str[i])) return 0;
    }

    return 1;
}

static void handle_num_argument(unsigned int *dest,     \
                                char **argv, int ai,    \
                                char *error_premise)
{
    if(!is_number(argv[ai])) {
        printf("%s\"%s\" is an invalid argument for the %s option\n", error_premise, argv[ai], argv[ai - 1]);
        exit(EXIT_FAILURE);
    }

    (*dest) = atoi(argv[ai]);
}

static void handle_logo_argument(struct logo *dest, char **argv,    \
                                 int *ai, char *home_dir,           \
                                 char *error_premise) {
    if(!set_logo(dest, argv[*ai], home_dir)) {
        printf("%sNo logo file named \"%s\" was found\n", error_premise, argv[*ai]);
        exit(EXIT_FAILURE);
    }
}

void handle_args(char **argv, int argc, int from_config,    \
                 char *interface, struct data_item items[], \
                 unsigned int *logo_field_distance,         \
                 unsigned int *min_padding,                 \
                 struct logo *logo, char *home_dir,         \
                 int *logo_chosen)
{
    char *error_premise = from_config ? "Error in config file\n" : "";
    int data_index = -1;
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
            for(int i = 0; i < FIELDS_NUM; i++) {
                handle_color_argument(&(items[i].field_color), &ai, argv, error_premise);
            }
        }
        else if(!strcmp("-vc", argv[ai])) {
            step_arg_next(argv, argc, &ai, error_premise);
            for(int i = 0; i < FIELDS_NUM; i++) {
                handle_color_argument(&(items[i].value_color), &ai, argv, error_premise);
            }
        }
        else if(!strcmp("-sc", argv[ai])) {
            step_arg_next(argv, argc, &ai, error_premise);
            for(int i = 0; i < FIELDS_NUM; i++) {
                handle_color_argument(&(items[i].sep_color), &ai, argv, error_premise);
            }
        }
        else if(!strcmp("-loc", argv[ai])) {
            step_arg_next(argv, argc, &ai, error_premise);
            for(int i = 0; i < FIELDS_NUM; i++) {
                handle_color_argument(&(items[i].logo_color), &ai, argv, error_premise);
            }
        }
        else if(!strcmp("-ns", argv[ai])) {
            for(int i = 0; i < FIELDS_NUM; i++) {
                handle_sep_argument(items[i].sep, " ", error_premise);
            }
        }
        else if(!strcmp("-s", argv[ai])) {
            step_arg_next(argv, argc, &ai, error_premise);

            for(int i = 0; i < FIELDS_NUM; i++) {
                handle_sep_argument(items[i].sep, argv[ai], error_premise);
            }
        }
        else if(data_arg_index(&data_index, argv[ai], items)) {
            handle_data_argument(argv, argc, &(items[data_index]), &ai, error_premise);
        }
        else if(!strcmp("-mld", argv[ai])) {
            step_arg_next(argv, argc, &ai, error_premise);
            handle_num_argument(logo_field_distance, argv, ai, error_premise);

            if((*logo_field_distance) > 64) {
                printf("%s\"%s\" is an invalid argument for the %s option\nLogo-field distance can have a maximum value of 64\n", error_premise, argv[ai], argv[ai - 1]);\
                exit(EXIT_FAILURE);
            }
        }
        else if(!strcmp("-mp", argv[ai])) {
            step_arg_next(argv, argc, &ai, error_premise);
            handle_num_argument(min_padding, argv, ai, error_premise);

            if((*min_padding) > 64) {
                printf("%s\"%s\" is an invalid argument for the %s option\nMinimum padding can have a maximum value of 64\n", error_premise, argv[ai], argv[ai - 1]);\
                exit(EXIT_FAILURE);
            }
        }
        else if(!strcmp("-lo", argv[ai])) {
            step_arg_next(argv, argc, &ai, error_premise);
            handle_logo_argument(logo, argv, &ai, home_dir, error_premise);
            (*logo_chosen) = 1;
        }
        else {
            unrecognized_argument(argv[ai], error_premise);
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

int logo_from_file(struct logo *dest, char *path, unsigned max_rows, unsigned max_row_length) {
    FILE *fs = fopen(path, "r");
    if(fs == NULL) return 0;
    char buf[max_row_length];

    unsigned int i = 0;
    while(i < max_rows) {
        if(!fgets(buf, max_row_length, fs)) break;
        
        buf[strcspn(buf, "\n")] = '\0';

        strcpy(dest->row[i], buf);
        i++;
    }
    fclose(fs);

    dest->rows_used = i;

    return 1;
}

void free_args(char **argv, int argc) {
    for(int i = 0; i < argc; i++) {
        free(argv[i]);
    }
    free(argv);
}
