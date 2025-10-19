/**
 * @file ms_json_api.c
 * @brief Public API implementation and value accessors
 */

#include "ms_json_api.h"
#include "ms_json_parser.h"
#include "ms_json_builder.h"
#include "ms_json_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Configuration */
#define JSON_MAX_DEPTH_DEFAULT 256
#define MAX_FILE_SIZE (10 * 1024 * 1024) /* 10MB max file size */
#define SERIALIZE_BUFFER_INITIAL_SIZE 1024
#define SERIALIZE_MAX_DOUBLE_LENGTH 64

/* Serialization context */
typedef struct {
    ms_allocator_t* allocator;
    char* buffer;
    size_t position;
    size_t capacity;
    int needs_comma;
} ms_json_serialize_context_t;

/* Forward declarations for internal functions */
static ms_json_result_t ms_json_validate_no_trailing_content(ms_json_parse_context_t* ctx, ms_json_value_t** result);
static char* ms_json_read_entire_file(FILE* file);
static int ms_json_validate_access(const ms_json_value_t* value, const void* result, ms_json_type_t expected_type);

/* Serialization functions */
static ms_json_result_t ms_json_serialize_value(const ms_json_value_t* value, ms_json_serialize_context_t* ctx);
static ms_json_result_t ms_json_serialize_null(ms_json_serialize_context_t* ctx);
static ms_json_result_t ms_json_serialize_bool(int value, ms_json_serialize_context_t* ctx);
static ms_json_result_t ms_json_serialize_number(double value, ms_json_serialize_context_t* ctx);
static ms_json_result_t ms_json_serialize_string(const char* value, ms_json_serialize_context_t* ctx);
static ms_json_result_t ms_json_serialize_array(const ms_json_value_t* value, ms_json_serialize_context_t* ctx);
static ms_json_result_t ms_json_serialize_object(const ms_json_value_t* value, ms_json_serialize_context_t* ctx);
static ms_json_result_t ms_json_serialize_ensure_capacity(ms_json_serialize_context_t* ctx, size_t needed);
static ms_json_result_t ms_json_serialize_append(ms_json_serialize_context_t* ctx, const char* data, size_t length);

/* Main parsing function */
ms_json_result_t ms_json_parse(const char* input, const ms_json_options_t* options,
                              ms_json_value_t** result) {
    if (!input || !result) {
        return MS_JSON_ERROR_INVALID_ARGUMENT;
    }

    ms_json_parse_context_t ctx = {
        .input = input,
        .position = 0,
        .length = strlen(input),
        .options = options ? *options : (ms_json_options_t){0},
        .allocator = NULL,
        .depth = 0
    };

    /* Set default options if not provided */
    if (!options) {
        ctx.options.max_depth = JSON_MAX_DEPTH_DEFAULT;
        ctx.options.allow_comments = 0;
        ctx.options.allocator = NULL;
    }

    ctx.allocator = ctx.options.allocator ? ctx.options.allocator : ms_allocator_default();

    if (!ms_json_skip_whitespace_and_comments(&ctx)) {
        return MS_JSON_ERROR_SYNTAX;
    }

    ms_json_result_t parse_result = ms_json_parse_value(&ctx, result);

    if (parse_result == MS_JSON_SUCCESS) {
        parse_result = ms_json_validate_no_trailing_content(&ctx, result);
    }

    return parse_result;
}

static ms_json_result_t ms_json_validate_no_trailing_content(ms_json_parse_context_t* ctx, ms_json_value_t** result) {
    if (!ctx || !result) {
        return MS_JSON_ERROR_INVALID_ARGUMENT;
    }

    ms_json_skip_whitespace_and_comments(ctx);

    if (ctx->position < ctx->length) {
        if (*result) {
            ms_json_destroy(*result, ctx->allocator);
            *result = NULL;
        }
        return MS_JSON_ERROR_SYNTAX;
    }

    return MS_JSON_SUCCESS;
}

/* File I/O operations */
ms_json_result_t ms_json_parse_file(const char* filename, const ms_json_options_t* options,
                                   ms_json_value_t** result) {
    if (!filename || !result) {
        return MS_JSON_ERROR_INVALID_ARGUMENT;
    }

    FILE* file = fopen(filename, "rb");
    if (!file) {
        return MS_JSON_ERROR_INVALID_ARGUMENT;
    }

    char* file_content = ms_json_read_entire_file(file);
    fclose(file);

    if (!file_content) {
        return MS_JSON_ERROR_MEMORY;
    }

    ms_json_result_t parse_result = ms_json_parse(file_content, options, result);
    free(file_content);

    return parse_result;
}

