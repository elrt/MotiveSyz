/*
 * @file ms_json_types.h
 * @brief JSON data types and structures
 */

#ifndef MS_JSON_TYPES_H
#define MS_JSON_TYPES_H

#include "ms_memory.h"
#include <stddef.h>

/**
 * @brief JSON value types
 */
typedef enum {
    MS_JSON_NULL = 0,    /**< Null value */
    MS_JSON_BOOL,        /**< Boolean value */
    MS_JSON_NUMBER,      /**< Numeric value */
    MS_JSON_STRING,      /**< String value */
    MS_JSON_ARRAY,       /**< Array of JSON values */
    MS_JSON_OBJECT       /**< Object with key-value pairs */
} ms_json_type_t;

/**
 * @brief JSON value structure (opaque)
 */
typedef struct ms_json_value ms_json_value_t;

/**
 * @brief JSON array structure (opaque)
 */
typedef struct ms_json_array ms_json_array_t;

/**
 * @brief JSON object structure (opaque)
 */
typedef struct ms_json_object ms_json_object_t;

/**
 * @brief JSON parsing result codes
 */
typedef enum {
    MS_JSON_SUCCESS = 0,              /**< Parsing completed successfully */
    MS_JSON_ERROR_INVALID_ARGUMENT,   /**< Invalid parameters provided */
    MS_JSON_ERROR_SYNTAX,             /**< JSON syntax error */
    MS_JSON_ERROR_MEMORY,             /**< Memory allocation failed */
    MS_JSON_ERROR_EOF,                /**< Unexpected end of input */
    MS_JSON_ERROR_DEPTH               /**< Nesting depth exceeded */
} ms_json_result_t;

/**
 * @brief JSON parsing options
 */
typedef struct {
    ms_allocator_t* allocator;  /**< Allocator to use (NULL for default) */
    size_t max_depth;           /**< Maximum nesting depth (0 = unlimited) */
    int allow_comments;         /**< Allow C-style comments in JSON */
} ms_json_options_t;

#endif /* MS_JSON_TYPES_H */
