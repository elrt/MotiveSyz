/**
 * @file main.c
 * @brief MotiveSyz library demonstration and validation
 */

#include <stdio.h>
#include <string.h>
#include "motivesyz/motivesyz.h"

/**
 * @brief Basic memory operations demonstration
 */
static void demo_basic_memory(void) {
    println("=== Basic Memory Operations ===");

    ms_allocator_t* alloc = ms_allocator_default();

    int* number = NULL;
    if (ms_allocator_allocate(alloc, sizeof(int), (void**)&number) == MS_MEMORY_SUCCESS) {
        *number = 42;
        print_format("Allocated integer: %d\n", *number);
        ms_allocator_deallocate(alloc, number);
    }

    // Array allocation
    const size_t count = 5;
    int* array = NULL;
    if (ms_allocator_allocate_zeroed(alloc, count, sizeof(int), (void**)&array) == MS_MEMORY_SUCCESS) {
        for (size_t i = 0; i < count; i++) {
            array[i] = i * 10;
        }
        print_format("Array[%zu]: %d, %d, %d, %d, %d\n",
                    count, array[0], array[1], array[2], array[3], array[4]);
        ms_allocator_deallocate(alloc, array);
    }

    print_green("✓ Basic memory operations completed\n");
}

/**
 * @brief String and output utilities demonstration
 */
static void demo_output_utilities(void) {
    println("=== Output Utilities ===");

    print_red("Error message\n");
    print_green("Success message\n");
    print_blue("Info message\n");
    print_yellow("Warning message\n");
    print_cyan("Debug message\n");

    print_format("User: %s, ID: %d, Score: %.1f\n", "username", 123, 95.5f);

    print_multiple(3, "Multiple ", "strings ", "combined\n");

    print_line('=', 30);

    print_green("✓ Output utilities demonstrated\n");
}

/**
 * @brief Advanced memory patterns demonstration
 */
static void demo_advanced_memory(void) {
    println("=== Advanced Memory Patterns ===");

    ms_allocator_t* alloc = ms_allocator_default();

    size_t size = 3;
    int* data = NULL;
    if (ms_allocator_allocate_zeroed(alloc, size, sizeof(int), (void**)&data) == MS_MEMORY_SUCCESS) {

        for (size_t i = 0; i < size; i++) {
            data[i] = i + 1;
        }

        size_t new_size = 6;
        int* new_data = NULL;
        if (ms_allocator_reallocate(alloc, data, new_size * sizeof(int), (void**)&new_data) == MS_MEMORY_SUCCESS) {
            data = new_data;

            for (size_t i = size; i < new_size; i++) {
                data[i] = i + 1;
            }
            print_format("Resized array: %d, %d, %d, %d, %d, %d\n",
                        data[0], data[1], data[2], data[3], data[4], data[5]);
        }

        ms_allocator_deallocate(alloc, data);
    }

    ms_allocator_t* custom_alloc = ms_allocator_create();
    if (custom_alloc) {
        char* buffer = NULL;
        if (ms_allocator_allocate(custom_alloc, 64, (void**)&buffer) == MS_MEMORY_SUCCESS) {
            strcpy(buffer, "Custom allocator isolation test");
            print_format("Custom allocator: %s\n", buffer);
            ms_allocator_deallocate(custom_alloc, buffer);
        }
        ms_allocator_destroy(custom_alloc);
    }

    print_green("✓ Advanced memory patterns validated\n");
}

/**
 * @brief JSON creation and manipulation demonstration
 */
static void demo_json_creation(void) {
    println("=== JSON Creation & Manipulation ===");

    ms_allocator_t* alloc = ms_allocator_default();

    // Create a complex JSON object
    ms_json_value_t* user = ms_json_create_object(alloc);
    if (user) {
        // Add basic properties
        ms_json_object_set(user, "name", ms_json_create_string(alloc, "A Human"));
        ms_json_object_set(user, "age", ms_json_create_number(alloc, 30));
        ms_json_object_set(user, "is_active", ms_json_create_bool(alloc, 1));
        ms_json_object_set(user, "balance", ms_json_create_number(alloc, 1250.75));

        // Create tags array
        ms_json_value_t* tags = ms_json_create_array(alloc);
        if (tags) {
            ms_json_array_append(tags, ms_json_create_string(alloc, "premium"));
            ms_json_array_append(tags, ms_json_create_string(alloc, "verified"));
            ms_json_array_append(tags, ms_json_create_string(alloc, "developer"));
            ms_json_object_set(user, "tags", tags);
        }

        // Create address obj.
        ms_json_value_t* address = ms_json_create_object(alloc);
        if (address) {
            ms_json_object_set(address, "street", ms_json_create_string(alloc, "123 Main St"));
            ms_json_object_set(address, "city", ms_json_create_string(alloc, "Narva"));
            ms_json_object_set(address, "zipcode", ms_json_create_string(alloc, "14870"));
            ms_json_object_set(user, "address", address);
        }

        // Serialize to string and print
        char* json_str = NULL;
        if (ms_json_serialize(user, alloc, &json_str) == MS_JSON_SUCCESS) {
            print_cyan("Created JSON object:\n");
            println(json_str);
            ms_allocator_deallocate(alloc, json_str);
        }

        // Access and print specific values
        ms_json_value_t* name_value = NULL;
        const char* name = NULL;
        if (ms_json_get_object_value(user, "name", &name_value) == MS_JSON_SUCCESS) {
            ms_json_get_string(name_value, &name);
            print_format("User name: %s\n", name);
        }

        ms_json_value_t* age_value = NULL;
        double age = 0;
        if (ms_json_get_object_value(user, "age", &age_value) == MS_JSON_SUCCESS) {
            ms_json_get_number(age_value, &age);
            print_format("User age: %.0f\n", age);
        }

        // Cleanup
        ms_json_destroy(user, alloc);
    }

    print_green("✓ JSON creation and manipulation completed\n");
}

