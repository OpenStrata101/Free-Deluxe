// src/main.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include "args.h"
#include "memory.h"
#include "display.h"
#include "utils.h"

#define DELUXE_MODE 1
#define DEFAULT_UPDATE_INTERVAL 1
#define MAX_UPDATE_INTERVAL 3600
#define MAX_REPEAT_COUNT 1000
#define SHOW_CURSOR "\033[?25h"  // Added this definition

// Global flag for signal handling
static volatile sig_atomic_t keep_running = 1;

// Signal handler prototype
static void signal_handler(int signum);
static void cleanup_handler(void);

// Initialize program state
static void initialize_program(ProgramOptions *opts) {
    // Setup signal handlers
    struct sigaction sa = {
        .sa_handler = signal_handler,
        .sa_flags = SA_RESTART,
    };
    sigemptyset(&sa.sa_mask);
    
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        fprintf(stderr, "Failed to set SIGINT handler: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        fprintf(stderr, "Failed to set SIGTERM handler: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Register cleanup handler
    if (atexit(cleanup_handler) != 0) {
        fprintf(stderr, "Failed to register cleanup handler\n");
        exit(EXIT_FAILURE);
    }

    // Setup deluxe mode if enabled
    if (opts->display_mode == DELUXE_MODE) {
        show_loading_animation();
        setup_terminal();
        
        // Set default update interval for deluxe mode if not specified
        if (opts->repeat_seconds == 0) {
            opts->repeat_seconds = DEFAULT_UPDATE_INTERVAL;
        }
    }
}

// Validate and adjust program options
static void validate_options(ProgramOptions *opts) {
    // Validate update interval
    if (opts->repeat_seconds < 0) {
        fprintf(stderr, "Warning: Negative update interval corrected to 0\n");
        opts->repeat_seconds = 0;
    } else if (opts->repeat_seconds > MAX_UPDATE_INTERVAL) {
        fprintf(stderr, "Warning: Update interval limited to %d seconds\n", 
                MAX_UPDATE_INTERVAL);
        opts->repeat_seconds = MAX_UPDATE_INTERVAL;
    }

    // Validate repeat count
    if (opts->repeat_count < 0) {
        fprintf(stderr, "Warning: Negative repeat count corrected to 0 (infinite)\n");
        opts->repeat_count = 0;
    } else if (opts->repeat_count > MAX_REPEAT_COUNT) {
        fprintf(stderr, "Warning: Repeat count limited to %d\n", MAX_REPEAT_COUNT);
        opts->repeat_count = MAX_REPEAT_COUNT;
    }
}

// Main display loop
static void display_loop(ProgramOptions *opts) {
    int count = 0;
    const int is_deluxe_mode = (opts->display_mode == DELUXE_MODE);
    
    while (keep_running && (count < opts->repeat_count || opts->repeat_count == 0)) {
        MemoryInfo info = get_memory_info();
        
        // Check if memory info retrieval was successful
        if (info.total == 0) {
            fprintf(stderr, "Error: Failed to retrieve memory information\n");
            break;
        }

        // Display memory information based on mode
        if (is_deluxe_mode) {
            display_memory_deluxe(&info, opts);
        } else {
            display_memory(&info, opts);
        }

        // Handle update interval
        if (opts->repeat_seconds > 0) {
            // Use sleep() for longer intervals, usleep() for sub-second precision
            if (opts->repeat_seconds >= 1) {
                sleep(opts->repeat_seconds);
            } else {
                usleep(opts->repeat_seconds * 1000000);
            }
            count++;
        } else {
            break;
        }
        
        // Check for errors after each iteration
        if (ferror(stdout)) {
            fprintf(stderr, "Error: Failed to write to stdout\n");
            break;
        }
    }
}

// Signal handler implementation
static void signal_handler(int signum) {
    (void)signum;  // Explicitly mark parameter as unused
    keep_running = 0;
    
    // Re-enable cursor immediately if interrupted
    printf(SHOW_CURSOR);
    fflush(stdout);
}

// Cleanup handler implementation
static void cleanup_handler(void) {
    cleanup();  // Call the original cleanup function
    fflush(stdout);
    fflush(stderr);
}

int main(int argc, char **argv) {
    // Parse command line arguments
    ProgramOptions opts = parse_args(argc, argv);
    
    // Initialize program (signal handlers, terminal setup)
    initialize_program(&opts);
    
    // Validate and adjust options
    validate_options(&opts);
    
    // Enter main display loop
    display_loop(&opts);
    
    return EXIT_SUCCESS;
}