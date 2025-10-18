# MotiveSyz

![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)
![License](https://img.shields.io/badge/License-GPL%20v2-blue.svg)
![Version](https://img.shields.io/badge/Version-0.2.0-green.svg)
![Status](https://img.shields.io/badge/Status-Production--ready-brightgreen.svg)

A clean C library providing simplified, expressive syntax for system programming and everyday tasks. Licensed under GPL-2.0.

MotiveSyz reimagines C standard library APIs with modern safety guarantees, better error handling, and intuitive interfaces while maintaining zero-cost abstractions and full compatibility.

## Why MotiveSyz?

| Task | Standard C | MotiveSyz |
|------|------------|-----------|
| Safe string concat | 5-10 lines | 1 function |
| Memory error detection | Manual | Automatic |
| Colored output | Complex | One call |

## Features

### Memory Safety
- Bounds-checked allocations with comprehensive overflow protection
- Guard-based corruption detection in debug builds
- Thread-safe allocators with instance isolation
- Zero-overhead in release with compile-time configuration
- Use-after-free detection with allocator tracking

### Expressive Output
- Colorful terminal output with automatic capability detection
- Safe formatting with automatic buffer management
- Clean API without format specifier complexity
- Type-safe variadic functions
- Visual separators and structured logging

### Modern API Design
- Consistent error handling with detailed result codes
- Resource ownership tracking preventing use-after-free
- Compiler-enforced contracts with extensive parameter validation
- Full Doxygen documentation with examples
- Cross-platform compatibility with system-specific optimizations

### Basic Usage

```c
#include <motivesyz/motivesyz.h>

int main(void) {
    // Clean memory management
    ms_allocator_t* alloc = ms_allocator_default();

    int* data = NULL;
    if (ms_allocator_allocate_zeroed(alloc, 100, sizeof(int), (void**)&data) == MS_MEMORY_SUCCESS) {
        // Safe array usage
        for (int i = 0; i < 100; i++) {
            data[i] = i * 2;
        }

        // Expressive output
        print_green("Array initialized successfully!\n");
        print_format("First element: %d, Last element: %d\n", data[0], data[99]);

        ms_allocator_deallocate(alloc, data);
    }

    return 0;
}
```

## API Reference

### Memory Management

```c
// Create dedicated allocator for resource isolation
ms_allocator_t* allocator = ms_allocator_create();

// Safe allocation with overflow protection
void* data = NULL;
ms_memory_result_t result = ms_allocator_allocate(allocator, size, &data);

// Automatic zero-initialization for arrays
int* array = NULL;
ms_allocator_allocate_zeroed(allocator, count, sizeof(int), (void**)&array);

// Reallocation preserves data on failure
ms_allocator_reallocate(allocator, ptr, new_size, &new_ptr);

// Safe deallocation with validation
ms_allocator_deallocate(allocator, ptr);

// Cleanup allocator resources
ms_allocator_destroy(allocator);
```

### Output System

```c
// Basic output
print("Hello ");
println("World!");

// Colored output (auto-detected)
print_red("Error message");
print_green("Success message");
print_blue("Info message");

// Safe formatting
print_format("User: %s, Score: %d, Time: %.2f", name, score, time);

// Multiple strings
print_multiple(4, "This ", "is ", "concatenated", " safely");

// Visual elements
print_line('=', 50);  // Separator line
```

## Building

### Requirements
- C99 compatible compiler (GCC, Clang, MSVC)
- GNU Make or compatible build system
- Standard C library

### Build Options

```bash
# Debug build with full safety checks
make DEBUG=1

# Release build (maximum performance)
make

# With custom compiler
make CC=clang
```

### Integration

Add to your project:
```makefile
CFLAGS += -I/path/to/motivesyz/src
LDFLAGS += -L/path/to/motivesyz/bin -lmotivesyz
```

## Performance

MotiveSyz is designed for zero-overhead in release mode:
- Debug: Full safety checks and validation
- Release: Minimal overhead, inline optimizations
- Size: Minimal footprint with essential features only

## License

This project is licensed under the GNU General Public License v2.0 - see the LICENSE file for details.

### Considerations:

- Not suitable for proprietary/closed-source software
- License inheritance affects combined works
- Source disclosure required when distributing binaries
