// recreation of the cat command
#include "cclarg.h"
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    HELP_FLAG,
    SHOW_ENDS_FLAG,
    SHOW_TABS_FLAG,
    SHOW_NONPRINTING_FLAG,
    FLAGS_COUNT
} FlagsEnum;

typedef enum { FILES_POSITIONAL, POSITIONALS_COUNT } PositionalEnum;

void help(const char *_);
void print_file(const char *file_name);
Flag flags[FLAGS_COUNT] = {
    {"-h", "--help", "Show help menu", NULL, false, false},
    {"-E", "--show-ends", "Display $ at the end of each line", NULL, false,
     false, NULL},
    {"-T", "--show-tabs", "Display TAB characters as ^I", NULL, false, false,
     NULL},
    {"-v", "--show-nonprinting",
     "Use ^ and M- notation, except for LFD and TAB", NULL, false, false,
     NULL}};

Positional positionals[POSITIONALS_COUNT] = {
    {"files", "Files to concatenate", NULL, NULL, 0, false, true}};

int main(int argc, char *argv[]) {
    int parse_return_code;
    parse_return_code = parse_args(argc, argv, flags, FLAGS_COUNT, positionals,
                                   POSITIONALS_COUNT);

    // error checking
    switch (parse_return_code) {
    case ARG_PARSE_INVALID_FLAG:
        printf("Parse Error: invalid flag\n");
        return EXIT_FAILURE;

    case ARG_PARSE_FLAG_REQUIRES_VALUE:
        printf("Parse Error: flag requires value\n");
        return EXIT_FAILURE;

    case ARG_PARSE_UNEXPECTED_POSITIONAL_ARGUMENT:
        printf("Parse Error: unexpected positional argument\n");
        return EXIT_FAILURE;

    default:
        break;
    }

    if (flags[HELP_FLAG].is_set) {
        print_help(argv[0], "Concatenate FILE(s) to standard output.", flags,
                   FLAGS_COUNT, positionals, POSITIONALS_COUNT);
        return EXIT_SUCCESS;
    }

    if (positionals[FILES_POSITIONAL].is_set) {
        for (int i = 0; i < positionals[FILES_POSITIONAL].value_count; i++)
            print_file(positionals[FILES_POSITIONAL].values[i]);
    }

    return EXIT_SUCCESS;
}

void print_file(const char *file) {
    FILE *fp = fopen(file, "r");
    if (fp == NULL) {
        perror("Error opening file");
        return;
    }

    char ch;
    while ((ch = fgetc(fp)) != EOF) {
        if (ch == '\t' && flags[SHOW_TABS_FLAG].is_set) {
            printf("^I");
        } else if (ch < 32 && flags[SHOW_NONPRINTING_FLAG].is_set &&
                   ch != 10) {      // ASCII 10 is newline
            printf("^%c", ch + 64); // +64 to convert control characters
                                    // to readable format
        } else {
            putchar(ch);
        }
    }

    if (flags[SHOW_ENDS_FLAG].is_set)
        printf("$");

    fclose(fp);
}
