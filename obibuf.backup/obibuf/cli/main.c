/*
 * OBI Buffer Protocol - CLI Interface
 * Command-line tools for validation, testing, and debugging
 */

#include "obi_buffer/obi_buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_usage(const char *program_name) {
    printf("OBI Buffer Protocol CLI v%d.%d.%d\n", 
           OBI_BUFFER_VERSION_MAJOR, 
           OBI_BUFFER_VERSION_MINOR, 
           OBI_BUFFER_VERSION_PATCH);
    printf("Usage: %s <command> [options]\n\n", program_name);
    printf("Commands:\n");
    printf("  validate <file>     Validate buffer against schema\n");
    printf("  normalize <file>    Apply USCN normalization\n");
    printf("  audit <file>        Generate audit log\n");
    printf("  version             Show version information\n");
    printf("  help                Show this help message\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    // Initialize OBI Buffer library
    obi_result_t result = obi_init();
    if (result != OBI_SUCCESS) {
        fprintf(stderr, "Failed to initialize OBI Buffer: %s\n", 
                obi_result_to_string(result));
        return 1;
    }

    const char *command = argv[1];

    if (strcmp(command, "version") == 0) {
        printf("OBI Buffer Protocol v%d.%d.%d\n", 
               OBI_BUFFER_VERSION_MAJOR, 
               OBI_BUFFER_VERSION_MINOR, 
               OBI_BUFFER_VERSION_PATCH);
        printf("Zero Trust: %s\n", obi_is_zero_trust_enforced() ? "Enabled" : "Disabled");
    } else if (strcmp(command, "help") == 0) {
        print_usage(argv[0]);
    } else if (strcmp(command, "validate") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: validate command requires file argument\n");
            return 1;
        }
        printf("Validation command not yet implemented for: %s\n", argv[2]);
    } else {
        fprintf(stderr, "Unknown command: %s\n", command);
        print_usage(argv[0]);
        return 1;
    }

    obi_cleanup();
    return 0;
}
