#include "ms_print.h"

void print(const char* text) {
    printf("%s", text);
}

void println(const char* text) {
    printf("%s\n", text);
}

void print_multiple(int argument_count, ...) {
    va_list arguments;
    va_start(arguments, argument_count);

    for (int i = 0; i < argument_count; i++) {
        const char* text = va_arg(arguments, const char*);
        printf("%s", text);
    }

    va_end(arguments);
}

void print_colored(const char* color_code, const char* text) {
    printf("%s%s%s", color_code, text, COLOR_RESET);
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
