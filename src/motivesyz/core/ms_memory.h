#ifndef MS_MEMORY_H
#define MS_MEMORY_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Opaque memory allocator handle
 *
 * The allocator provides thread-safe memory management with overflow protection
 * and optional debugging features.
 */
typedef struct ms_allocator ms_allocator_t;

/**
 * @brief Memory allocation result codes
 */
typedef enum {
    MS_MEMORY_SUCCESS = 0,              /**< Operation completed successfully */
    MS_MEMORY_ERROR_INVALID_ARGUMENT,   /**< Invalid parameters provided */
    MS_MEMORY_ERROR_OUT_OF_MEMORY,      /**< System out of memory */
    MS_MEMORY_ERROR_OVERFLOW,           /**< Size calculation overflow */
    MS_MEMORY_ERROR_CORRUPTED,          /**< Memory corruption detected */
    MS_MEMORY_ERROR_DOUBLE_FREE         /**< Attempt to free already freed memory */
} ms_memory_result_t;

/**
 * @brief Create a new memory allocator instance
 *
 * @return New allocator instance or NULL if creation fails
 *
 * @note The allocator must be destroyed with ms_allocator_destroy()
 */
ms_allocator_t* ms_allocator_create(void);

/**
 * @brief Destroy an allocator instance and release its resources
 *
 * @param allocator Allocator to destroy, safe to call with NULL
 *
 * @warning Any allocations from this allocator become invalid after destruction
 */
void ms_allocator_destroy(ms_allocator_t* allocator);

/**
 * @brief Allocate memory block with comprehensive overflow protection
 *
 * @param allocator Allocator instance to use for allocation
 * @param size Number of bytes to allocate, must be > 0
 * @param result Output parameter for allocated memory pointer
 *
 * @return MS_MEMORY_SUCCESS on success, appropriate error code on failure
 *
 * @note The allocated memory is not initialized
 * @note On failure, *result is set to NULL
 */
ms_memory_result_t ms_allocator_allocate(ms_allocator_t* allocator,
                                        size_t size, void** result);

/**
 * @brief Allocate and zero-initialize array with overflow protection
 *
 * @param allocator Allocator instance to use for allocation
 * @param count Number of elements to allocate, 0 is allowed
 * @param size Size of each element in bytes
 * @param result Output parameter for allocated memory pointer
 *
 * @return MS_MEMORY_SUCCESS on success, appropriate error code on failure
 *
 * @note If count is 0, returns success with NULL pointer
 * @note Automatically checks for multiplication overflow
 * @note Memory is initialized to zero before returning
 */
ms_memory_result_t ms_allocator_allocate_zeroed(ms_allocator_t* allocator,
                                               size_t count, size_t size,
                                               void** result);

/**
 * @brief Reallocate memory block preserving original on failure
 *
 * @param allocator Allocator instance to use for reallocation
 * @param ptr Pointer to existing memory block, can be NULL
 * @param new_size New size in bytes, 0 triggers deallocation
 * @param result Output parameter for reallocated memory pointer
 *
 * @return MS_MEMORY_SUCCESS on success, original block preserved on failure
 *
 * @note If ptr is NULL, behaves like ms_allocator_allocate()
 * @note If new_size is 0, behaves like ms_allocator_deallocate()
 * @note On realloc failure, original block remains valid in *result
 */
ms_memory_result_t ms_allocator_reallocate(ms_allocator_t* allocator, void* ptr,
                                          size_t new_size, void** result);

/**
 * @brief Deallocate memory block
 *
 * @param allocator Allocator instance that created the block
 * @param ptr Pointer to memory block to deallocate
 *
 * @return MS_MEMORY_SUCCESS on success, error code if corruption detected
 *
 * @note Safe to call with NULL pointer (no-op)
 * @note Only deallocates memory allocated by the same allocator instance
 */
ms_memory_result_t ms_allocator_deallocate(ms_allocator_t* allocator, void* ptr);

/**
 * @brief Get thread-safe default allocator instance
 *
 * @return Default allocator instance (always valid)
 *
 * @note The default allocator is shared and thread-safe
 * @note Do not destroy the default allocator
 */
ms_allocator_t* ms_allocator_default(void);

#endif
