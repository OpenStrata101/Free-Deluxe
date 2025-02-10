#ifndef UTILS_H
#define UTILS_H

#define VERSION "1.0.0"
#define PROGRAM_NAME "freed"

#include "args.h"  // Add this to get ProgramOptions type

void cleanup(void);
void setup_terminal(void);
void format_size(unsigned long bytes, char *result, ProgramOptions *opts);

#endif