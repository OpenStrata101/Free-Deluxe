// src/memory.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include "memory.h"

unsigned long get_cached_memory() {
    FILE *fp;
    char line[256];
    unsigned long cached = 0;

    fp = fopen("/proc/meminfo", "r");
    if (fp == NULL) {
        return 0;
    }

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "Cached:", 7) == 0) {
            sscanf(line, "Cached: %lu", &cached);
            cached *= 1024; // Convert from KB to bytes
            break;
        }
    }

    fclose(fp);
    return cached;
}

MemoryInfo get_memory_info(void) {
    struct sysinfo si;
    MemoryInfo info = {0};

    if (sysinfo(&si) != 0) {
        perror("sysinfo");
        exit(1);
    }

    info.cached = get_cached_memory();
    info.total = si.totalram;
    info.free = si.freeram;
    info.shared = si.sharedram;
    info.buffers = si.bufferram;
    info.used = info.total - info.free - info.buffers - info.cached;
    info.available = info.free + info.buffers + info.cached;
    
    info.swap_total = si.totalswap;
    info.swap_free = si.freeswap;
    info.swap_used = si.totalswap - si.freeswap;

    return info;
}