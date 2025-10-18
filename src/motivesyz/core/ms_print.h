/* MotiveSyz Print */
#ifndef MS_PRINT_H
#define MS_PRINT_H

#include <stdio.h>
#include <stdarg.h>

/* ANSI terminal color codes */
#define COLOR_RED     "\033[0;31m"
#define COLOR_GREEN   "\033[0;32m"
#define COLOR_YELLOW  "\033[0;33m"
#define COLOR_BLUE    "\033[0;34m"
#define COLOR_RESET   "\033[0m"

void print(const char* text);
void println(const char* text);
void print_multiple(int count, ...);

void print_red(const char* text);
void print_green(const char* text);
void print_blue(const char* text);
void print_yellow(const char* text);

#endif
