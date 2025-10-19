/*
 * @file ms_json_parser.c
 * @brief Core JSON parsing implementation
 */
#include "ms_json_internal.h"
#include "ms_json_parser.h"
#include "ms_json_builder.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>

/* Configuration constants */
#define MAX_NUMBER_LENGTH 64
#define MAX_STRING_LENGTH (1024 * 1024) /* 1MB max string length */
#define NULL_LENGTH 4
#define TRUE_LENGTH 4
#define FALSE_LENGTH 5

/* Forward declarations for internal functions */
static int ms_json_skip_comments(ms_json_parse_context_t* ctx);
static int ms_json_skip_line_comment(ms_json_parse_context_t* ctx);
static int ms_json_skip_block_comment(ms_json_parse_context_t* ctx);
static ms_json_result_t parse_null(ms_json_parse_context_t* ctx, ms_json_value_t** result);
static ms_json_result_t parse_boolean(ms_json_parse_context_t* ctx, ms_json_value_t** result);
static ms_json_result_t parse_number(ms_json_parse_context_t* ctx, ms_json_value_t** result);
static int ms_json_parse_number_string(ms_json_parse_context_t* ctx);
static ms_json_result_t ms_json_convert_number_string(ms_json_parse_context_t* ctx,
                                                     size_t start, ms_json_value_t** result);
static ms_json_result_t parse_string(ms_json_parse_context_t* ctx, ms_json_value_t** result);
static char* ms_json_parse_string_content(ms_json_parse_context_t* ctx);
static int ms_json_copy_string_content(ms_json_parse_context_t* ctx, size_t start, char* output, size_t max_len);
static char ms_json_decode_escape(char escape_char);
static ms_json_result_t parse_array(ms_json_parse_context_t* ctx, ms_json_value_t** result);
static ms_json_result_t ms_json_parse_array_elements(ms_json_parse_context_t* ctx, ms_json_value_t* array);
static ms_json_result_t parse_object(ms_json_parse_context_t* ctx, ms_json_value_t** result);
static ms_json_result_t ms_json_parse_object_entries(ms_json_parse_context_t* ctx, ms_json_value_t* object);
static int ms_json_expect_colon(ms_json_parse_context_t* ctx);
static int ms_json_expect_comma(ms_json_parse_context_t* ctx);

/* Public API implementation */
ms_json_result_t ms_json_parse_value(ms_json_parse_context_t* ctx, ms_json_value_t** result) {
    if (!ctx || !result) {
        return MS_JSON_ERROR_INVALID_ARGUMENT;
    }

    if (!ms_json_skip_whitespace_and_comments(ctx)) {
        return MS_JSON_ERROR_SYNTAX;
    }

    if (ctx->position >= ctx->length) {
        return MS_JSON_ERROR_EOF;
    }

    char current_char = ctx->input[ctx->position];

    switch (current_char) {
        case 'n': return parse_null(ctx, result);
        case 't':
        case 'f': return parse_boolean(ctx, result);
        case '"': return parse_string(ctx, result);
        case '[': return parse_array(ctx, result);
        case '{': return parse_object(ctx, result);
        default:
            if (isdigit((unsigned char)current_char) || current_char == '-') {
                return parse_number(ctx, result);
            }
            return MS_JSON_ERROR_SYNTAX;
    }
}

int ms_json_skip_whitespace_and_comments(ms_json_parse_context_t* ctx) {
    if (!ctx) {
        return 0;
    }

    while (ctx->position < ctx->length) {
        char current_char = ctx->input[ctx->position];

        if (isspace((unsigned char)current_char)) {
            ctx->position++;
            continue;
        }

        if (!ctx->options.allow_comments || current_char != '/') {
            break;
        }

        if (!ms_json_skip_comments(ctx)) {
            return 0;
        }
    }

    return 1;
}

/* Comment skipping extracted to separate function */
static int ms_json_skip_comments(ms_json_parse_context_t* ctx) {
    if (!ctx || ctx->position + 1 >= ctx->length) {
        return 0;
    }

    char next_char = ctx->input[ctx->position + 1];

    if (next_char == '/') {
        return ms_json_skip_line_comment(ctx);
    } else if (next_char == '*') {
        return ms_json_skip_block_comment(ctx);
    }

    return 1;
}

