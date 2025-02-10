// src/utils.c
#include <stdio.h>
#include "utils.h"

#define HIDE_CURSOR "\033[?25l"
#define SHOW_CURSOR "\033[?25h"
#define UNIT_COUNT 5
#define KILOBYTE 1024.0

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
    while (size >= KILOBYTE && unit < UNIT_COUNT - 1) {
        size /= KILOBYTE;
        unit++;
    }

    // Format with different precisions based on unit size
    const int precision = (unit == 0) ? 0 : (unit == 1) ? 1 : 2;
    sprintf(result, "%.*f %s", precision, size, units[unit]);
}
