#include <stdio.h>
#include <string.h>
#include "utils.h"
#include "args.h"

#define HIDE_CURSOR "\033[?25l"
#define SHOW_CURSOR "\033[?25h"
#define UNIT_COUNT 5
#define KILOBYTE 1024.0
#define MEGABYTE (KILOBYTE * 1024.0)
#define GIGABYTE (MEGABYTE * 1024.0)
#define TERABYTE (GIGABYTE * 1024.0)

// Unit strings for binary and SI units
static const char* const BINARY_UNITS[] = {"B", "KiB", "MiB", "GiB", "TiB"};
static const char* const SI_UNITS[] = {"B", "KB", "MB", "GB", "TB"};

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
        *result = '\0';
        return;  // Early return for invalid parameters
    }

    double size = (double)bytes;
    int unit = 0;
    const double divisor = opts->si_units ? 1000.0 : 1024.0;
    const char* const *units = opts->si_units ? SI_UNITS : BINARY_UNITS;

    // If a specific unit is selected, convert to that
    if (opts->unit > 0) {
        for (int i = 0; i < opts->unit - 1; i++) {
            size /= divisor;
        }
        unit = opts->unit - 1;
    } else {
        // Auto-scale to appropriate unit
        while (size >= divisor && unit < UNIT_COUNT - 1) {
            size /= divisor;
            unit++;
        }
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
    int written = snprintf(result, result_size, "%.*f %s", precision, size, units[unit]);
    
    // Ensure null termination in case of truncation
    if (written >= (int)result_size) {
        result[result_size - 1] = '\0';
    }
}