#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "../include/display.h"
#include "../include/memory.h"
#include "../include/args.h"
#include "../include/common.h"
#include "../include/utils.h" 

// Spinner frames array
static const char* SPINNER_FRAMES[] = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};
#define SPINNER_FRAME_COUNT (sizeof(SPINNER_FRAMES) / sizeof(SPINNER_FRAMES[0]))

// Unit strings for formatting
static const char* UNITS[] = {"B", "KiB", "MiB", "GiB", "TiB"};
static const char* SI_UNITS[] = {"B", "KB", "MB", "GB", "TB"};

static void show_spinner(int frame) {
    printf("%s%s%s", COLOR_CYAN, SPINNER_FRAMES[frame % SPINNER_FRAME_COUNT], COLOR_RESET);
}

static void draw_memory_bar(double percentage, int width, const char* color) {
    // Pre-allocate buffer for the bar
    static char bar_buffer[256];
    int offset = 0;

    // Ensure percentage is within bounds
    if (percentage < 0) percentage = 0;
    if (percentage > 100) percentage = 100;

    int filled = (int)(percentage * width / 100);
    if (filled > width) filled = width;

    // Start building the bar
    offset += snprintf(bar_buffer + offset, sizeof(bar_buffer) - offset,
                      "%s%s[", color, COLOR_BOLD);

    // Add the bar segments
    for (int i = 0; i < width; i++) {
        offset += snprintf(bar_buffer + offset, sizeof(bar_buffer) - offset,
                         "%s", (i < filled) ? "▇" : "▁");
    }

    // Add the percentage and reset color
    offset += snprintf(bar_buffer + offset, sizeof(bar_buffer) - offset,
                      "] %.1f%%%s", percentage, COLOR_RESET);

    // Write the complete bar at once
    printf("%s", bar_buffer);
}

void display_memory(MemoryInfo *info, ProgramOptions *opts) {
    char total[FORMAT_BUFFER_SIZE], used[FORMAT_BUFFER_SIZE], 
         free[FORMAT_BUFFER_SIZE], available[FORMAT_BUFFER_SIZE], 
         cached[FORMAT_BUFFER_SIZE], swap_total[FORMAT_BUFFER_SIZE], 
         swap_used[FORMAT_BUFFER_SIZE], buffers[FORMAT_BUFFER_SIZE];
    
    // Format all sizes at once
    format_size(info->total, total, FORMAT_BUFFER_SIZE, opts);
    format_size(info->used, used, FORMAT_BUFFER_SIZE, opts);
    format_size(info->free, free, FORMAT_BUFFER_SIZE, opts);
    format_size(info->available, available, FORMAT_BUFFER_SIZE, opts);
    format_size(info->cached, cached, FORMAT_BUFFER_SIZE, opts);
    format_size(info->buffers, buffers, FORMAT_BUFFER_SIZE, opts);
    format_size(info->swap_total, swap_total, FORMAT_BUFFER_SIZE, opts);
    format_size(info->swap_used, swap_used, FORMAT_BUFFER_SIZE, opts);
    
    // Build output using a single buffer
    static char output_buffer[MAX_BUFFER_SIZE];
    int offset = 0;

    // Basic statistics
    offset += snprintf(output_buffer + offset, sizeof(output_buffer) - offset,
        "\nMemory Statistics:\n"
        "----------------\n"
        "Total Memory:     %s\n"
        "Used Memory:      %s\n"
        "Free Memory:      %s\n"
        "Available Memory: %s\n"
        "Buffers:          %s\n"
        "Cached Memory:    %s\n",
        total, used, free, available, buffers, cached);

    // Add swap information if available
    if (info->swap_total > 0) {
        offset += snprintf(output_buffer + offset, sizeof(output_buffer) - offset,
            "\nSwap Usage:\n"
            "-----------\n"
            "Swap Total:    %s\n"
            "Swap Used:     %s\n",
            swap_total, swap_used);
    }

    // Write the complete buffer at once
    printf("%s", output_buffer);
}