/**
 * @brief JSON parsing demonstration
 */
static void demo_json_parsing(void) {
    println("=== JSON Parsing ===");

    ms_allocator_t* alloc = ms_allocator_default();

    // Sample JSON string to parse
    const char* json_input =
        "{\n"
        "  \"product\": \"Laptop\",\n"
        "  \"price\": 999.98,\n"
        "  \"in_stock\": true,\n"
        "  \"specifications\": {\n"
        "    \"cpu\": \"Intel Pentium 4\",\n"
        "    \"ram\": \"2GB\",\n"
        "    \"storage\": \"128GB HDD\"\n"
        "  },\n"
        "  \"features\": [\"Backlit Keyboard\", \"Fingerprint Reader\", \"Thunderbolt 4\"]\n"
        "}";

    ms_json_value_t* parsed_json = NULL;
    ms_json_result_t result = ms_json_parse(json_input, NULL, &parsed_json);

    if (result == MS_JSON_SUCCESS && parsed_json) {
        print_green("✓ JSON parsed successfully!\n");

        // Extract and display values
        ms_json_value_t* product_value = NULL;
        const char* product = NULL;
        if (ms_json_get_object_value(parsed_json, "product", &product_value) == MS_JSON_SUCCESS) {
            ms_json_get_string(product_value, &product);
            print_format("Product: %s\n", product);
        }

        ms_json_value_t* price_value = NULL;
        double price = 0;
        if (ms_json_get_object_value(parsed_json, "price", &price_value) == MS_JSON_SUCCESS) {
            ms_json_get_number(price_value, &price);
            print_format("Price: $%.2f\n", price);
        }

        ms_json_value_t* stock_value = NULL;
        int in_stock = 0;
        if (ms_json_get_object_value(parsed_json, "in_stock", &stock_value) == MS_JSON_SUCCESS) {
            ms_json_get_bool(stock_value, &in_stock);
            print_format("In stock: %s\n", in_stock ? "Yes" : "No");
        }

        // Access nested object
        ms_json_value_t* specs = NULL;
        if (ms_json_get_object_value(parsed_json, "specifications", &specs) == MS_JSON_SUCCESS) {
            ms_json_value_t* cpu_value = NULL;
            const char* cpu = NULL;
            if (ms_json_get_object_value(specs, "cpu", &cpu_value) == MS_JSON_SUCCESS) {
                ms_json_get_string(cpu_value, &cpu);
                print_format("CPU: %s\n", cpu);
            }
        }

        // Access array
        ms_json_value_t* features = NULL;
        size_t feature_count = 0;
        if (ms_json_get_object_value(parsed_json, "features", &features) == MS_JSON_SUCCESS &&
            ms_json_get_array_length(features, &feature_count) == MS_JSON_SUCCESS) {

            print_format("Features (%zu):\n", feature_count);
            for (size_t i = 0; i < feature_count; i++) {
                ms_json_value_t* feature = NULL;
                if (ms_json_get_array_element(features, i, &feature) == MS_JSON_SUCCESS) {
                    const char* feature_str = NULL;
                    ms_json_get_string(feature, &feature_str);
                    print_format("  - %s\n", feature_str);
                }
            }
        }

        // Serialize back to string
        char* serialized = NULL;
        if (ms_json_serialize(parsed_json, alloc, &serialized) == MS_JSON_SUCCESS) {
            print_cyan("\nSerialized JSON:\n");
            println(serialized);
            ms_allocator_deallocate(alloc, serialized);
        }

        ms_json_destroy(parsed_json, alloc);
    } else {
        print_format("✗ JSON parsing failed with error code: %d\n", result);
    }

    print_green("✓ JSON parsing demonstration completed\n");
}