static char* ms_json_read_entire_file(FILE* file) {
    if (!file) {
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        return NULL;
    }

    long file_size = ftell(file);
    if (file_size < 0 || file_size > MAX_FILE_SIZE) {
        return NULL;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        return NULL;
    }

    char* content = malloc(file_size + 1);
    if (!content) {
        return NULL;
    }

    size_t bytes_read = fread(content, 1, file_size, file);
    if (bytes_read != (size_t)file_size) {
        free(content);
        return NULL;
    }

    content[file_size] = '\0';
    return content;
}

ms_json_result_t ms_json_serialize_file(const ms_json_value_t* value, const char* filename) {
    if (!value || !filename) {
        return MS_JSON_ERROR_INVALID_ARGUMENT;
    }

    char* json_string = NULL;
    ms_json_result_t result = ms_json_serialize(value, NULL, &json_string);

    if (result != MS_JSON_SUCCESS) {
        return result;
    }

    FILE* file = fopen(filename, "w");
    if (!file) {
        free(json_string);
        return MS_JSON_ERROR_INVALID_ARGUMENT;
    }

    size_t len = strlen(json_string);
    size_t written = fwrite(json_string, 1, len, file);
    fclose(file);

    free(json_string);

    return (written == len) ? MS_JSON_SUCCESS : MS_JSON_ERROR_INVALID_ARGUMENT;
}

/* Real serialization implementation */
ms_json_result_t ms_json_serialize(const ms_json_value_t* value, ms_allocator_t* allocator,
                                  char** result) {
    if (!value || !result) {
        return MS_JSON_ERROR_INVALID_ARGUMENT;
    }

    if (!allocator) {
        allocator = ms_allocator_default();
    }

    ms_json_serialize_context_t ctx = {
        .allocator = allocator,
        .buffer = NULL,
        .position = 0,
        .capacity = 0,
        .needs_comma = 0
    };

    /* Allocate initial buffer */
    if (ms_allocator_allocate(allocator, SERIALIZE_BUFFER_INITIAL_SIZE, (void**)&ctx.buffer) != MS_MEMORY_SUCCESS) {
        return MS_JSON_ERROR_MEMORY;
    }
    ctx.capacity = SERIALIZE_BUFFER_INITIAL_SIZE;

    ms_json_result_t serialize_result = ms_json_serialize_value(value, &ctx);

    if (serialize_result == MS_JSON_SUCCESS) {
        /* Ensure null termination */
        if (ctx.position >= ctx.capacity) {
            if (ms_json_serialize_ensure_capacity(&ctx, 1) != MS_JSON_SUCCESS) {
                ms_allocator_deallocate(allocator, ctx.buffer);
                return MS_JSON_ERROR_MEMORY;
            }
        }
        ctx.buffer[ctx.position] = '\0';
        *result = ctx.buffer;
    } else {
        if (ctx.buffer) {
            ms_allocator_deallocate(allocator, ctx.buffer);
        }
    }

    return serialize_result;
}

static ms_json_result_t ms_json_serialize_value(const ms_json_value_t* value, ms_json_serialize_context_t* ctx) {
    switch (ms_json_value_get_type(value)) {
        case MS_JSON_NULL:
            return ms_json_serialize_null(ctx);
        case MS_JSON_BOOL:
            return ms_json_serialize_bool(ms_json_value_get_bool(value), ctx);
        case MS_JSON_NUMBER:
            return ms_json_serialize_number(ms_json_value_get_number(value), ctx);
        case MS_JSON_STRING:
            return ms_json_serialize_string(ms_json_value_get_string(value), ctx);
        case MS_JSON_ARRAY:
            return ms_json_serialize_array(value, ctx);
        case MS_JSON_OBJECT:
            return ms_json_serialize_object(value, ctx);
        default:
            return MS_JSON_ERROR_INVALID_ARGUMENT;
    }
}

static ms_json_result_t ms_json_serialize_null(ms_json_serialize_context_t* ctx) {
    return ms_json_serialize_append(ctx, "null", 4);
}

static ms_json_result_t ms_json_serialize_bool(int value, ms_json_serialize_context_t* ctx) {
    if (value) {
        return ms_json_serialize_append(ctx, "true", 4);
    } else {
        return ms_json_serialize_append(ctx, "false", 5);
    }
}

static ms_json_result_t ms_json_serialize_number(double value, ms_json_serialize_context_t* ctx) {
    char number_buffer[SERIALIZE_MAX_DOUBLE_LENGTH];

    /* Handle special cases */
    if (isnan(value)) {
        return ms_json_serialize_append(ctx, "null", 4);
    }

    if (isinf(value)) {
        if (value > 0) {
            return ms_json_serialize_append(ctx, "1e999", 5);
        } else {
            return ms_json_serialize_append(ctx, "-1e999", 6);
        }
    }

    /* Use snprintf for proper formatting */
    int length = snprintf(number_buffer, sizeof(number_buffer), "%.15g", value);

    if (length < 0 || length >= (int)sizeof(number_buffer)) {
        return MS_JSON_ERROR_INVALID_ARGUMENT;
    }

    return ms_json_serialize_append(ctx, number_buffer, (size_t)length);
}

