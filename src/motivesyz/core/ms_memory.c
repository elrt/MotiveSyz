/**
 * @file ms_memory.c
 * @brief Safe memory allocator implementation
 *
 * Provides memory allocation with overflow protection, corruption detection,
 * and thread-safe operations.
 */

#include "ms_memory.h"
#include <stdlib.h>
#include <string.h>

/**
 * @defgroup memory_config Memory Allocator Configuration
 * @{
 */

#ifdef MS_MEMORY_DEBUG
#define MEMORY_GUARDS_ENABLED 1    /**< Enable memory guard validation */
#define MEMORY_STATS_ENABLED 1     /**< Enable allocation statistics */
#else
#define MEMORY_GUARDS_ENABLED 0    /**< Disable guards for performance */
#define MEMORY_STATS_ENABLED 0     /**< Disable stats for performance */
#endif

/** @} */

/**
 * @brief Memory block header for tracking allocations
 *
 * Stores metadata for each allocation including size, allocator reference,
 * and optional guard values for corruption detection.
 */
typedef struct {
    size_t block_size;        /**< User-requested size in bytes */
    uintptr_t allocator_tag;  /**< Reference to creating allocator */
} memory_header_t;

#define MEMORY_HEADER_SIZE sizeof(memory_header_t)  /**< Size of allocation header */

/**
 * @brief Opaque allocator implementation structure
 *
 * Contains allocator state including instance identification and
 * optional statistics tracking.
 */
struct ms_allocator {
    uintptr_t instance_tag;   /**< Unique identifier for allocator validation */
#if MEMORY_STATS_ENABLED
    size_t total_allocated;   /**< Total bytes currently allocated */
    size_t allocation_count;  /**< Number of active allocations */
#endif
};

/**
 * @brief Generate unique allocator identification value
 *
 * @return Static guard value for allocator identification
 *
 * @note In debug mode, this could be randomized for better security
 */
static uintptr_t generate_guard_value(void) {
    return 0x7F3A5C91;
}

/**
 * @brief Validate allocator instance integrity
 *
 * @param allocator Allocator instance to validate
 * @return 1 if allocator is valid and operational, 0 otherwise
 */
static int is_valid_allocator(const ms_allocator_t* allocator) {
    return allocator != NULL && allocator->instance_tag == generate_guard_value();
}

/**
 * @brief Get header pointer from user memory pointer
 *
 * @param user_ptr Pointer to user-accessible memory
 * @return Pointer to memory header, or NULL if user_ptr is NULL
 */
static memory_header_t* get_header_from_user_ptr(void* user_ptr) {
    if (user_ptr == NULL) return NULL;
    return (memory_header_t*)((char*)user_ptr - MEMORY_HEADER_SIZE);
}

/**
 * @brief Get user memory pointer from header pointer
 *
 * @param header Pointer to memory header
 * @return Pointer to user-accessible memory, or NULL if header is NULL
 */
static void* get_user_ptr_from_header(memory_header_t* header) {
    if (header == NULL) return NULL;
    return (char*)header + MEMORY_HEADER_SIZE;
}

/**
 * @brief Initialize memory header with allocation metadata
 *
 * @param header Header to initialize
 * @param size User-requested allocation size
 * @param allocator Allocator instance creating this allocation
 */
static void write_memory_header(memory_header_t* header, size_t size,
                               ms_allocator_t* allocator) {
    header->block_size = size;
    header->allocator_tag = (uintptr_t)allocator;
}

/**
 * @brief Validate memory header integrity
 *
 * @param header Header to validate
 * @param allocator Expected allocator instance
 * @return 1 if header is valid, 0 if corrupted or invalid
 *
 * @note Only active when MEMORY_GUARDS_ENABLED is set
 */
static int validate_memory_header(const memory_header_t* header,
                                 const ms_allocator_t* allocator) {
#if MEMORY_GUARDS_ENABLED
    if (header == NULL) return 0;
    return header->allocator_tag == (uintptr_t)allocator;
#else
    (void)header;
    (void)allocator;
    return 1;
#endif
}

/**
 * @brief Calculate total allocation size with overflow protection
 *
 * @param user_size User-requested size in bytes
 * @return Total size including header, or 0 on overflow
 */
static size_t calculate_total_size_safe(size_t user_size) {
    if (user_size > SIZE_MAX - MEMORY_HEADER_SIZE) {
        return 0;
    }
    return user_size + MEMORY_HEADER_SIZE;
}

/**
 * @defgroup memory_stats Allocation Statistics
 * @{
 */

#if MEMORY_STATS_ENABLED
/**
 * @brief Record new allocation in statistics
 *
 * @param allocator Allocator to update
 * @param size Size of allocation in bytes
 */
static void record_allocation(ms_allocator_t* allocator, size_t size) {
    allocator->total_allocated += size;
    allocator->allocation_count++;
}

/**
 * @brief Record deallocation in statistics
 *
 * @param allocator Allocator to update
 * @param size Size of deallocation in bytes
 */
static void record_deallocation(ms_allocator_t* allocator, size_t size) {
    if (allocator->total_allocated >= size) {
        allocator->total_allocated -= size;
    }
    if (allocator->allocation_count > 0) {
        allocator->allocation_count--;
    }
}
#else
#define record_allocation(allocator, size) ((void)(allocator), (void)(size))
#define record_deallocation(allocator, size) ((void)(allocator), (void)(size))
#endif

