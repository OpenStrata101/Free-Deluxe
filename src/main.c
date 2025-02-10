// src/main.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "args.h"
#include "memory.h"
#include "display.h"
#include "utils.h"

#define DELUXE_MODE 1
#define DEFAULT_UPDATE_INTERVAL 1

int main(int argc, char **argv) {
    ProgramOptions opts = parse_args(argc, argv);

    // Show installation animation
    if (opts.display_mode == DELUXE_MODE) {
        show_loading_animation();
        setup_terminal();
        atexit(cleanup);
    }

    // Set default update interval for deluxe mode
    if (opts.display_mode == DELUXE_MODE && opts.repeat_seconds == 0) {
        opts.repeat_seconds = DEFAULT_UPDATE_INTERVAL;  // Update every second by default in deluxe mode
    }

    int count = 0;
    const int is_deluxe_mode = (opts.display_mode == DELUXE_MODE);

    while (count < opts.repeat_count || opts.repeat_count == 0) {
        MemoryInfo info = get_memory_info();

        if (is_deluxe_mode) {
            display_memory_deluxe(&info, &opts);
        } else {
            display_memory(&info, &opts);
        }

        if (opts.repeat_seconds > 0) {
            usleep(opts.repeat_seconds * 1000000);
            count++;
        } else {
            break;
        }
    }

    return 0;
}
