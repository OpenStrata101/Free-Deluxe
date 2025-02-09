// src/display.c
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "display.h"
#include "memory.h"
#include "args.h"

#define CLEAR_SCREEN "\033[H\033[J"
#define CLEAR_LINE "\033[K"
#define CURSOR_UP "\033[A"

// ANSI color codes
#define COLOR_RESET "\033[0m"
#define COLOR_BLUE "\033[34m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_CYAN "\033[36m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_WHITE "\033[37m"
#define COLOR_BOLD "\033[1m"
#define COLOR_DIM "\033[2m"

// Nerd Font icons
#define ICON_RAM "  "       // nf-md-memory
#define ICON_CPU "󰘚 "       // nf-md-cpu_64_bit
#define ICON_USED "󰋊 "      // nf-md-folder
#define ICON_FREE "󰉒 "      // nf-md-folder_open
#define ICON_CACHE "󰆦 "     // nf-md-cached
#define ICON_SWAP "󰒋 "      // nf-md-swap_horizontal
#define ICON_CHART "󰕋 "     // nf-md-chart_box

void show_spinner(int frame) {
    const char* spinner[] = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};
    printf("%s%s%s", COLOR_CYAN, spinner[frame % 10], COLOR_RESET);
}

void draw_memory_bar(double percentage, int width, const char* color) {
    int filled = (int)(percentage * width / 100);
    printf("%s%s", color, COLOR_BOLD);
    printf("[");
    
    // Blocks: '▁', '▂', '▃', '▄', '▅', '▆', '▇', '█'
    for (int i = 0; i < width; i++) {
        if (i < filled) {
            printf("▇");
        } else {
            printf("▁");
        }
    }
    printf("] %.1f%%%s", percentage, COLOR_RESET);
}

void display_memory_deluxe(MemoryInfo *info, ProgramOptions *opts) {
    static int frame = 0;
    char formatted[10][20];
    
    // Format all sizes
    format_size(info->total, formatted[0], opts);
    format_size(info->used, formatted[1], opts);
    format_size(info->free, formatted[2], opts);
    format_size(info->shared, formatted[3], opts);
    format_size(info->cached, formatted[4], opts);
    format_size(info->available, formatted[5], opts);
    format_size(info->swap_total, formatted[6], opts);
    format_size(info->swap_used, formatted[7], opts);
    
    double mem_used_percent = (double)info->used * 100 / info->total;
    double swap_used_percent = info->swap_total ? 
        ((double)info->swap_used * 100 / info->swap_total) : 0;

    printf(CLEAR_SCREEN);
    printf("\n");
    
    // Minimal header
    printf("%s%s System Memory Monitor %s ", COLOR_CYAN, ICON_CPU, COLOR_RESET);
    show_spinner(frame++);
    printf("\n\n");

    // RAM section
    printf("%s%s RAM%s  %s%s%s\n", COLOR_BLUE, ICON_RAM, COLOR_RESET, 
           COLOR_BOLD, formatted[0], COLOR_RESET);
    
    printf("   ");
    draw_memory_bar(mem_used_percent, 30, COLOR_BLUE);
    printf("\n\n");

    // Main stats in a compact format
    printf("%s%s Used%s    %-10s", COLOR_MAGENTA, ICON_USED, COLOR_RESET, formatted[1]);
    printf("   %s%s Cache%s  %-10s\n", COLOR_YELLOW, ICON_CACHE, COLOR_RESET, formatted[4]);
    
    printf("%s%s Free%s    %-10s", COLOR_GREEN, ICON_FREE, COLOR_RESET, formatted[2]);
    printf("   %s%s Avail%s  %-10s\n", COLOR_CYAN, ICON_CHART, COLOR_RESET, formatted[5]);

    // Swap section if enabled
    if (info->swap_total > 0) {
        printf("\n%s%s SWAP%s\n", COLOR_YELLOW, ICON_SWAP, COLOR_RESET);
        printf("   ");
        draw_memory_bar(swap_used_percent, 30, COLOR_YELLOW);
        printf("\n");
    }

    printf("\n");
}

// Add a loading animation
void show_loading_animation() {
    const char* frames[] = {
        "⠋ Installing", "⠙ Installing", "⠹ Installing",
        "⠸ Installing", "⠼ Installing", "⠴ Installing",
        "⠦ Installing", "⠧ Installing", "⠇ Installing", "⠏ Installing"
    };
    
    for (int i = 0; i < 20; i++) {
        printf("\r%s%s%s", COLOR_CYAN, frames[i % 10], COLOR_RESET);
        fflush(stdout);
        usleep(100000);
    }
    printf("\r%s✓ Installation complete!%s\n", COLOR_GREEN, COLOR_RESET);
}