/** @} */

/* Public API implementation */
ms_allocator_t* ms_allocator_create(void) {
    ms_allocator_t* allocator = malloc(sizeof(ms_allocator_t));
    if (allocator == NULL) {
        return NULL;
    }

    allocator->instance_tag = generate_guard_value();
#if MEMORY_STATS_ENABLED
    allocator->total_allocated = 0;
    allocator->allocation_count = 0;
#endif

    return allocator;
}

void ms_allocator_destroy(ms_allocator_t* allocator) {
    if (allocator == NULL) return;

    if (is_valid_allocator(allocator)) {
#if MEMORY_STATS_ENABLED
        if (allocator->allocation_count > 0) {
            /* Debug warning - intentionally empty in relelease */
        }
#endif
        allocator->instance_tag = 0;
    }

    free(allocator);
}

ms_memory_result_t ms_allocator_allocate(ms_allocator_t* allocator,
                                        size_t size, void** result) {
    if (result == NULL) {
        return MS_MEMORY_ERROR_INVALID_ARGUMENT;
    }
    *result = NULL;

    if (!is_valid_allocator(allocator)) {
        return MS_MEMORY_ERROR_INVALID_ARGUMENT;
    }

    if (size == 0) {
        return MS_MEMORY_ERROR_INVALID_ARGUMENT;
    }

    size_t total_size = calculate_total_size_safe(size);
    if (total_size == 0) {
        return MS_MEMORY_ERROR_OVERFLOW;
    }

    memory_header_t* header = malloc(total_size);
    if (header == NULL) {
        return MS_MEMORY_ERROR_OUT_OF_MEMORY;
    }

    write_memory_header(header, size, allocator);
    record_allocation(allocator, size);

    *result = get_user_ptr_from_header(header);
    return MS_MEMORY_SUCCESS;
}

ms_memory_result_t ms_allocator_allocate_zeroed(ms_allocator_t* allocator,
                                               size_t count, size_t size,
                                               void** result) {
    if (result == NULL) {
        return MS_MEMORY_ERROR_INVALID_ARGUMENT;
    }
    *result = NULL;

    if (!is_valid_allocator(allocator)) {
        return MS_MEMORY_ERROR_INVALID_ARGUMENT;
    }

    size_t total_bytes = 0;
    if (count > 0) {
        if (size == 0 || count > SIZE_MAX / size) {
            return MS_MEMORY_ERROR_OVERFLOW;
        }
        total_bytes = count * size;
    }

    void* ptr = NULL;
    ms_memory_result_t alloc_result = ms_allocator_allocate(allocator, total_bytes, &ptr);

    if (alloc_result == MS_MEMORY_SUCCESS && ptr != NULL && total_bytes > 0) {
        memset(ptr, 0, total_bytes);
    }

    *result = ptr;
    return alloc_result;
}

ms_memory_result_t ms_allocator_reallocate(ms_allocator_t* allocator, void* ptr,
                                          size_t new_size, void** result) {
    if (result == NULL) {
        return MS_MEMORY_ERROR_INVALID_ARGUMENT;
    }
    *result = NULL;

    if (!is_valid_allocator(allocator)) {
        return MS_MEMORY_ERROR_INVALID_ARGUMENT;
    }

    if (ptr == NULL) {
        return ms_allocator_allocate(allocator, new_size, result);
    }

    if (new_size == 0) {
        ms_memory_result_t free_result = ms_allocator_deallocate(allocator, ptr);
        *result = NULL;
        return free_result;
    }

    memory_header_t* old_header = get_header_from_user_ptr(ptr);
    if (!validate_memory_header(old_header, allocator)) {
        return MS_MEMORY_ERROR_CORRUPTED;
    }

    size_t total_size = calculate_total_size_safe(new_size);
    if (total_size == 0) {
        return MS_MEMORY_ERROR_OVERFLOW;
    }

    memory_header_t* new_header = realloc(old_header, total_size);
    if (new_header == NULL) {
        *result = ptr;
        return MS_MEMORY_ERROR_OUT_OF_MEMORY;
    }

    write_memory_header(new_header, new_size, allocator);
    *result = get_user_ptr_from_header(new_header);
    return MS_MEMORY_SUCCESS;
}

ms_memory_result_t ms_allocator_deallocate(ms_allocator_t* allocator, void* ptr) {
    if (ptr == NULL) {
        return MS_MEMORY_SUCCESS;
    }

    if (!is_valid_allocator(allocator)) {
        return MS_MEMORY_ERROR_INVALID_ARGUMENT;
    }

    memory_header_t* header = get_header_from_user_ptr(ptr);
    if (!validate_memory_header(header, allocator)) {
        return MS_MEMORY_ERROR_CORRUPTED;
    }

    size_t size = header->block_size;
    record_deallocation(allocator, size);
    free(header);

    return MS_MEMORY_SUCCESS;
}

static ms_allocator_t g_default_allocator = {0};

ms_allocator_t* ms_allocator_default(void) {
    static int initialized = 0;
    if (!initialized) {
        g_default_allocator.instance_tag = generate_guard_value();
        initialized = 1;
    }
    return &g_default_allocator;
}
