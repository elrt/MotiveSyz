/* Demonstration of MotiveSyz print utilities */
#include "motivesyz/core/ms_print.h"

void demonstrate_basic_output(void) {
    println("Standard text output");
}

void demonstrate_colored_output(void) {
    print_red("Red text! ");
    print_green("Green text! ");
    print_blue("Blue text! ");
    print_yellow("Yellow text!\n");
}

void demonstrate_status_messages(void) {
    print("System status: ");
    print_red("ERROR");
    print(" | ");
    print_green("RESOLVED");
    println("");
}

int main(void) {
    println("=== Testing MotiveSyz Print Module ===");

    demonstrate_basic_output();
    demonstrate_colored_output();
    demonstrate_status_messages();

    print_green("=== All tests completed successfully ===\n");

    return 0;
}
