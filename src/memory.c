// src/memory.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/sysinfo.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>     // For offsetof macro
#include <time.h>       // For nanosleep
#include "memory.h"

#define MEMINFO_PATH "/proc/meminfo"
#define LINE_BUFFER_SIZE 256
#define KB_TO_BYTES 1024UL
#define MAX_RETRIES 3
#define FIELD_NAME_MAX 32

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

// Field mapping structure for more maintainable code
typedef struct {
    const char *name;
    size_t offset;
    size_t name_len;
} MemField;

// Safe multiplication checking for overflow
static bool safe_multiply(unsigned long a, unsigned long b, unsigned long *result) {
    if (a > 0 && b > ULONG_MAX / a) {
        return false; // Overflow would occur
    }
    *result = a * b;
    return true;
}

// Parse a memory value line from /proc/meminfo
static bool parse_memory_line(const char *line, unsigned long *value) {
    char name[FIELD_NAME_MAX];
    unsigned long kb_value;
    
    // Parse line format "Name: value kB"
    if (sscanf(line, "%31[^:]: %lu", name, &kb_value) != 2) {
        return false;
    }
    
    // Convert KB to bytes with overflow check
    return safe_multiply(kb_value, KB_TO_BYTES, value);
}

// Read memory information from /proc/meminfo
static bool read_proc_meminfo(MemInfoRaw *info) {
    FILE *fp = NULL;
    int retries = 0;
    bool success = false;
    
    // Initialize field mappings for cleaner code
    // Each entry contains: the field name, its offset in the structure, and the name length
    const MemField fields[] = {
        {"MemTotal:",      offsetof(MemInfoRaw, mem_total),     9},
        {"MemFree:",       offsetof(MemInfoRaw, mem_free),      8},
        {"MemAvailable:",  offsetof(MemInfoRaw, mem_available), 13},
        {"Buffers:",       offsetof(MemInfoRaw, buffers),       8},
        {"Cached:",        offsetof(MemInfoRaw, cached),        7},
        {"SwapCached:",    offsetof(MemInfoRaw, swap_cached),   11},
        {"Active:",        offsetof(MemInfoRaw, active),        7},
        {"Inactive:",      offsetof(MemInfoRaw, inactive),      9},
        {"SwapTotal:",     offsetof(MemInfoRaw, swap_total),    10},
        {"SwapFree:",      offsetof(MemInfoRaw, swap_free),     9}
    };
    const int num_fields = sizeof(fields) / sizeof(MemField);
    
    // Try opening the file with retries for resilience
    while (retries < MAX_RETRIES && fp == NULL) {
        fp = fopen(MEMINFO_PATH, "r");
        if (fp == NULL) {
            retries++;
            // Small delay before retry
            struct timespec ts = {0, 100000000}; // 100ms
            nanosleep(&ts, NULL);
        }
    }
    
    if (fp == NULL) {
        fprintf(stderr, "Error opening %s after %d attempts: %s\n", 
                MEMINFO_PATH, MAX_RETRIES, strerror(errno));
        return false;
    }

    char line[LINE_BUFFER_SIZE];
    int found_count = 0;

    while (fgets(line, sizeof(line), fp)) {
        // Skip empty lines
        if (line[0] == '\n' || line[0] == '\0') continue;
        
        bool line_matched = false;
        
        // Check each field
        for (int i = 0; i < num_fields && !line_matched; i++) {
            if (strncmp(line, fields[i].name, fields[i].name_len) == 0) {
                // Special case for "Cached:" to avoid matching "SwapCached:"
                if (strcmp(fields[i].name, "Cached:") == 0 && 
                    strstr(line, "SwapCached:") != NULL) {
                    continue;
                }
                
                // Get the address of the target field using the offset
                unsigned long *target = (unsigned long*)((char*)info + fields[i].offset);
                
                if (parse_memory_line(line, target)) {
                    found_count++;
                    line_matched = true;
                } else {
                    fprintf(stderr, "Error parsing line: %s", line);
                }
            }
        }
    }

    fclose(fp);

    // Check if we found all required fields
    success = (found_count == num_fields);
    if (!success) {
        fprintf(stderr, "Warning: Only found %d of %d required memory fields\n",
                found_count, num_fields);
    }

    return success;
}

// Calculate derived memory values safely
static bool calculate_memory_values(const MemInfoRaw *raw, MemoryInfo *info) {
    // Copy direct values
    info->total = raw->mem_total;
    info->free = raw->mem_free;
    info->available = raw->mem_available;
    info->buffers = raw->buffers;
    info->cached = raw->cached;
    info->swap_total = raw->swap_total;
    info->swap_free = raw->swap_free;

    // Calculate used memory (total - free - buffers - cached)
    unsigned long total_deductions = 0;
    
    // Check for overflow in addition
    if (raw->mem_free > ULONG_MAX - raw->buffers ||
        raw->mem_free + raw->buffers > ULONG_MAX - raw->cached) {
        fprintf(stderr, "Warning: Overflow detected in memory calculations\n");
        info->used = 0;
    } else {
        total_deductions = raw->mem_free + raw->buffers + raw->cached;
        info->used = (total_deductions > raw->mem_total) ? 0 : raw->mem_total - total_deductions;
    }

    // Calculate swap used
    info->swap_used = (raw->swap_free > raw->swap_total) ? 0 : raw->swap_total - raw->swap_free;

    // Check for overflow in active + inactive
    if (raw->active > ULONG_MAX - raw->inactive) {
        fprintf(stderr, "Warning: Overflow detected in shared memory calculation\n");
        info->shared = 0;
    } else {
        info->shared = raw->active + raw->inactive;
    }

    return true;
}

