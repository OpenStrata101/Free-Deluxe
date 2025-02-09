// include/memory.h
#ifndef MEMORY_H
#define MEMORY_H

typedef struct {
    unsigned long total;
    unsigned long used;
    unsigned long free;
    unsigned long shared;
    unsigned long buffers;
    unsigned long cached;
    unsigned long available;
    unsigned long swap_total;
    unsigned long swap_used;
    unsigned long swap_free;
} MemoryInfo;

MemoryInfo get_memory_info(void);

#endif