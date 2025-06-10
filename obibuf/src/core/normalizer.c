/*
 * OBI Buffer Protocol - Normalizer Implementation
 * Unicode-Only Structural Charset Normalizer (USCN)
 * Implements isomorphic reduction principles for canonical form enforcement
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include "obibuf/core/obibuf.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
 #include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* USCN Configuration Constants */
#define USCN_MAX_MAPPINGS 256
#define USCN_CANONICAL_BUFFER_SIZE 8192
#define USCN_PATTERN_HASH_SIZE 32

/* Canonical Character Mapping Table */
typedef struct {
    const char *encoded_form;
    const char *canonical_form;
    size_t encoded_len;
    size_t canonical_len;
} uscn_mapping_t;

/* Predefined USCN Mappings (from schema) */
static const uscn_mapping_t uscn_mappings[] = {
    /* Path traversal normalization */
    {"%2e%2e%2f", "../", 9, 3},
    {"%c0%af", "../", 6, 3},
    {".%2e/", "../", 5, 3},
    {"%2e%2e/", "../", 7, 3},
    
    /* Character normalization */
    {"%2f", "/", 3, 1},
    {"%2e", ".", 3, 1},
    {"%20", " ", 3, 1},
    
    /* Unicode overlong encodings */
    {"%c0%ae", ".", 6, 1},
    {"%c0%af", "/", 6, 1},
    
    /* End marker */
    {NULL, NULL, 0, 0}
};

/* Normalizer internal state */
struct obi_normalizer {
    bool initialized;
    bool case_sensitive;
    bool whitespace_normalize;
    char canonical_buffer[USCN_CANONICAL_BUFFER_SIZE];
    size_t buffer_used;
};

/* Character classification functions */
static bool is_hex_digit(char c) {
    return (c >= '0' && c <= '9') || 
           (c >= 'A' && c <= 'F') || 
           (c >= 'a' && c <= 'f');
}

static bool is_percent_encoded(const char *str, size_t pos, size_t len) {
    return (pos + 2 < len) && 
           (str[pos] == '%') && 
           is_hex_digit(str[pos + 1]) && 
           is_hex_digit(str[pos + 2]);
}

/* Core USCN reduction functions */
static size_t apply_character_mappings(const char *input, size_t input_len, 
                                     char *output, size_t output_max) {
    size_t input_pos = 0;
    size_t output_pos = 0;
    
    while (input_pos < input_len && output_pos < output_max - 1) {
        bool mapped = false;
        
        /* Check for multi-character mappings first */
        for (const uscn_mapping_t *mapping = uscn_mappings; 
             mapping->encoded_form != NULL; mapping++) {
            
            if (input_pos + mapping->encoded_len <= input_len &&
                memcmp(input + input_pos, mapping->encoded_form, 
                       mapping->encoded_len) == 0) {
                
                /* Apply canonical mapping */
                if (output_pos + mapping->canonical_len < output_max) {
                    memcpy(output + output_pos, mapping->canonical_form, 
                           mapping->canonical_len);
                    output_pos += mapping->canonical_len;
                    input_pos += mapping->encoded_len;
                    mapped = true;
                    break;
                }
            }
        }
        
        /* If no mapping found, copy character directly */
        if (!mapped) {
            output[output_pos++] = input[input_pos++];
        }
    }
    
    output[output_pos] = '\0';
    return output_pos;
}

static void normalize_case(char *str, size_t len, bool case_sensitive) {
    if (case_sensitive) {
        return; /* No case normalization */
    }
    
    for (size_t i = 0; i < len; i++) {
        str[i] = tolower(str[i]);
    }
}

static size_t normalize_whitespace(char *str, size_t len) {
    size_t write_pos = 0;
    bool in_whitespace = false;
    
    for (size_t i = 0; i < len; i++) {
        if (isspace(str[i])) {
            if (!in_whitespace) {
                str[write_pos++] = ' '; /* Normalize to single space */
                in_whitespace = true;
            }
        } else {
            str[write_pos++] = str[i];
            in_whitespace = false;
        }
    }
    
    /* Trim trailing whitespace */
    while (write_pos > 0 && str[write_pos - 1] == ' ') {
        write_pos--;
    }
    
    str[write_pos] = '\0';
    return write_pos;
}