// Fallback to sysinfo if /proc/meminfo fails
static bool get_memory_from_sysinfo(MemoryInfo *info) {
    struct sysinfo si;
    if (sysinfo(&si) != 0) {
        fprintf(stderr, "Error getting system info: %s\n", strerror(errno));
        return false;
    }

    // Check for potential overflow from unit conversion
    if (si.mem_unit > 1 && si.totalram > ULONG_MAX / si.mem_unit) {
        fprintf(stderr, "Warning: Potential overflow in memory unit conversion\n");
        // Use raw values without conversion in this case
        info->total = si.totalram;
        info->free = si.freeram;
        info->shared = si.sharedram;
        info->buffers = si.bufferram;
    } else {
        // Convert to bytes (multiply by mem_unit)
        info->total = si.totalram * si.mem_unit;
        info->free = si.freeram * si.mem_unit;
        info->shared = si.sharedram * si.mem_unit;
        info->buffers = si.bufferram * si.mem_unit;
    }
    
    info->cached = 0; // Not available through sysinfo
    
    // Safer calculation of available memory
    if (si.freeram > ULONG_MAX - si.bufferram) {
        // Handle overflow
        info->available = info->free; // Just use free as fallback
    } else {
        info->available = info->free + info->buffers;
    }
    
    // Calculate used memory
    if (info->free > info->total) {
        info->used = 0; // Handle inconsistent values
    } else if (info->buffers > info->total - info->free) {
        info->used = 0; // Handle overflow
    } else {
        info->used = info->total - info->free - info->buffers;
    }
    
    // Handle swap calculations with overflow checks
    if (si.mem_unit > 1 && (si.totalswap > ULONG_MAX / si.mem_unit || 
                            si.freeswap > ULONG_MAX / si.mem_unit)) {
        info->swap_total = si.totalswap;
        info->swap_free = si.freeswap;
    } else {
        info->swap_total = si.totalswap * si.mem_unit;
        info->swap_free = si.freeswap * si.mem_unit;
    }
    
    if (info->swap_free > info->swap_total) {
        info->swap_used = 0;
    } else {
        info->swap_used = info->swap_total - info->swap_free;
    }

    return true;
}

// Public function to retrieve memory information
MemoryInfo get_memory_info(void) {
    MemoryInfo info = {0}; // Initialize to zero
    MemInfoRaw raw = {0};
    bool success = false;

    // Try /proc/meminfo first
    if (read_proc_meminfo(&raw)) {
        success = calculate_memory_values(&raw, &info);
        if (!success) {
            fprintf(stderr, "Warning: Failed to calculate memory values, using fallback\n");
        }
    }

    // Fallback to sysinfo if needed
    if (!success) {
        fprintf(stderr, "Falling back to sysinfo for memory information\n");
        success = get_memory_from_sysinfo(&info);
        
        if (!success) {
            fprintf(stderr, "Critical: All memory information retrieval methods failed\n");
            // Set errno to indicate the error
            errno = ENODATA;
        }
    }

    return info;
}

// New function to print memory information in human-readable format
void print_memory_info(const MemoryInfo *info) {
    const double GB = 1024.0 * 1024.0 * 1024.0;
    
    printf("Memory Information:\n");
    printf("------------------\n");
    printf("Total:     %.2f GB (%lu bytes)\n", info->total / GB, info->total);
    printf("Used:      %.2f GB (%lu bytes)\n", info->used / GB, info->used);
    printf("Free:      %.2f GB (%lu bytes)\n", info->free / GB, info->free);
    printf("Available: %.2f GB (%lu bytes)\n", info->available / GB, info->available);
    printf("Buffers:   %.2f GB (%lu bytes)\n", info->buffers / GB, info->buffers);
    printf("Cached:    %.2f GB (%lu bytes)\n", info->cached / GB, info->cached);
    printf("Shared:    %.2f GB (%lu bytes)\n", info->shared / GB, info->shared);
    printf("\nSwap Information:\n");
    printf("------------------\n");
    printf("Total:     %.2f GB (%lu bytes)\n", info->swap_total / GB, info->swap_total);
    printf("Used:      %.2f GB (%lu bytes)\n", info->swap_used / GB, info->swap_used);
    printf("Free:      %.2f GB (%lu bytes)\n", info->swap_free / GB, info->swap_free);
}