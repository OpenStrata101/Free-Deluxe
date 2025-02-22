// src/args.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include "args.h"
#include "utils.h"

#define MAX_SECONDS 3600
#define MAX_COUNT 1000

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

// Function to validate numeric input
static int parse_number(const char *str, int *result, int min, int max) {
    char *endptr;
    errno = 0;  // Reset errno before calling strtol
    
    // Convert string to number
    long val = strtol(str, &endptr, 10);
    
    // Check for conversion errors
    if (errno == ERANGE || val > INT_MAX || val < INT_MIN) {
        return -1;  // Number out of range
    }
    
    // Check if the entire string was consumed
    if (*endptr != '\0') {
        return -1;  // Invalid characters in input
    }
    
    // Check bounds
    if (val < min || val > max) {
        return -1;  // Outside of allowed range
    }
    
    *result = (int)val;
    return 0;
}

// Function to validate and handle numeric arguments
static int handle_numeric_arg(const char *optarg, int *target, int min, int max, const char *option_name) {
    if (parse_number(optarg, target, min, max) != 0) {
        fprintf(stderr, "Error: Invalid value for --%s. Must be between %d and %d\n", 
                option_name, min, max);
        return -1;
    }
    return 0;
}

// Function to check for mutually exclusive options
static int check_exclusive_options(ProgramOptions *opts) {
    int unit_count = 0;
    
    // Count how many unit options are set
    unit_count += (opts->unit == 1); // bytes
    unit_count += (opts->unit == 2); // kilo
    unit_count += (opts->unit == 3); // mega
    unit_count += (opts->unit == 4); // giga
    unit_count += (opts->unit == 5); // tera
    unit_count += (opts->unit == 0); // human
    
    if (unit_count > 1) {
        fprintf(stderr, "Error: Only one unit option (-b, -k, -m, -g, -t, -h) can be specified\n");
        return -1;
    }
    
    return 0;
}

ProgramOptions parse_args(int argc, char **argv) {
    ProgramOptions opts = {0}; // Initialize all to 0
    int option_index = 0;
    int c;
    int error = 0;

    // Reset getopt
    optind = 1;

    while ((c = getopt_long(argc, argv, "bkmgthds:c:wHVS",
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
            
            case 's':
                if (handle_numeric_arg(optarg, &opts.repeat_seconds, 0, MAX_SECONDS, "seconds") != 0) {
                    error = 1;
                }
                break;
                
            case 'c':
                if (handle_numeric_arg(optarg, &opts.repeat_count, 0, MAX_COUNT, "count") != 0) {
                    error = 1;
                }
                break;
                
            case 'w': 
                opts.wide_output = 1; 
                break;
                
            case 'H': 
                show_help(); 
                exit(EXIT_SUCCESS);
                
            case 'V': 
                show_version(); 
                exit(EXIT_SUCCESS);
                
            case '?':
                // getopt_long already printed an error message
                error = 1;
                break;
                
            default:
                fprintf(stderr, "Error: Unknown option %c\n", c);
                error = 1;
                break;
        }
    }

    // Check for non-option arguments
    if (optind < argc) {
        fprintf(stderr, "Error: Unexpected argument: %s\n", argv[optind]);
        error = 1;
    }

    // Check for mutually exclusive options
    if (check_exclusive_options(&opts) != 0) {
        error = 1;
    }

    // If any error occurred, show help and exit
    if (error) {
        show_help();
        exit(EXIT_FAILURE);
    }

    return opts;
}

void show_help(void) {
    printf("Usage: %s [options]\n\n", PROGRAM_NAME);
    printf("Options:\n");
    printf("  -b, --bytes         show output in bytes\n");
    printf("  -k, --kilo          show output in kilobytes\n");
    printf("  -m, --mega          show output in megabytes\n");
    printf("  -g, --giga          show output in gigabytes\n");
    printf("  -t, --tera          show output in terabytes\n");
    printf("  -h, --human         show human-readable output (default)\n");
    printf("  -d, --deluxe        show deluxe output with icons\n");
    printf("  -S, --si            use powers of 1000 not 1024\n");
    printf("  -s N, --seconds N   repeat printing every N seconds (0-%d)\n", MAX_SECONDS);
    printf("  -c N, --count N     repeat printing N times (0-%d)\n", MAX_COUNT);
    printf("  -w, --wide          use wide output format\n");
    printf("      --help          display this help and exit\n");
    printf("  -V, --version       output version information and exit\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s -d               show deluxe output with icons\n", PROGRAM_NAME);
    printf("  %s -h -s 1          show human-readable output, updating every second\n", PROGRAM_NAME);
    printf("  %s -m -w            show megabytes in wide format\n", PROGRAM_NAME);
}

void show_version(void) {
    printf("%s version %s\n", PROGRAM_NAME, VERSION);
    printf("Copyright (C) 2024 Your Name\n");
    printf("License GPLv3+: GNU GPL version 3 or later\n");
}