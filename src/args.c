// src/args.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "args.h"
#include "utils.h"

static struct option long_options[] = {
    {"bytes",     no_argument,       0, 'b'},
    {"kilo",      no_argument,       0, 'k'},
    {"mega",      no_argument,       0, 'm'},
    {"giga",      no_argument,       0, 'g'},
    {"tera",      no_argument,       0, 't'},
    {"human",     no_argument,       0, 'h'},
    {"deluxe",    no_argument,       0, 'd'},
    {"si",        no_argument,       0, 'S'},
    {"seconds",   required_argument, 0, 's'},
    {"count",     required_argument, 0, 'c'},
    {"wide",      no_argument,       0, 'w'},
    {"help",      no_argument,       0, 'H'},
    {"version",   no_argument,       0, 'V'},
    {0, 0, 0, 0}
};

ProgramOptions parse_args(int argc, char **argv) {
    ProgramOptions opts = {0}; // Initialize all to 0
    int option_index = 0;
    int c;

    while ((c = getopt_long(argc, argv, "bkmgthds:c:wHV",
                           long_options, &option_index)) != -1) {
        switch (c) {
            case 'b': opts.unit = 1; break;
            case 'k': opts.unit = 2; break;
            case 'm': opts.unit = 3; break;
            case 'g': opts.unit = 4; break;
            case 't': opts.unit = 5; break;
            case 'h': opts.unit = 0; break;
            case 'd': opts.display_mode = 1; break;
            case 'S': opts.si_units = 1; break;
            case 's': opts.repeat_seconds = atoi(optarg); break;
            case 'c': opts.repeat_count = atoi(optarg); break;
            case 'w': opts.wide_output = 1; break;
            case 'H': show_help(); exit(0);
            case 'V': show_version(); exit(0);
            default: show_help(); exit(1);
        }
    }
    
    return opts;
}

void show_help(void) {
    printf("Usage: %s [options]\n", PROGRAM_NAME);
    printf("Options:\n");
    printf("  -b, --bytes         show output in bytes\n");
    printf("  -k, --kilo          show output in kilobytes\n");
    printf("  -m, --mega          show output in megabytes\n");
    printf("  -g, --giga          show output in gigabytes\n");
    printf("  -t, --tera          show output in terabytes\n");
    printf("  -h, --human         show human-readable output\n");
    printf("  -d, --deluxe        show deluxe output with icons\n");
    printf("  -S, --si            use powers of 1000 not 1024\n");
    printf("  -s N, --seconds N   repeat printing every N seconds\n");
    printf("  -c N, --count N     repeat printing N times\n");
    printf("  -w, --wide          wide output\n");
    printf("      --help          display this help and exit\n");
    printf("  -V, --version       output version information and exit\n");
}

void show_version(void) {
    printf("%s version %s\n", PROGRAM_NAME, VERSION);
}