static int ms_json_skip_line_comment(ms_json_parse_context_t* ctx) {
    if (!ctx) {
        return 0;
    }

    ctx->position += 2; /* Skip "//" */

    while (ctx->position < ctx->length && ctx->input[ctx->position] != '\n') {
        ctx->position++;
    }

    if (ctx->position < ctx->length) {
        ctx->position++; /* Skip newline */
    }

    return 1;
}

static int ms_json_skip_block_comment(ms_json_parse_context_t* ctx) {
    if (!ctx) {
        return 0;
    }

    ctx->position += 2; /* Skip comment start markers */

    while (ctx->position + 1 < ctx->length) {
        if (ctx->input[ctx->position] == '*' && ctx->input[ctx->position + 1] == '/') {
            ctx->position += 2; /* Skip comment end */
            return 1;
        }
        ctx->position++;
    }

    return 0; /* Unclosed block comment */
}

/* Specific value parsers - each focused on one responsibility */
static ms_json_result_t parse_null(ms_json_parse_context_t* ctx, ms_json_value_t** result) {
    if (!ctx || !result) {
        return MS_JSON_ERROR_INVALID_ARGUMENT;
    }

    if (ctx->position + NULL_LENGTH > ctx->length) {
        return MS_JSON_ERROR_EOF;
    }

    if (strncmp(&ctx->input[ctx->position], "null", NULL_LENGTH) != 0) {
        return MS_JSON_ERROR_SYNTAX;
    }

    ctx->position += NULL_LENGTH;
    *result = ms_json_create_null(ctx->allocator);
    return *result ? MS_JSON_SUCCESS : MS_JSON_ERROR_MEMORY;
}

static ms_json_result_t parse_boolean(ms_json_parse_context_t* ctx, ms_json_value_t** result) {
    if (!ctx || !result) {
        return MS_JSON_ERROR_INVALID_ARGUMENT;
    }

    if (ctx->position + TRUE_LENGTH > ctx->length) {
        return MS_JSON_ERROR_EOF;
    }

    if (strncmp(&ctx->input[ctx->position], "true", TRUE_LENGTH) == 0) {
        ctx->position += TRUE_LENGTH;
        *result = ms_json_create_bool(ctx->allocator, 1);
        return *result ? MS_JSON_SUCCESS : MS_JSON_ERROR_MEMORY;
    }

    if (ctx->position + FALSE_LENGTH <= ctx->length &&
        strncmp(&ctx->input[ctx->position], "false", FALSE_LENGTH) == 0) {
        ctx->position += FALSE_LENGTH;
        *result = ms_json_create_bool(ctx->allocator, 0);
        return *result ? MS_JSON_SUCCESS : MS_JSON_ERROR_MEMORY;
    }

    return MS_JSON_ERROR_SYNTAX;
}

static ms_json_result_t parse_number(ms_json_parse_context_t* ctx, ms_json_value_t** result) {
    if (!ctx || !result) {
        return MS_JSON_ERROR_INVALID_ARGUMENT;
    }

    size_t number_start = ctx->position;

    if (!ms_json_parse_number_string(ctx)) {
        return MS_JSON_ERROR_SYNTAX;
    }

    return ms_json_convert_number_string(ctx, number_start, result);
}

static int ms_json_parse_number_string(ms_json_parse_context_t* ctx) {
    if (!ctx) {
        return 0;
    }

    size_t start = ctx->position;
    size_t digits = 0;

    while (ctx->position < ctx->length && digits < MAX_NUMBER_LENGTH) {
        char current_char = ctx->input[ctx->position];
        if (!isdigit((unsigned char)current_char) &&
            current_char != '-' && current_char != '+' &&
            current_char != '.' && current_char != 'e' && current_char != 'E') {
            break;
        }
        ctx->position++;
        digits++;
    }

    /* Ensure we have at least one digit */
    if (digits == 0 || (digits == 1 && (ctx->input[start] == '-' || ctx->input[start] == '+'))) {
        return 0;
    }

    return ctx->position > start;
}

