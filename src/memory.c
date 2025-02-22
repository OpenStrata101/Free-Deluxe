// src/memory.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/sysinfo.h>
#include <limits.h>
#include "memory.h"

#define MEMINFO_PATH "/proc/meminfo"
#define LINE_BUFFER_SIZE 256
#define KB_TO_BYTES 1024UL

// Structure to hold raw memory values from /proc/meminfo
typedef struct {
    unsigned long mem_total;
    unsigned long mem_free;
    unsigned long mem_available;
    unsigned long buffers;
    unsigned long cached;
    unsigned long swap_cached;
    unsigned long active;
    unsigned long inactive;
    unsigned long swap_total;
    unsigned long swap_free;
} MemInfoRaw;

// Safe multiplication checking for overflow
static int safe_multiply(unsigned long a, unsigned long b, unsigned long *result) {
    if (a > 0 && b > ULONG_MAX / a) {
        return -1; // Overflow would occur
    }
    *result = a * b;
    return 0;
}

// Parse a memory value line from /proc/meminfo
static int parse_memory_line(const char *line, unsigned long *value) {
    char name[32];
    unsigned long kb_value;
    
    // Parse line format "Name: value kB"
    if (sscanf(line, "%31[^:]: %lu", name, &kb_value) != 2) {
        return -1;
    }
    
    // Convert KB to bytes with overflow check
    if (safe_multiply(kb_value, KB_TO_BYTES, value) != 0) {
        return -1;
    }
    
    return 0;
}

// Read memory information from /proc/meminfo
static int read_proc_meminfo(MemInfoRaw *info) {
    FILE *fp = fopen(MEMINFO_PATH, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error opening %s: %s\n", MEMINFO_PATH, strerror(errno));
        return -1;
    }

    char line[LINE_BUFFER_SIZE];
    int found_count = 0;
    const int required_fields = 10; // Number of fields we need to find

    while (fgets(line, sizeof(line), fp)) {
        unsigned long *target = NULL;

        // Match the line with the corresponding field
        if (strncmp(line, "MemTotal:", 9) == 0)
            target = &info->mem_total;
        else if (strncmp(line, "MemFree:", 8) == 0)
            target = &info->mem_free;
        else if (strncmp(line, "MemAvailable:", 12) == 0)
            target = &info->mem_available;
        else if (strncmp(line, "Buffers:", 8) == 0)
            target = &info->buffers;
        else if (strncmp(line, "Cached:", 7) == 0 && !strstr(line, "SwapCached:"))
            target = &info->cached;
        else if (strncmp(line, "SwapCached:", 11) == 0)
            target = &info->swap_cached;
        else if (strncmp(line, "Active:", 7) == 0)
            target = &info->active;
        else if (strncmp(line, "Inactive:", 9) == 0)
            target = &info->inactive;
        else if (strncmp(line, "SwapTotal:", 10) == 0)
            target = &info->swap_total;
        else if (strncmp(line, "SwapFree:", 9) == 0)
            target = &info->swap_free;

        if (target) {
            if (parse_memory_line(line, target) == 0) {
                found_count++;
            } else {
                fprintf(stderr, "Error parsing line: %s", line);
            }
        }
    }

    fclose(fp);

    // Check if we found all required fields
    if (found_count < required_fields) {
        fprintf(stderr, "Warning: Only found %d of %d required memory fields\n",
                found_count, required_fields);
        return -1;
    }

    return 0;
}

// Calculate derived memory values safely
static int calculate_memory_values(const MemInfoRaw *raw, MemoryInfo *info) {
    // Copy direct values
    info->total = raw->mem_total;
    info->free = raw->mem_free;
    info->available = raw->mem_available;
    info->buffers = raw->buffers;
    info->cached = raw->cached;
    info->swap_total = raw->swap_total;
    info->swap_free = raw->swap_free;

    // Calculate used memory (total - free - buffers - cached)
    unsigned long total_deductions = raw->mem_free + raw->buffers + raw->cached;
    if (total_deductions > raw->mem_total) {
        info->used = 0; // Prevent underflow
    } else {
        info->used = raw->mem_total - total_deductions;
    }

    // Calculate swap used
    if (raw->swap_free > raw->swap_total) {
        info->swap_used = 0; // Prevent underflow
    } else {
        info->swap_used = raw->swap_total - raw->swap_free;
    }

    // Set shared memory from active/inactive sum
    info->shared = raw->active + raw->inactive;

    return 0;
}

// Fallback to sysinfo if /proc/meminfo fails
static int get_memory_from_sysinfo(MemoryInfo *info) {
    struct sysinfo si;
    if (sysinfo(&si) != 0) {
        fprintf(stderr, "Error getting system info: %s\n", strerror(errno));
        return -1;
    }

    // Convert to bytes (sysinfo values are in bytes)
    info->total = si.totalram;
    info->free = si.freeram;
    info->shared = si.sharedram;
    info->buffers = si.bufferram;
    info->cached = 0; // Not available through sysinfo
    info->available = si.freeram + si.bufferram; // Best estimate
    info->used = info->total - info->free - info->buffers;
    info->swap_total = si.totalswap;
    info->swap_free = si.freeswap;
    info->swap_used = si.totalswap - si.freeswap;

    return 0;
}

MemoryInfo get_memory_info(void) {
    MemoryInfo info = {0}; // Initialize to zero
    MemInfoRaw raw = {0};

    // Try /proc/meminfo first
    if (read_proc_meminfo(&raw) == 0) {
        if (calculate_memory_values(&raw, &info) != 0) {
            // If calculation fails, try sysinfo as fallback
            fprintf(stderr, "Warning: Failed to calculate memory values, using fallback\n");
            get_memory_from_sysinfo(&info);
        }
    } else {
        // Fallback to sysinfo
        fprintf(stderr, "Warning: Failed to read /proc/meminfo, using fallback\n");
        get_memory_from_sysinfo(&info);
    }

    return info;
}