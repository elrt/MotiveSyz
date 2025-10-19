/*
 * @file ms_json_builder.c
 * @brief JSON value creation and manipulation implementation
 */

#include "ms_json_builder.h"
#include "ms_json_internal.h"  // ДОБАВИТЬ ВМЕСТО ЛОКАЛЬНЫХ СТРУКТУР
#include <string.h>
#include <stdlib.h>

/* Internal helper functions */
static ms_json_result_t ms_json_array_grow(ms_json_array_t* array);
static ms_json_result_t ms_json_object_grow(ms_json_object_t* object);
static void ms_json_free_value_data(ms_json_value_t* value, ms_allocator_t* allocator);
static uint32_t ms_json_hash_string(const char* str);

/* Simple string hash function */
static uint32_t ms_json_hash_string(const char* str) {
    uint32_t hash = 2166136261u;
    while (*str) {
        hash ^= (uint32_t)(*str);
        hash *= 16777619u;
        str++;
    }
    return hash;
}

/* JSON value creation functions */
ms_json_value_t* ms_json_create_null(ms_allocator_t* allocator) {
    if (!allocator) {
        allocator = ms_allocator_default();
    }

    ms_json_value_t* value = NULL;
    if (ms_allocator_allocate(allocator, sizeof(ms_json_value_t), (void**)&value) != MS_MEMORY_SUCCESS) {
        return NULL;
    }

    value->type = MS_JSON_NULL;
    value->allocator = allocator;
    return value;
}

ms_json_value_t* ms_json_create_bool(ms_allocator_t* allocator, int value) {
    ms_json_value_t* json_value = ms_json_create_null(allocator);
    if (!json_value) {
        return NULL;
    }

    json_value->type = MS_JSON_BOOL;
    json_value->data.boolean = value ? 1 : 0;
    return json_value;
}

ms_json_value_t* ms_json_create_number(ms_allocator_t* allocator, double value) {
    ms_json_value_t* json_value = ms_json_create_null(allocator);
    if (!json_value) {
        return NULL;
    }

    json_value->type = MS_JSON_NUMBER;
    json_value->data.number = value;
    return json_value;
}

ms_json_value_t* ms_json_create_string(ms_allocator_t* allocator, const char* value) {
    if (!value) {
        return ms_json_create_null(allocator);
    }

    ms_json_value_t* json_value = ms_json_create_null(allocator);
    if (!json_value) {
        return NULL;
    }

    size_t len = strlen(value);
    char* string_copy = NULL;
    if (ms_allocator_allocate(allocator, len + 1, (void**)&string_copy) != MS_MEMORY_SUCCESS) {
        ms_allocator_deallocate(allocator, json_value);
        return NULL;
    }

    memcpy(string_copy, value, len);
    string_copy[len] = '\0';

    json_value->type = MS_JSON_STRING;
    json_value->data.string = string_copy;
    return json_value;
}

ms_json_value_t* ms_json_create_array(ms_allocator_t* allocator) {
    ms_json_value_t* json_value = ms_json_create_null(allocator);
    if (!json_value) {
        return NULL;
    }

    json_value->type = MS_JSON_ARRAY;
    json_value->data.array.allocator = allocator;
    json_value->data.array.items = NULL;
    json_value->data.array.count = 0;
    json_value->data.array.capacity = 0;

    return json_value;
}

ms_json_value_t* ms_json_create_object(ms_allocator_t* allocator) {
    ms_json_value_t* json_value = ms_json_create_null(allocator);
    if (!json_value) {
        return NULL;
    }

    json_value->type = MS_JSON_OBJECT;
    json_value->data.object.allocator = allocator;
    json_value->data.object.entries = NULL;
    json_value->data.object.count = 0;
    json_value->data.object.capacity = 0;

    return json_value;
}

/* JSON value destruction */
void ms_json_destroy(ms_json_value_t* value, ms_allocator_t* allocator) {
    if (!value) {
        return;
    }

    if (!allocator) {
        allocator = value->allocator ? value->allocator : ms_allocator_default();
    }

    ms_json_free_value_data(value, allocator);
    ms_allocator_deallocate(allocator, value);
}