static ms_json_result_t ms_json_convert_number_string(ms_json_parse_context_t* ctx,
                                                     size_t start, ms_json_value_t** result) {
    if (!ctx || !result || start >= ctx->position) {
        return MS_JSON_ERROR_INVALID_ARGUMENT;
    }

    size_t number_length = ctx->position - start;
    if (number_length >= MAX_NUMBER_LENGTH) {
        return MS_JSON_ERROR_SYNTAX;
    }

    char number_string[MAX_NUMBER_LENGTH + 1];
    if (number_length > sizeof(number_string) - 1) {
        return MS_JSON_ERROR_SYNTAX;
    }

    memcpy(number_string, &ctx->input[start], number_length);
    number_string[number_length] = '\0';

    char* parse_end;
    double value = strtod(number_string, &parse_end);

    /* Check if entire string was parsed */
    if (parse_end != number_string + number_length) {
        return MS_JSON_ERROR_SYNTAX;
    }

    /* Check for overflow/underflow using DBL_MAX instead of HUGE_VAL */
    if (value >= DBL_MAX || value <= -DBL_MAX) {
        /* Check if this is actually zero or an error */
        int is_zero = 1;
        for (size_t i = 0; i < number_length; i++) {
            if (number_string[i] != '0' && number_string[i] != '.' &&
                number_string[i] != '-' && number_string[i] != '+') {
                is_zero = 0;
                break;
            }
        }
        if (!is_zero) {
            return MS_JSON_ERROR_SYNTAX;
        }
    }

    *result = ms_json_create_number(ctx->allocator, value);
    return *result ? MS_JSON_SUCCESS : MS_JSON_ERROR_MEMORY;
}

static ms_json_result_t parse_string(ms_json_parse_context_t* ctx, ms_json_value_t** result) {
    if (!ctx || !result) {
        return MS_JSON_ERROR_INVALID_ARGUMENT;
    }

    if (ctx->input[ctx->position] != '"') {
        return MS_JSON_ERROR_SYNTAX;
    }

    // ДОБАВИТЬ ПРОВЕРКУ ДЛИНЫ СТРОКИ
    if (ctx->length - ctx->position > MAX_STRING_LENGTH * 2) {
        return MS_JSON_ERROR_SYNTAX; // Слишком длинная строка
    }

    ctx->position++; /* Skip opening quote */

    char* parsed_string = ms_json_parse_string_content(ctx);
    if (!parsed_string) {
        return MS_JSON_ERROR_SYNTAX;
    }

    ms_json_result_t create_result = MS_JSON_SUCCESS;
    *result = ms_json_create_string(ctx->allocator, parsed_string);

    if (!*result) {
        create_result = MS_JSON_ERROR_MEMORY;
    }

    /* Always free the temporary string */
    free(parsed_string);

    return create_result;
}

static char* ms_json_parse_string_content(ms_json_parse_context_t* ctx) {
    if (!ctx) {
        return NULL;
    }

    size_t start = ctx->position;
    size_t content_length = 0;
    size_t max_content_length = (ctx->length - ctx->position < MAX_STRING_LENGTH) ?
                               ctx->length - ctx->position : MAX_STRING_LENGTH;

    /* Calculate content length with escape processing */
    while (ctx->position < ctx->length &&
           ctx->input[ctx->position] != '"' &&
           content_length < max_content_length) {
        if (ctx->input[ctx->position] == '\\') {
            ctx->position++; /* Skip escape character */
            if (ctx->position >= ctx->length) {
                return NULL; /* Incomplete escape sequence */
            }
        }
        ctx->position++;
        content_length++;
    }

    if (ctx->position >= ctx->length || ctx->input[ctx->position] != '"') {
        return NULL; /* Unclosed string */
    }

    if (content_length >= MAX_STRING_LENGTH) {
        return NULL; /* String too long */
    }

    char* result_string = malloc(content_length + 1);
    if (!result_string) {
        return NULL;
    }

    if (!ms_json_copy_string_content(ctx, start, result_string, content_length)) {
        free(result_string);
        return NULL;
    }

    ctx->position++; /* Skip closing quote */
    return result_string;
}

