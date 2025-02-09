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