#include <stdio.h>
#include <string.h>
#include "utils.h"

#define HIDE_CURSOR "\033[?25l"
#define SHOW_CURSOR "\033[?25h"
#define UNIT_COUNT 5
#define KILOBYTE 1024.0
#define MAX_RESULT_SIZE 32  // Maximum size for formatted string

// Unit strings are now static const to ensure they're stored in read-only memory
static const char* const UNITS[] = {"B", "KiB", "MiB", "GiB", "TiB"};

void cleanup(void) {
    // Flush stdout before showing cursor to ensure proper order
    fflush(stdout);
    fputs(SHOW_CURSOR, stdout);
}

void setup_terminal(void) {
    // Use fputs instead of printf for simple string output
    fputs(HIDE_CURSOR, stdout);
    // Flush to ensure cursor is hidden immediately
    fflush(stdout);
}

void format_size(unsigned long bytes, char *result, size_t result_size, const ProgramOptions *opts) {
    if (!result || result_size < 1) {
        return;  // Early return for invalid parameters
    }

    double size = (double)bytes;  // Explicit cast to double
    int unit = 0;

    // Convert to appropriate unit
    while (size >= KILOBYTE && unit < UNIT_COUNT - 1) {
        size /= KILOBYTE;
        unit++;
    }

    // Format with different precisions based on unit size
    int precision;
    if (unit == 0) {
        precision = 0;
    } else if (unit == 1) {
        precision = 1;
    } else {
        precision = 2;
    }

    // Use snprintf for safe string formatting
    int written = snprintf(result, result_size, "%.*f %s", precision, size, UNITS[unit]);
    
    // Ensure null termination in case of truncation
    if (written >= result_size) {
        result[result_size - 1] = '\0';
    }
}