static ms_json_result_t ms_json_serialize_string(const char* value, ms_json_serialize_context_t* ctx) {
    if (!value) {
        return ms_json_serialize_append(ctx, "\"\"", 2);
    }

    /* Start with quote */
    ms_json_result_t result = ms_json_serialize_append(ctx, "\"", 1);
    if (result != MS_JSON_SUCCESS) {
        return result;
    }

    /* Escape and append string content */
    const char* p = value;
    while (*p) {
        char escape_buffer[8];
        const char* to_append = NULL;
        size_t append_length = 1;

        switch (*p) {
            case '"':  to_append = "\\\""; append_length = 2; break;
            case '\\': to_append = "\\\\"; append_length = 2; break;
            case '\b': to_append = "\\b"; append_length = 2; break;
            case '\f': to_append = "\\f"; append_length = 2; break;
            case '\n': to_append = "\\n"; append_length = 2; break;
            case '\r': to_append = "\\r"; append_length = 2; break;
            case '\t': to_append = "\\t"; append_length = 2; break;
            default:
                if ((unsigned char)*p < 0x20) {
                    /* Control character - escape as Unicode */
                    snprintf(escape_buffer, sizeof(escape_buffer), "\\u%04x", (unsigned char)*p);
                    to_append = escape_buffer;
                    append_length = 6;
                } else {
                    to_append = p;
                    append_length = 1;
                }
                break;
        }

        result = ms_json_serialize_append(ctx, to_append, append_length);
        if (result != MS_JSON_SUCCESS) {
            return result;
        }

        p++;
    }

    /* Closing quote */
    return ms_json_serialize_append(ctx, "\"", 1);
}

static ms_json_result_t ms_json_serialize_array(const ms_json_value_t* value, ms_json_serialize_context_t* ctx) {
    ms_json_result_t result = ms_json_serialize_append(ctx, "[", 1);
    if (result != MS_JSON_SUCCESS) {
        return result;
    }

    int saved_needs_comma = ctx->needs_comma;
    ctx->needs_comma = 0;

    const ms_json_array_t* array = ms_json_value_get_array_const(value);
    for (size_t i = 0; i < array->count; i++) {
        if (ctx->needs_comma) {
            result = ms_json_serialize_append(ctx, ",", 1);
            if (result != MS_JSON_SUCCESS) {
                return result;
            }
        }

        result = ms_json_serialize_value(array->items[i], ctx);
        if (result != MS_JSON_SUCCESS) {
            return result;
        }

        ctx->needs_comma = 1;
    }

    ctx->needs_comma = saved_needs_comma;
    return ms_json_serialize_append(ctx, "]", 1);
}

static ms_json_result_t ms_json_serialize_object(const ms_json_value_t* value, ms_json_serialize_context_t* ctx) {
    ms_json_result_t result = ms_json_serialize_append(ctx, "{", 1);
    if (result != MS_JSON_SUCCESS) {
        return result;
    }

    int saved_needs_comma = ctx->needs_comma;
    ctx->needs_comma = 0;

    const ms_json_object_t* object = ms_json_value_get_object_const(value);
    for (size_t i = 0; i < object->count; i++) {
        if (ctx->needs_comma) {
            result = ms_json_serialize_append(ctx, ",", 1);
            if (result != MS_JSON_SUCCESS) {
                return result;
            }
        }

        /* Serialize key */
        result = ms_json_serialize_string(object->entries[i].key, ctx);
        if (result != MS_JSON_SUCCESS) {
            return result;
        }

        /* Colon separator */
        result = ms_json_serialize_append(ctx, ":", 1);
        if (result != MS_JSON_SUCCESS) {
            return result;
        }

        /* Serialize value */
        result = ms_json_serialize_value(object->entries[i].value, ctx);
        if (result != MS_JSON_SUCCESS) {
            return result;
        }

        ctx->needs_comma = 1;
    }

    ctx->needs_comma = saved_needs_comma;
    return ms_json_serialize_append(ctx, "}", 1);
}

static ms_json_result_t ms_json_serialize_ensure_capacity(ms_json_serialize_context_t* ctx, size_t needed) {
    if (ctx->position + needed <= ctx->capacity) {
        return MS_JSON_SUCCESS;
    }

    size_t new_capacity = ctx->capacity * 2;
    if (new_capacity < ctx->position + needed) {
        new_capacity = ctx->position + needed;
    }

    char* new_buffer = NULL;
    if (ms_allocator_reallocate(ctx->allocator, ctx->buffer, new_capacity, (void**)&new_buffer) != MS_MEMORY_SUCCESS) {
        return MS_JSON_ERROR_MEMORY;
    }

    ctx->buffer = new_buffer;
    ctx->capacity = new_capacity;
    return MS_JSON_SUCCESS;
}

