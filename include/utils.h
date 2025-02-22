// src/utils.h
#ifndef UTILS_H
#define UTILS_H

typedef struct {
    // Add your ProgramOptions structure fields here
} ProgramOptions;

void cleanup(void);
void setup_terminal(void);
void format_size(unsigned long bytes, char *result, size_t result_size, const ProgramOptions *opts);

#endif