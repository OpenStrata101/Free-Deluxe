// src/memory.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include "memory.h"

#define MEMINFO_PATH "/proc/meminfo"
#define LINE_BUFFER_SIZE 256
#define KB_TO_BYTES 1024

unsigned long get_cached_memory() {
    FILE *fp = fopen(MEMINFO_PATH, "r");
    if (fp == NULL) {
        perror("Failed to open /proc/meminfo");
        return 0;
    }

    char line[LINE_BUFFER_SIZE];
    unsigned long cached = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "Cached:", 7) == 0) {
            if (sscanf(line, "Cached: %lu", &cached) == 1) {
                cached *= KB_TO_BYTES; // Convert from KB to bytes
            }
            break;
        }
    }

    fclose(fp);
    return cached;
}

MemoryInfo get_memory_info(void) {
    struct sysinfo si;
    if (sysinfo(&si) != 0) {
        perror("sysinfo");
        exit(EXIT_FAILURE);
    }

    MemoryInfo info = {
        .total = si.totalram,
        .free = si.freeram,
        .shared = si.sharedram,
        .buffers = si.bufferram,
        .cached = get_cached_memory(),
        .swap_total = si.totalswap,
        .swap_free = si.freeswap
    };

    info.used = info.total - info.free - info.buffers - info.cached;
    info.available = info.free + info.buffers + info.cached;
    info.swap_used = info.swap_total - info.swap_free;

    return info;
}
