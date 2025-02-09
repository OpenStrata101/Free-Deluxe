#ifndef DISPLAY_H
#define DISPLAY_H

#include "memory.h"
#include "args.h"

void display_memory(MemoryInfo *info, ProgramOptions *opts);
void display_memory_deluxe(MemoryInfo *info, ProgramOptions *opts);
void format_size(unsigned long bytes, char *result, ProgramOptions *opts);
void show_loading_animation(void);  // Added this declaration

#endif