static ms_json_result_t ms_json_serialize_append(ms_json_serialize_context_t* ctx, const char* data, size_t length) {
    ms_json_result_t result = ms_json_serialize_ensure_capacity(ctx, length);
    if (result != MS_JSON_SUCCESS) {
        return result;
    }

    memcpy(ctx->buffer + ctx->position, data, length);
    ctx->position += length;
    return MS_JSON_SUCCESS;
}

/* Value type accessors */
ms_json_type_t ms_json_get_type(const ms_json_value_t* value) {
    return value ? ms_json_value_get_type(value) : MS_JSON_NULL;
}

ms_json_result_t ms_json_get_bool(const ms_json_value_t* value, int* result) {
    if (!ms_json_validate_access(value, result, MS_JSON_BOOL)) {
        return MS_JSON_ERROR_INVALID_ARGUMENT;
    }

    *result = ms_json_value_get_bool(value);
    return MS_JSON_SUCCESS;
}

ms_json_result_t ms_json_get_number(const ms_json_value_t* value, double* result) {
    if (!ms_json_validate_access(value, result, MS_JSON_NUMBER)) {
        return MS_JSON_ERROR_INVALID_ARGUMENT;
    }

    *result = ms_json_value_get_number(value);
    return MS_JSON_SUCCESS;
}

ms_json_result_t ms_json_get_string(const ms_json_value_t* value, const char** result) {
    if (!ms_json_validate_access(value, result, MS_JSON_STRING)) {
        return MS_JSON_ERROR_INVALID_ARGUMENT;
    }

    *result = ms_json_value_get_string(value);
    return MS_JSON_SUCCESS;
}

static int ms_json_validate_access(const ms_json_value_t* value, const void* result, ms_json_type_t expected_type) {
    return value && result && ms_json_value_get_type(value) == expected_type;
}

/* Array accessors */
ms_json_result_t ms_json_get_array_length(const ms_json_value_t* value, size_t* result) {
    if (!ms_json_validate_access(value, result, MS_JSON_ARRAY)) {
        return MS_JSON_ERROR_INVALID_ARGUMENT;
    }

    const ms_json_array_t* array = ms_json_value_get_array_const(value);
    *result = array->count;
    return MS_JSON_SUCCESS;
}

ms_json_result_t ms_json_get_array_element(const ms_json_value_t* value, size_t index,
                                          ms_json_value_t** result) {
    if (!ms_json_validate_access(value, result, MS_JSON_ARRAY)) {
        return MS_JSON_ERROR_INVALID_ARGUMENT;
    }

    const ms_json_array_t* array = ms_json_value_get_array_const(value);
    if (index >= array->count) {
        return MS_JSON_ERROR_INVALID_ARGUMENT;
    }

    *result = array->items[index];
    return MS_JSON_SUCCESS;
}

/* Object accessors */
ms_json_result_t ms_json_get_object_size(const ms_json_value_t* value, size_t* result) {
    if (!ms_json_validate_access(value, result, MS_JSON_OBJECT)) {
        return MS_JSON_ERROR_INVALID_ARGUMENT;
    }

    const ms_json_object_t* object = ms_json_value_get_object_const(value);
    *result = object->count;
    return MS_JSON_SUCCESS;
}

ms_json_result_t ms_json_get_object_value(const ms_json_value_t* value, const char* key,
                                         ms_json_value_t** result) {
    if (!ms_json_validate_access(value, result, MS_JSON_OBJECT) || !key) {
        return MS_JSON_ERROR_INVALID_ARGUMENT;
    }

    const ms_json_object_t* object = ms_json_value_get_object_const(value);
    for (size_t i = 0; i < object->count; i++) {
        if (object->entries[i].key &&
            strcmp(object->entries[i].key, key) == 0) {
            *result = object->entries[i].value;
            return MS_JSON_SUCCESS;
        }
    }

    return MS_JSON_ERROR_INVALID_ARGUMENT;
}

ms_json_result_t ms_json_object_has_key(const ms_json_value_t* value, const char* key, int* result) {
    if (!ms_json_validate_access(value, result, MS_JSON_OBJECT) || !key) {
        return MS_JSON_ERROR_INVALID_ARGUMENT;
    }

    *result = 0;
    const ms_json_object_t* object = ms_json_value_get_object_const(value);
    for (size_t i = 0; i < object->count; i++) {
        if (object->entries[i].key &&
            strcmp(object->entries[i].key, key) == 0) {
            *result = 1;
            break;
        }
    }

    return MS_JSON_SUCCESS;
}