static void ms_json_free_value_data(ms_json_value_t* value, ms_allocator_t* allocator) {
    if (!value || !allocator) {
        return;
    }

    switch (value->type) {
        case MS_JSON_STRING:
            if (value->data.string) {
                ms_allocator_deallocate(allocator, value->data.string);
            }
            break;

        case MS_JSON_ARRAY:
            for (size_t i = 0; i < value->data.array.count; i++) {
                ms_json_destroy(value->data.array.items[i], allocator);
            }
            if (value->data.array.items) {
                ms_allocator_deallocate(allocator, value->data.array.items);
            }
            break;

        case MS_JSON_OBJECT:
            for (size_t i = 0; i < value->data.object.count; i++) {
                ms_json_object_entry_t* entry = &value->data.object.entries[i];
                if (entry->key) {
                    ms_allocator_deallocate(allocator, entry->key);
                }
                if (entry->value) {
                    ms_json_destroy(entry->value, allocator);
                }
            }
            if (value->data.object.entries) {
                ms_allocator_deallocate(allocator, value->data.object.entries);
            }
            break;

        default:
            /* No special cleanup needed for NULL, BOOL, NUMBER */
            break;
    }
}

/* Array manipulation */
ms_json_result_t ms_json_array_append(ms_json_value_t* array, ms_json_value_t* element) {
    if (!array || array->type != MS_JSON_ARRAY || !element) {
        return MS_JSON_ERROR_INVALID_ARGUMENT;
    }

    ms_json_array_t* arr = &array->data.array;

    /* Grow array if needed */
    if (arr->count >= arr->capacity) {
        if (ms_json_array_grow(arr) != MS_JSON_SUCCESS) {
            return MS_JSON_ERROR_MEMORY;
        }
    }

    arr->items[arr->count] = element;
    arr->count++;
    return MS_JSON_SUCCESS;
}

static ms_json_result_t ms_json_array_grow(ms_json_array_t* array) {
    size_t new_capacity = array->capacity == 0 ? 4 : array->capacity * 2;
    ms_json_value_t** new_items = NULL;

    if (ms_allocator_reallocate(array->allocator, array->items,
                               new_capacity * sizeof(ms_json_value_t*),
                               (void**)&new_items) != MS_MEMORY_SUCCESS) {
        return MS_JSON_ERROR_MEMORY;
    }

    array->items = new_items;
    array->capacity = new_capacity;
    return MS_JSON_SUCCESS;
}

/* Object manipulation */
ms_json_result_t ms_json_object_set(ms_json_value_t* object, const char* key, ms_json_value_t* value) {
    if (!object || object->type != MS_JSON_OBJECT || !key || !value) {
        return MS_JSON_ERROR_INVALID_ARGUMENT;
    }

    ms_json_object_t* obj = &object->data.object;
    uint32_t key_hash = ms_json_hash_string(key);

    /* Check if key already exists */
    for (size_t i = 0; i < obj->count; i++) {
        if (obj->entries[i].key &&
            obj->entries[i].hash == key_hash &&
            strcmp(obj->entries[i].key, key) == 0) {
            /* Replace existing value */
            ms_json_destroy(obj->entries[i].value, obj->allocator);
            obj->entries[i].value = value;
            return MS_JSON_SUCCESS;
        }
    }

    /* Grow object if needed */
    if (obj->count >= obj->capacity) {
        if (ms_json_object_grow(obj) != MS_JSON_SUCCESS) {
            return MS_JSON_ERROR_MEMORY;
        }
    }

    /* Create key copy */
    size_t key_len = strlen(key);
    char* key_copy = NULL;
    if (ms_allocator_allocate(obj->allocator, key_len + 1, (void**)&key_copy) != MS_MEMORY_SUCCESS) {
        return MS_JSON_ERROR_MEMORY;
    }
    memcpy(key_copy, key, key_len);
    key_copy[key_len] = '\0';

    /* Add new entry */
    obj->entries[obj->count].key = key_copy;
    obj->entries[obj->count].hash = key_hash;
    obj->entries[obj->count].value = value;
    obj->count++;

    return MS_JSON_SUCCESS;
}

static ms_json_result_t ms_json_object_grow(ms_json_object_t* object) {
    size_t new_capacity = object->capacity == 0 ? 4 : object->capacity * 2;
    ms_json_object_entry_t* new_entries = NULL;

    if (ms_allocator_reallocate(object->allocator, object->entries,
                               new_capacity * sizeof(ms_json_object_entry_t),
                               (void**)&new_entries) != MS_MEMORY_SUCCESS) {
        return MS_JSON_ERROR_MEMORY;
    }

    object->entries = new_entries;
    object->capacity = new_capacity;
    return MS_JSON_SUCCESS;
}