static int ms_json_copy_string_content(ms_json_parse_context_t* ctx, size_t start, char* output, size_t max_len) {
    if (!ctx || !output || start >= ctx->position) {
        return 0;
    }

    size_t output_position = 0;

    for (size_t i = start; i < ctx->position && output_position < max_len; i++) {
        if (ctx->input[i] == '\\' && i + 1 < ctx->position) {
            i++; /* Skip backslash */
            output[output_position++] = ms_json_decode_escape(ctx->input[i]);
        } else {
            output[output_position++] = ctx->input[i];
        }
    }

    if (output_position > max_len) {
        return 0; /* Buffer overflow */
    }

    output[output_position] = '\0';
    return 1;
}

static char ms_json_decode_escape(char escape_char) {
    switch (escape_char) {
        case '"': return '"';
        case '\\': return '\\';
        case '/': return '/';
        case 'b': return '\b';
        case 'f': return '\f';
        case 'n': return '\n';
        case 'r': return '\r';
        case 't': return '\t';
        default: return escape_char;
    }
}

static ms_json_result_t parse_array(ms_json_parse_context_t* ctx, ms_json_value_t** result) {
    if (!ctx || !result) {
        return MS_JSON_ERROR_INVALID_ARGUMENT;
    }

    if (ctx->options.max_depth > 0 && ctx->depth >= ctx->options.max_depth) {
        return MS_JSON_ERROR_DEPTH;
    }

    ctx->depth++;

    if (ctx->input[ctx->position] != '[') {
        ctx->depth--;
        return MS_JSON_ERROR_SYNTAX;
    }

    ctx->position++; /* Skip '[' */

    ms_json_value_t* array = ms_json_create_array(ctx->allocator);
    if (!array) {
        ctx->depth--;
        return MS_JSON_ERROR_MEMORY;
    }

    if (!ms_json_skip_whitespace_and_comments(ctx)) {
        ms_json_destroy(array, ctx->allocator);
        ctx->depth--;
        return MS_JSON_ERROR_SYNTAX;
    }

    /* Empty array */
    if (ctx->position < ctx->length && ctx->input[ctx->position] == ']') {
        ctx->position++;
        ctx->depth--;
        *result = array;
        return MS_JSON_SUCCESS;
    }

    ms_json_result_t parse_result = ms_json_parse_array_elements(ctx, array);
    if (parse_result != MS_JSON_SUCCESS) {
        ms_json_destroy(array, ctx->allocator);
        ctx->depth--;
        return parse_result;
    }

    ctx->depth--;
    *result = array;
    return MS_JSON_SUCCESS;
}

static ms_json_result_t ms_json_parse_array_elements(ms_json_parse_context_t* ctx, ms_json_value_t* array) {
    if (!ctx || !array) {
        return MS_JSON_ERROR_INVALID_ARGUMENT;
    }

    while (ctx->position < ctx->length) {
        ms_json_value_t* element = NULL;
        ms_json_result_t result = ms_json_parse_value(ctx, &element);

        if (result != MS_JSON_SUCCESS) {
            return result;
        }

        if (ms_json_array_append(array, element) != MS_JSON_SUCCESS) {
            ms_json_destroy(element, ctx->allocator);
            return MS_JSON_ERROR_MEMORY;
        }

        if (!ms_json_skip_whitespace_and_comments(ctx)) {
            return MS_JSON_ERROR_SYNTAX;
        }

        if (ctx->position >= ctx->length) {
            return MS_JSON_ERROR_EOF;
        }

        if (ctx->input[ctx->position] == ']') {
            ctx->position++;
            return MS_JSON_SUCCESS;
        }

        if (ctx->input[ctx->position] != ',') {
            return MS_JSON_ERROR_SYNTAX;
        }

        ctx->position++; /* Skip comma */
        if (!ms_json_skip_whitespace_and_comments(ctx)) {
            return MS_JSON_ERROR_SYNTAX;
        }
    }

    return MS_JSON_ERROR_EOF;
}

