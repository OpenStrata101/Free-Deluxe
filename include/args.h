// include/args.h
#ifndef ARGS_H
#define ARGS_H

typedef struct {
    int display_mode;    // 0: normal, 1: deluxe
    int unit;           // 0: auto, 1: bytes, 2: KB, 3: MB, 4: GB, 5: TB
    int repeat_seconds; // 0: no repeat, >0: seconds between updates
    int repeat_count;   // 0: infinite, >0: number of repeats
    int wide_output;    // 0: normal, 1: wide
    int show_total;     // 0: no total, 1: show total
    int single_line;    // 0: normal, 1: single line
    int si_units;       // 0: power of 1024, 1: power of 1000
} ProgramOptions;

ProgramOptions parse_args(int argc, char **argv);
void show_help(void);
void show_version(void);

#endif