void display_memory_deluxe(MemoryInfo *info, ProgramOptions *opts) {
    static char buffer[MAX_BUFFER_SIZE];
    int offset = 0;
    static int frame = 0;

    // Format all memory values at once
    char formatted[8][FORMAT_BUFFER_SIZE];
    format_size(info->total, formatted[0], FORMAT_BUFFER_SIZE, opts);
    format_size(info->used, formatted[1], FORMAT_BUFFER_SIZE, opts);
    format_size(info->free, formatted[2], FORMAT_BUFFER_SIZE, opts);
    format_size(info->available, formatted[3], FORMAT_BUFFER_SIZE, opts);
    format_size(info->cached, formatted[4], FORMAT_BUFFER_SIZE, opts);
    format_size(info->swap_total, formatted[5], FORMAT_BUFFER_SIZE, opts);
    format_size(info->swap_used, formatted[6], FORMAT_BUFFER_SIZE, opts);
    format_size(info->buffers, formatted[7], FORMAT_BUFFER_SIZE, opts);

    // Calculate percentages
    double mem_used_percent = (double)info->used * 100 / info->total;
    double swap_used_percent = info->swap_total ? 
        ((double)info->swap_used * 100 / info->swap_total) : 0;

    // Clear screen and build header
    offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                      "%s\n%s%s System Memory Monitor %s ",
                      CLEAR_SCREEN, COLOR_CYAN, ICON_CPU, COLOR_RESET);

    // Add spinner frame
    offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                      "%s%s%s\n\n",
                      COLOR_CYAN, SPINNER_FRAMES[frame++ % SPINNER_FRAME_COUNT], COLOR_RESET);

    // RAM section
    offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                      "%s%s RAM%s  %s%s%s\n   ",
                      COLOR_BLUE, ICON_RAM, COLOR_RESET,
                      COLOR_BOLD, formatted[0], COLOR_RESET);

    // Write buffer so far to allow draw_memory_bar to write directly
    printf("%s", buffer);
    offset = 0;  // Reset buffer offset

    // Draw memory usage bar
    draw_memory_bar(mem_used_percent, 30, COLOR_BLUE);
    printf("\n\n");

    // Main stats
    offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                      "%s%s Used%s    %-12s   %s%s Cache%s  %-12s\n"
                      "%s%s Free%s    %-12s   %s%s Avail%s  %-12s\n"
                      "%s%s Buffers%s %-12s\n",
                      COLOR_MAGENTA, ICON_USED, COLOR_RESET, formatted[1],
                      COLOR_YELLOW, ICON_CACHE, COLOR_RESET, formatted[4],
                      COLOR_GREEN, ICON_FREE, COLOR_RESET, formatted[2],
                      COLOR_CYAN, ICON_CHART, COLOR_RESET, formatted[3],
                      COLOR_BLUE, ICON_RAM, COLOR_RESET, formatted[7]);

    // Swap section if enabled
    if (info->swap_total > 0) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                          "\n%s%s SWAP%s\n   ",
                          COLOR_YELLOW, ICON_SWAP, COLOR_RESET);
        
        // Write accumulated buffer
        printf("%s", buffer);
        
        // Draw swap usage bar
        draw_memory_bar(swap_used_percent, 30, COLOR_YELLOW);
        printf("\n");
    } else {
        // Write accumulated buffer
        printf("%s", buffer);
    }

    printf("\n");  // Final newline
}

void show_loading_animation(void) {
    static const char* frames[] = {
        "⠋ Installing", "⠙ Installing", "⠹ Installing",
        "⠸ Installing", "⠼ Installing", "⠴ Installing",
        "⠦ Installing", "⠧ Installing", "⠇ Installing", "⠏ Installing"
    };
    
    printf(HIDE_CURSOR);
    for (int i = 0; i < 20; i++) {
        printf("\r%s%s%s", COLOR_CYAN, frames[i % 10], COLOR_RESET);
        fflush(stdout);
        usleep(100000);  // 100ms delay
    }
    printf("\r%s✓ Installation complete!%s\n", COLOR_GREEN, COLOR_RESET);
    printf(SHOW_CURSOR);
}