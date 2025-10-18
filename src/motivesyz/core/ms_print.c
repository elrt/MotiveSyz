/**
 * @file ms_print.c
 * @brief Implementation of simplified printing utilities
 *
 * Handles color support detection, buffer management, and safe output operations.
 */

#include "ms_print.h"
#include <stdlib.h>
#include <unistd.h>

/**
 * @defgroup internal_state Internal Print State
 * @brief Module-level state for buffer and color management
 * @{
 */

static char* format_buffer = NULL;    /**< Dynamic buffer for formatted output */
static size_t buffer_size = 1024;     /**< Current buffer size in bytes */
static int color_support_checked = 0; /**< Color support check flag */
static int color_supported = 0;       /**< Terminal color support result */

/** @} */

/**
 * @brief Check if terminal supports color output
 *
 * @return 1 if terminal supports colors, 0 otherwise
 *
 * @note Result is cached after first check
 * @note Uses isatty() to detect terminal
 */
static int check_color_support(void) {
    if (color_support_checked) {
        return color_supported;
    }

    color_support_checked = 1;
    color_supported = isatty(STDOUT_FILENO);
    return color_supported;
}

/**
 * @brief Cleanup function for format buffer
 *
 * @note Automatically called on program exit
 * @note Safe to call multiple times
 */
static void cleanup_buffer(void) {
    free(format_buffer);
    format_buffer = NULL;
}

/**
 * @brief Internal helper for colored output with automatic reset
 *
 * @param color_code ANSI color sequence to use
 * @param text Text to print in color
 *
 * @note Only uses color if terminal supports it
 * @note Always resets color after printing
 */
static void print_colored(const char* color_code, const char* text) {
    if (check_color_support() && text != NULL) {
        printf("%s%s%s", color_code, text, COLOR_RESET);
    } else if (text != NULL) {
        printf("%s", text);
    }
}

/* Public API implementation */
void print(const char* text) {
    if (text != NULL) {
        printf("%s", text);
    }
}

void println(const char* text) {
    if (text != NULL) {
        printf("%s\n", text);
    }
}

void print_red(const char* text) {
    print_colored(COLOR_RED, text);
}

void print_green(const char* text) {
    print_colored(COLOR_GREEN, text);
}

void print_blue(const char* text) {
    print_colored(COLOR_BLUE, text);
}

void print_yellow(const char* text) {
    print_colored(COLOR_YELLOW, text);
}

void print_cyan(const char* text) {
    print_colored(COLOR_CYAN, text);
}

void print_format(const char* format, ...) {
    if (format == NULL) {
        return;
    }

    /* Lazy initialization of format buffer */
    if (format_buffer == NULL) {
        format_buffer = malloc(buffer_size);
        if (format_buffer == NULL) {
            return;
        }
        atexit(cleanup_buffer);
    }

    va_list args;
    va_start(args, format);

    /* Try formatting with current buffer size */
    int needed = vsnprintf(format_buffer, buffer_size, format, args);
    va_end(args);

    /* Resize buffer if output was truncated */
    if (needed >= (int)buffer_size) {
        buffer_size = (size_t)needed + 1;
        char* new_buffer = realloc(format_buffer, buffer_size);
        if (new_buffer != NULL) {
            format_buffer = new_buffer;

            /* Retry formatting with larger buffer */
            va_start(args, format);
            vsnprintf(format_buffer, buffer_size, format, args);
            va_end(args);
        }
    }

    printf("%s", format_buffer);
}

void print_multiple(int count, ...) {
    va_list args;
    va_start(args, count);

    for (int i = 0; i < count; i++) {
        const char* text = va_arg(args, const char*);
        if (text != NULL) {
            printf("%s", text);
        }
    }

    va_end(args);
}

void print_line(char fill_char, int length) {
    if (length <= 0) {
        return;
    }

    for (int i = 0; i < length; i++) {
        putchar(fill_char);
    }
    putchar('\n');
}
