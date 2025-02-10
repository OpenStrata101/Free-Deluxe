// src/utils.c
#include <stdio.h>
#include "utils.h"

#define HIDE_CURSOR "\033[?25l"
#define SHOW_CURSOR "\033[?25h"

void cleanup(void) {
    printf(SHOW_CURSOR);  // Restore cursor on exit
}

void setup_terminal(void) {
    printf(HIDE_CURSOR);  // Hide cursor during execution
}

void format_size(unsigned long bytes, char *result, ProgramOptions *opts) {
    const char* units[] = {"B", "KiB", "MiB", "GiB", "TiB"};
    double size = bytes;
    int unit = 0;
    
    // Convert to appropriate unit
    while (size >= 1024.0 && unit < 4) {
        size /= 1024.0;
        unit++;
    }
    
    // Format with different precisions based on unit size
    if (unit == 0) {
        // Bytes should be displayed as whole numbers
        sprintf(result, "%.0f %s", size, units[unit]);
    } else if (unit == 1) {
        // KiB with one decimal place
        sprintf(result, "%.1f %s", size, units[unit]);
    } else {
        // MiB, GiB, TiB with two decimal places
        sprintf(result, "%.2f %s", size, units[unit]);
    }
}