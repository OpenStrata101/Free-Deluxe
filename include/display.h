#ifndef DISPLAY_H
#define DISPLAY_H

#include "memory.h"
#include "args.h"

void display_memory(MemoryInfo *info, ProgramOptions *opts);
void display_memory_deluxe(MemoryInfo *info, ProgramOptions *opts);
// Remove the declaration of format_size from here
void show_loading_animation(void);

#endif /* DISPLAY_H */