#ifndef MS_JSON_INTERNAL_H
#define MS_JSON_INTERNAL_H

#include "ms_json_types.h"

/*
 * Internal JSON structures (not exposed in public API)
 */

typedef struct {
    char* key;
    uint32_t hash;
    ms_json_value_t* value;
} ms_json_object_entry_t;

typedef struct ms_json_array {
    ms_json_value_t** items;
    size_t count;
    size_t capacity;
    ms_allocator_t* allocator;
} ms_json_array_t;

typedef struct ms_json_object {
    ms_json_object_entry_t* entries;
    size_t count;
    size_t capacity;
    ms_allocator_t* allocator;
} ms_json_object_t;

/**
 * Internal JSON value data union
 */
typedef union {
    int boolean;
    double number;
    char* string;
    ms_json_array_t array;
    ms_json_object_t object;
} ms_json_data_t;

/**
 * Internal JSON value structure
 */
struct ms_json_value {
    ms_json_type_t type;
    ms_json_data_t data;
    ms_allocator_t* allocator;
};

/* Internal accessors for .c files */
static inline ms_json_type_t ms_json_value_get_type(const ms_json_value_t* value) {
    return value->type;
}

static inline int ms_json_value_get_bool(const ms_json_value_t* value) {
    return value->data.boolean;
}

static inline double ms_json_value_get_number(const ms_json_value_t* value) {
    return value->data.number;
}

static inline const char* ms_json_value_get_string(const ms_json_value_t* value) {
    return value->data.string;
}

static inline ms_json_array_t* ms_json_value_get_array(ms_json_value_t* value) {
    return &value->data.array;
}

static inline const ms_json_array_t* ms_json_value_get_array_const(const ms_json_value_t* value) {
    return &value->data.array;
}

static inline ms_json_object_t* ms_json_value_get_object(ms_json_value_t* value) {
    return &value->data.object;
}

static inline const ms_json_object_t* ms_json_value_get_object_const(const ms_json_value_t* value) {
    return &value->data.object;
}

#endif /* MS_JSON_INTERNAL_H */