/* Public API Implementation */
obi_result_t obi_normalizer_create(obi_normalizer_t **normalizer) {
    if (!normalizer) {
        return OBI_ERROR_INVALID_INPUT;
    }
    
    *normalizer = malloc(sizeof(obi_normalizer_t));
    if (!*normalizer) {
        return OBI_ERROR_BUFFER_OVERFLOW;
    }
    
    /* Initialize with schema defaults */
    (*normalizer)->initialized = true;
    (*normalizer)->case_sensitive = false; /* From YAML: case_sensitivity: false */
    (*normalizer)->whitespace_normalize = true;
    (*normalizer)->buffer_used = 0;
    memset((*normalizer)->canonical_buffer, 0, USCN_CANONICAL_BUFFER_SIZE);
    
    return OBI_SUCCESS;
}

obi_result_t obi_normalizer_destroy(obi_normalizer_t *normalizer) {
    if (!normalizer) {
        return OBI_ERROR_INVALID_INPUT;
    }
    
    /* Clear sensitive data */
    memset(normalizer->canonical_buffer, 0, USCN_CANONICAL_BUFFER_SIZE);
    normalizer->initialized = false;
    
    free(normalizer);
    return OBI_SUCCESS;
}

obi_result_t obi_normalize_buffer(obi_normalizer_t *normalizer, obi_buffer_t *buffer) {
    if (!normalizer || !buffer || !normalizer->initialized) {
        return OBI_ERROR_INVALID_INPUT;
    }
    
    /* Phase 1: Character mapping reduction */
    char temp_buffer[USCN_CANONICAL_BUFFER_SIZE];
    size_t normalized_len = apply_character_mappings(
        (const char*)buffer->data, buffer->length,
        temp_buffer, USCN_CANONICAL_BUFFER_SIZE
    );
    
    if (normalized_len == 0) {
        return OBI_ERROR_NORMALIZATION_FAILED;
    }
    
    /* Phase 2: Case normalization */
    normalize_case(temp_buffer, normalized_len, normalizer->case_sensitive);
    
    /* Phase 3: Whitespace normalization */
    if (normalizer->whitespace_normalize) {
        normalized_len = normalize_whitespace(temp_buffer, normalized_len);
    }
    
    /* Store canonical form in normalizer */
    if (normalized_len >= USCN_CANONICAL_BUFFER_SIZE) {
        return OBI_ERROR_BUFFER_OVERFLOW;
    }
    
    memcpy(normalizer->canonical_buffer, temp_buffer, normalized_len);
    normalizer->canonical_buffer[normalized_len] = '\0';
    normalizer->buffer_used = normalized_len;
    
    /* Update buffer with canonical form */
    if (normalized_len > buffer->max_size) {
        return OBI_ERROR_BUFFER_OVERFLOW;
    }
    
    memcpy(buffer->data, normalizer->canonical_buffer, normalized_len);
    buffer->length = normalized_len;
    buffer->normalized = true;
    
    return OBI_SUCCESS;
}

/* USCN Pattern Hash Generation */
obi_result_t obi_generate_canonical_hash(obi_normalizer_t *normalizer, 
                                        uint8_t *hash_output, 
                                        size_t hash_size) {
    if (!normalizer || !hash_output || hash_size < USCN_PATTERN_HASH_SIZE) {
        return OBI_ERROR_INVALID_INPUT;
    }
    
    if (!normalizer->initialized || normalizer->buffer_used == 0) {
        return OBI_ERROR_NORMALIZATION_FAILED;
    }
    
    /* Simple hash for canonical pattern (production would use SHA-256) */
    uint32_t hash = 0x811C9DC5; /* FNV-1a offset basis */
    const uint32_t prime = 0x01000193; /* FNV-1a prime */
    
    for (size_t i = 0; i < normalizer->buffer_used; i++) {
        hash ^= (uint8_t)normalizer->canonical_buffer[i];
        hash *= prime;
    }
    
    /* Convert to byte array */
    for (size_t i = 0; i < 4 && i < hash_size; i++) {
        hash_output[i] = (uint8_t)(hash >> (i * 8));
    }
    
    /* Zero remaining bytes */
    for (size_t i = 4; i < hash_size; i++) {
        hash_output[i] = 0;
    }
    
    return OBI_SUCCESS;
}

/* Diagnostic functions for testing */
const char* obi_get_canonical_form(obi_normalizer_t *normalizer) {
    if (!normalizer || !normalizer->initialized) {
        return NULL;
    }
    return normalizer->canonical_buffer;
}

size_t obi_get_canonical_length(obi_normalizer_t *normalizer) {
    if (!normalizer || !normalizer->initialized) {
        return 0;
    }
    return normalizer->buffer_used;
}