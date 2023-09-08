// cclarg.h
#ifndef CCLARG_H
#define CCLARG_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *short_name;
    const char *long_name;
    const char *description;
    const char *value;
    bool requires_value;
    bool is_set;
    void (*callback)(const char *value);
} Flag;

typedef struct {
    const char *name;
    const char *description;
    char *value;   // use if not is_variadic
    char **values; // use if is_variadic
    int value_count;
    bool is_set;
    bool is_variadic;
} Positional;

typedef enum {
    ARG_PARSE_SUCCESS,
    ARG_PARSE_INVALID_FLAG,
    ARG_PARSE_FLAG_REQUIRES_VALUE,
    ARG_PARSE_UNEXPECTED_POSITIONAL_ARGUMENT
} ArgParseResult;

ArgParseResult parse_args(int argc, char *argv[], Flag flags[], int flag_count,
                          Positional positionals[], int pos_count) {
    int positional_index = 0;
    for (int i = 1; i < argc; i++) {
        bool matched = false;

        // if it starts '-', it is a flag
        if (flags && argv[i][0] == '-') {
            for (int j = 0; j < flag_count; j++) {
                if (strncmp(flags[j].short_name, argv[i],
                            strlen(flags[j].short_name)) == 0 ||
                    strncmp(flags[j].long_name, argv[i],
                            strlen(flags[j].long_name)) == 0) {

                    flags[j].is_set = true;
                    matched = true;

                    if (flags[j].requires_value) {
                        char *equal_char = strchr(argv[i], '=');
                        if (equal_char) {
                            flags[j].value = equal_char + 1;
                        } else if (i + 1 < argc) {
                            flags[j].value = argv[++i];
                        } else {
                            return ARG_PARSE_FLAG_REQUIRES_VALUE;
                        }
                    }

                    if (flags[j].callback)
                        flags[j].callback(flags[j].value ? flags[j].value
                                                         : NULL);

                    break; // break out of flag loop since we've found a match
                }
            }

            if (!matched)
                return ARG_PARSE_INVALID_FLAG;

        } else if (positionals) {
            if (positionals[positional_index].is_variadic) {
                positionals[positional_index].values = &argv[i];
                positionals[positional_index].value_count = argc - i;
                positionals[positional_index].is_set = true;
                break; // exit the loop as we've consumed all remaining
                       // arguments
            } else if (positional_index < pos_count) {
                positionals[positional_index].value = argv[i];
                positionals[positional_index].is_set = true;
                positional_index++;
            } else {
                return ARG_PARSE_UNEXPECTED_POSITIONAL_ARGUMENT;
            }
        }
    }

    return ARG_PARSE_SUCCESS;
}

void print_help(const char *name, const char *desc, Flag flags[],
                const int FLAGS_COUNT, Positional positionals[],
                const int POSITIONALS_COUNT) {
    printf("Concatenate FILE(s) to standard output.\n\n");

    // determine the maximum width needed for flags
    int max_width = 0;
    for (int i = 0; i < FLAGS_COUNT; i++) {
        int width = strlen(flags[i].short_name) + strlen(flags[i].long_name) +
                    2; // +2 for ", "
        if (width > max_width) {
            max_width = width;
        }
    }

    printf("Options:\n");
    // display available flags and their descriptions
    for (int i = 0; i < FLAGS_COUNT; i++) {
        printf("  %s, %-*s %s\n", flags[i].short_name, max_width,
               flags[i].long_name, flags[i].description);
    }

    // Determine the maximum width needed for positionals
    int max_width_positionals = 0;
    for (int i = 0; i < POSITIONALS_COUNT; i++) {
        int width = strlen(positionals[i].name);
        if (width > max_width_positionals) {
            max_width_positionals = width;
        }
    }

    printf("\nPositional arguments:\n");
    // display available positionals and their descriptions
    for (int i = 0; i < POSITIONALS_COUNT; i++) {
        printf("  %-*s  %s\n", max_width_positionals, positionals[i].name,
               positionals[i].description);
    }
}

#endif // CCLARG_H
