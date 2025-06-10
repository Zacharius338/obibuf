/*
 * OBI Buffer Protocol - Validator Tests
 */

#include "obi_buffer/obi_buffer.h"
#include <stdio.h>
#include <assert.h>

void test_validator_creation() {
    printf("Testing validator creation...\n");
    
    obi_validator_t *validator = NULL;
    obi_result_t result = obi_validator_create(&validator, true); // Zero Trust mode
    
    assert(result == OBI_SUCCESS);
    assert(validator != NULL);
    
    printf("✅ Validator creation test passed\n");
}

int main() {
    printf("🧪 Running OBI Buffer Validator Tests\n");
    
    // Initialize library
    obi_result_t init_result = obi_init();
    if (init_result != OBI_SUCCESS) {
        fprintf(stderr, "Failed to initialize OBI Buffer\n");
        return 1;
    }
    
    test_validator_creation();
    
    obi_cleanup();
    printf("✅ All validator tests passed!\n");
    return 0;
}
