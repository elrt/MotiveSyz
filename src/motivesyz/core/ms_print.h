/**
 * @file ms_print.h
 * @brief Simplified and colored output utilities
 *
 * Provides clean, expressive printing functions with automatic color support
 * detection and safe formatting.
 */

#ifndef MS_PRINT_H
#define MS_PRINT_H

#include <stdio.h>
#include <stdarg.h>

/**
 * @defgroup color_codes ANSI Color Codes
 * @brief Terminal color escape sequences
 * @{
 */

#define COLOR_RED     "\033[0;31m"    /**< Red text color */
#define COLOR_GREEN   "\033[0;32m"    /**< Green text color */
#define COLOR_YELLOW  "\033[0;33m"    /**< Yellow text color */
#define COLOR_BLUE    "\033[0;34m"    /**< Blue text color */
#define COLOR_CYAN    "\033[0;36m"    /**< Cyan text color */
#define COLOR_RESET   "\033[0m"       /**< Reset to default color */

/** @} */

/**
 * @brief Print text without newline
 *
 * @param text String to print, safe to call with NULL
 *
 * @note NULL text is silently ignored
 * @note No newline is appended
 */
void print(const char* text);

/**
 * @brief Print text with newline
 *
 * @param text String to print, safe to call with NULL
 *
 * @note NULL text is silently ignored
 * @note Automatically appends newline character
 */
void println(const char* text);

/**
 * @brief Print text in red color
 *
 * @param text String to print in red
 *
 * @note Color is only used if terminal supports it
 * @note Automatically resets color after printing
 */
void print_red(const char* text);

/**
 * @brief Print text in green color
 *
 * @param text String to print in green
 *
 * @note Color is only used if terminal supports it
 * @note Automatically resets color after printing
 */
void print_green(const char* text);

/**
 * @brief Print text in blue color
 *
 * @param text String to print in blue
 *
 * @note Color is only used if terminal supports it
 * @note Automatically resets color after printing
 */
void print_blue(const char* text);

/**
 * @brief Print text in yellow color
 *
 * @param text String to print in yellow
 *
 * @note Color is only used if terminal supports it
 * @note Automatically resets color after printing
 */
void print_yellow(const char* text);

/**
 * @brief Print text in cyan color
 *
 * @param text String to print in cyan
 *
 * @note Color is only used if terminal supports it
 * @note Automatically resets color after printing
 */
void print_cyan(const char* text);

/**
 * @brief Safe formatted printing with automatic buffer management
 *
 * @param format Format string (printf-style)
 * @param ... Arguments for format string
 *
 * @note Automatically handles buffer allocation and resizing
 * @note Safe against buffer overflows
 * @note NULL format is silently ignored
 */
void print_format(const char* format, ...);

/**
 * @brief Print multiple strings sequentially
 *
 * @param count Number of string arguments
 * @param ... String arguments to print
 *
 * @note Variable arguments must be of type const char*
 * @note NULL strings in arguments are silently ignored
 */
void print_multiple(int count, ...);

/**
 * @brief Print a horizontal line of repeated characters
 *
 * @param fill_char Character to use for the line
 * @param length Number of characters in the line
 *
 * @note If length <= 0, no output is produced
 * @note Useful for visual separation in output
 */
void print_line(char fill_char, int length);

#endif
