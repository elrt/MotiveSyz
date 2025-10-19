
/*
 * @file ms_json_api.h
 * @brief Public JSON API and accessor functions
 */

#ifndef MS_JSON_API_H
#define MS_JSON_API_H

#include "ms_json.h"

/**
 * @brief Main parsing API
 */
ms_json_result_t ms_json_parse(const char* input, const ms_json_options_t* options,
                              ms_json_value_t** result);

/**
 * @brief File I/O operations
 */
ms_json_result_t ms_json_parse_file(const char* filename, const ms_json_options_t* options,
                                   ms_json_value_t** result);
ms_json_result_t ms_json_serialize_file(const ms_json_value_t* value, const char* filename);

/**
 * @brief Serialization functions
 */
ms_json_result_t ms_json_serialize(const ms_json_value_t* value, ms_allocator_t* allocator,
                                  char** result);

/**
 * @brief Value accessors
 */
ms_json_type_t ms_json_get_type(const ms_json_value_t* value);
ms_json_result_t ms_json_get_bool(const ms_json_value_t* value, int* result);
ms_json_result_t ms_json_get_number(const ms_json_value_t* value, double* result);
ms_json_result_t ms_json_get_string(const ms_json_value_t* value, const char** result);

/**
 * @brief Array accessors
 */
ms_json_result_t ms_json_get_array_length(const ms_json_value_t* value, size_t* result);
ms_json_result_t ms_json_get_array_element(const ms_json_value_t* value, size_t index,
                                          ms_json_value_t** result);

/**
 * @brief Object accessors
 */
ms_json_result_t ms_json_get_object_size(const ms_json_value_t* value, size_t* result);
ms_json_result_t ms_json_get_object_value(const ms_json_value_t* value, const char* key,
                                         ms_json_value_t** result);
ms_json_result_t ms_json_object_has_key(const ms_json_value_t* value, const char* key, int* result);

#endif
