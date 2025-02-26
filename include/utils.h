#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>  // For size_t
#include "args.h"

void cleanup(void);
void setup_terminal(void);
void format_size(unsigned long bytes, char *result, size_t result_size, const ProgramOptions *opts);

#endif /* UTILS_H */