#ifndef MS_MEMORY_STATS_H
#define MS_MEMORY_STATS_H

#include "ms_memory.h"
#include <stddef.h>

typedef struct {
    size_t bytes_allocated;
    size_t allocation_count;
    size_t peak_bytes_allocated;
} ms_memory_stats_t;

/* Statistics are separate from core allocation */
void ms_memory_stats_record_allocation(ms_memory_stats_t* stats, size_t size);
void ms_memory_stats_record_deallocation(ms_memory_stats_t* stats, size_t size);
void ms_memory_stats_reset(ms_memory_stats_t* stats);

#endif
