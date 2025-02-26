#ifndef COMMON_H
#define COMMON_H

#define PROGRAM_NAME "freed"
#define VERSION "1.0.0"

// ANSI escape sequences
#define CLEAR_SCREEN "\033[H\033[J"
#define CLEAR_LINE "\033[K"
#define CURSOR_UP "\033[A"
#define HIDE_CURSOR "\033[?25l"
#define SHOW_CURSOR "\033[?25h"

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

// Buffer sizes
#define MAX_BUFFER_SIZE 4096
#define FORMAT_BUFFER_SIZE 128

#endif /* COMMON_H */