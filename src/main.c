// src/main.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "args.h"
#include "memory.h"
#include "display.h"
#include "utils.h"

int main(int argc, char **argv) {
    ProgramOptions opts = parse_args(argc, argv);
    
    // Show installation animation
    if (opts.display_mode == 1) {
        show_loading_animation();
        setup_terminal();
        atexit(cleanup);
    }
    
    // Set default update interval for deluxe mode
    if (opts.display_mode == 1 && opts.repeat_seconds == 0) {
        opts.repeat_seconds = 1;  // Update every second by default in deluxe mode
    }
    
    int count = 0;
    while (1) {
        MemoryInfo info = get_memory_info();
        
        if (opts.display_mode == 1) {
            display_memory_deluxe(&info, &opts);
        } else {
            display_memory(&info, &opts);
        }
        
        if (opts.repeat_seconds > 0) {
            usleep(opts.repeat_seconds * 1000000);
            if (opts.repeat_count > 0 && ++count >= opts.repeat_count) {
                break;
            }
        } else {
            break;
        }
    }
    
    return 0;
}