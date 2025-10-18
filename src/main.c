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

    print_line('=', 40);
    print_green("All tests completed successfully!\n");
    print_line('=', 40);

    return 0;
}
