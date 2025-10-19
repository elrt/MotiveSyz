/*
 * @file ms_json_builder.h
 * @brief JSON value creation and manipulation
 */

#ifndef MS_JSON_BUILDER_H
#define MS_JSON_BUILDER_H

#include "ms_json.h"

/**
 * @brief Create all JSON value types
 */
ms_json_value_t* ms_json_create_null(ms_allocator_t* allocator);
ms_json_value_t* ms_json_create_bool(ms_allocator_t* allocator, int value);
ms_json_value_t* ms_json_create_number(ms_allocator_t* allocator, double value);
ms_json_value_t* ms_json_create_string(ms_allocator_t* allocator, const char* value);
ms_json_value_t* ms_json_create_array(ms_allocator_t* allocator);
ms_json_value_t* ms_json_create_object(ms_allocator_t* allocator);

/**
 * @brief Destroy JSON value and children
 */
void ms_json_destroy(ms_json_value_t* value, ms_allocator_t* allocator);

/**
 * @brief Array manipulation functions
 */
ms_json_result_t ms_json_array_append(ms_json_value_t* array, ms_json_value_t* element);

/**
 * @brief Object manipulation functions
 */
ms_json_result_t ms_json_object_set(ms_json_value_t* object, const char* key, ms_json_value_t* value);

#endif