static ms_json_result_t parse_object(ms_json_parse_context_t* ctx, ms_json_value_t** result) {
    if (!ctx || !result) {
        return MS_JSON_ERROR_INVALID_ARGUMENT;
    }

    if (ctx->options.max_depth > 0 && ctx->depth >= ctx->options.max_depth) {
        return MS_JSON_ERROR_DEPTH;
    }

    ctx->depth++;

    if (ctx->input[ctx->position] != '{') {
        ctx->depth--;
        return MS_JSON_ERROR_SYNTAX;
    }

    ctx->position++; /* Skip '{' */

    ms_json_value_t* object = ms_json_create_object(ctx->allocator);
    if (!object) {
        ctx->depth--;
        return MS_JSON_ERROR_MEMORY;
    }

    if (!ms_json_skip_whitespace_and_comments(ctx)) {
        ms_json_destroy(object, ctx->allocator);
        ctx->depth--;
        return MS_JSON_ERROR_SYNTAX;
    }

    /* Empty object */
    if (ctx->position < ctx->length && ctx->input[ctx->position] == '}') {
        ctx->position++;
        ctx->depth--;
        *result = object;
        return MS_JSON_SUCCESS;
    }

    ms_json_result_t parse_result = ms_json_parse_object_entries(ctx, object);
    if (parse_result != MS_JSON_SUCCESS) {
        ms_json_destroy(object, ctx->allocator);
        ctx->depth--;
        return parse_result;
    }

    ctx->depth--;
    *result = object;
    return MS_JSON_SUCCESS;
}

static ms_json_result_t ms_json_parse_object_entries(ms_json_parse_context_t* ctx, ms_json_value_t* object) {
    if (!ctx || !object) {
        return MS_JSON_ERROR_INVALID_ARGUMENT;
    }

    while (ctx->position < ctx->length) {
        ms_json_value_t* key_value = NULL;
        ms_json_result_t result = parse_string(ctx, &key_value);

        if (result != MS_JSON_SUCCESS) {
            return result;
        }

        const char* key = NULL;
        if (ms_json_get_string(key_value, &key) != MS_JSON_SUCCESS) {
            ms_json_destroy(key_value, ctx->allocator);
            return MS_JSON_ERROR_SYNTAX;
        }

        if (!ms_json_skip_whitespace_and_comments(ctx)) {
            ms_json_destroy(key_value, ctx->allocator);
            return MS_JSON_ERROR_SYNTAX;
        }

        if (!ms_json_expect_colon(ctx)) {
            ms_json_destroy(key_value, ctx->allocator);
            return MS_JSON_ERROR_SYNTAX;
        }

        ms_json_value_t* value = NULL;
        result = ms_json_parse_value(ctx, &value);

        if (result != MS_JSON_SUCCESS) {
            ms_json_destroy(key_value, ctx->allocator);
            return result;
        }

        result = ms_json_object_set(object, key, value);
        ms_json_destroy(key_value, ctx->allocator);

        if (result != MS_JSON_SUCCESS) {
            ms_json_destroy(value, ctx->allocator);
            return result;
        }

        if (!ms_json_skip_whitespace_and_comments(ctx)) {
            return MS_JSON_ERROR_SYNTAX;
        }

        if (ctx->position >= ctx->length) {
            return MS_JSON_ERROR_EOF;
        }

        if (ctx->input[ctx->position] == '}') {
            ctx->position++;
            return MS_JSON_SUCCESS;
        }

        if (!ms_json_expect_comma(ctx)) {
            return MS_JSON_ERROR_SYNTAX;
        }
    }

    return MS_JSON_ERROR_EOF;
}

static int ms_json_expect_colon(ms_json_parse_context_t* ctx) {
    if (!ctx || ctx->position >= ctx->length || ctx->input[ctx->position] != ':') {
        return 0;
    }

    ctx->position++; /* Skip ':' */
    return ms_json_skip_whitespace_and_comments(ctx);
}

static int ms_json_expect_comma(ms_json_parse_context_t* ctx) {
    if (!ctx || ctx->input[ctx->position] != ',') {
        return 0;
    }

    ctx->position++; /* Skip comma */
    return ms_json_skip_whitespace_and_comments(ctx);
}