/**
 * @brief JSON error handling demonstration
 */
static void demo_json_error_handling(void) {
    println("=== JSON Error Handling ===");

    ms_allocator_t* alloc = ms_allocator_default();

    // Test invalid JSON
    const char* invalid_json = "{ \"name\": \"Human\", age: 67 }"; // Missing quotes around age
    ms_json_value_t* parsed = NULL;
    ms_json_result_t result = ms_json_parse(invalid_json, NULL, &parsed);

    if (result != MS_JSON_SUCCESS) {
        print_format("✓ Correctly detected invalid JSON (error code: %d)\n", result);
    }

    // Test with custom allocator
    ms_allocator_t* custom_alloc = ms_allocator_create();
    if (custom_alloc) {
        ms_json_options_t options = {
            .allocator = custom_alloc,
            .max_depth = 10,
            .allow_comments = 1
        };

        const char* json_with_comments =
            "{\n"
            "  // This is a comment\n"
            "  \"test\": \"value\"\n"
            "}";

        result = ms_json_parse(json_with_comments, &options, &parsed);
        if (result == MS_JSON_SUCCESS) {
            print_green("✓ JSON with comments parsed successfully\n");
            ms_json_destroy(parsed, custom_alloc);
        }

        ms_allocator_destroy(custom_alloc);
    }

    // Test array manipulation
    ms_json_value_t* numbers = ms_json_create_array(alloc);
    if (numbers) {
        for (int i = 1; i <= 5; i++) {
            ms_json_array_append(numbers, ms_json_create_number(alloc, i * 10));
        }

        size_t count = 0;
        ms_json_get_array_length(numbers, &count);
        print_format("Created array with %zu elements: ", count);

        for (size_t i = 0; i < count; i++) {
            ms_json_value_t* element = NULL;
            if (ms_json_get_array_element(numbers, i, &element) == MS_JSON_SUCCESS) {
                double value = 0;
                ms_json_get_number(element, &value);
                print_format("%.0f%s", value, i < count - 1 ? ", " : "");
            }
        }
        println("");

        ms_json_destroy(numbers, alloc);
    }

    print_green("✓ JSON error handling demonstrated\n");
}

/**
 * @brief JSON serialization demonstration - ТЕПЕРЬ РАБОЧАЯ
 */
static void demo_json_serialization(void) {
    println("=== JSON Serialization ===");

    ms_allocator_t* alloc = ms_allocator_default();

    // Create complex test object
    ms_json_value_t* obj = ms_json_create_object(alloc);
    ms_json_object_set(obj, "name", ms_json_create_string(alloc, "Test User"));
    ms_json_object_set(obj, "active", ms_json_create_bool(alloc, 1));
    ms_json_object_set(obj, "count", ms_json_create_number(alloc, 42.5));
    ms_json_object_set(obj, "null_value", ms_json_create_null(alloc));

    // Add array
    ms_json_value_t* tags = ms_json_create_array(alloc);
    ms_json_array_append(tags, ms_json_create_string(alloc, "admin"));
    ms_json_array_append(tags, ms_json_create_string(alloc, "user"));
    ms_json_object_set(obj, "tags", tags);

    // Serialize to string
    char* json_str = NULL;
    if (ms_json_serialize(obj, alloc, &json_str) == MS_JSON_SUCCESS) {
        print_green("✓ Serialization successful!\n");
        print_format("Serialized JSON: %s\n", json_str);

        // Verify it can be parsed back
        ms_json_value_t* parsed = NULL;
        if (ms_json_parse(json_str, NULL, &parsed) == MS_JSON_SUCCESS) {
            print_green("✓ Round-trip parsing successful!\n");
            ms_json_destroy(parsed, alloc);
        }

        ms_allocator_deallocate(alloc, json_str);
    } else {
        print_red("✗ Serialization failed!\n");
    }

    ms_json_destroy(obj, alloc);
    print_green("✓ JSON serialization validated\n");
}

/**
 * @brief Main demonstration entry point
 */
int main(void) {
    print_line('=', 40);
    print_cyan("MotiveSyz Library Demonstration\n");
    print_line('=', 40);

    demo_basic_memory();
    println("");
    demo_output_utilities();
    println("");
    demo_advanced_memory();
    println("");
    demo_json_creation();
    println("");
    demo_json_parsing();
    println("");
    demo_json_error_handling();
    println("");
    demo_json_serialization();  // Добавлен вызов новой функции

    print_line('=', 40);
    print_green("All tests completed successfully!\n");
    print_line('=', 40);

    return 0;
}
