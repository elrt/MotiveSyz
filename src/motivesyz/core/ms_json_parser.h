/*
 * @file ms_json_parser.h
 * @brief JSON parsing core implementation
 */

#ifndef MS_JSON_PARSER_H
#define MS_JSON_PARSER_H

#include "ms_json.h"

/**
 * @brief JSON parsing context structure
 */
typedef struct ms_json_parse_context {
    const char* input;           /**< Input string being parsed */
    size_t position;            /**< Current position in input */
    size_t length;              /**< Length of input string */
    ms_json_options_t options;  /**< Parsing options */
    ms_allocator_t* allocator;  /**< Allocator to use */
    size_t depth;               /**< Current nesting depth */
} ms_json_parse_context_t;

/**
 * @brief Parse any JSON value from context
 */
ms_json_result_t ms_json_parse_value(ms_json_parse_context_t* ctx, ms_json_value_t** result);

/**
 * @brief Skip whitespace and comments in input
 */
int ms_json_skip_whitespace_and_comments(ms_json_parse_context_t* ctx);

#endif
