/*
 * OBI Buffer Protocol - Validator Implementation
 * DFA Automaton-based schema validation with Zero Trust enforcement
 * Integrates with USCN normalizer for canonical form validation
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
#include <regex.h> 


/* Validation Configuration */
#define OBI_MAX_FIELD_COUNT 64
#define OBI_MAX_FIELD_NAME_LEN 128
#define OBI_MAX_PATTERN_LEN 256
#define OBI_VALIDATION_HASH_SIZE 32

/* Field validation types */
typedef enum {
    OBI_FIELD_UINT64,
    OBI_FIELD_STRING,
    OBI_FIELD_BINARY,
    OBI_FIELD_SHA256_DIGEST,
    OBI_FIELD_TIMESTAMP
} obi_field_type_t;

/* Schema field definition */
typedef struct {
    char name[OBI_MAX_FIELD_NAME_LEN];
    obi_field_type_t type;
    bool required;
    size_t max_length;
    char validation_pattern[OBI_MAX_PATTERN_LEN];
    regex_t compiled_regex;
    bool regex_compiled;
} obi_field_schema_t;

/* Validator state structure */
struct obi_validator {
    bool initialized;
    bool zero_trust_mode;
    obi_field_schema_t fields[OBI_MAX_FIELD_COUNT];
    size_t field_count;
    obi_normalizer_t *normalizer;
    obi_automaton_t *automaton;
    uint64_t validation_count;
};

/* DFA State Transition Matrix */
typedef enum {
    DFA_STATE_INIT = 0,
    DFA_STATE_FIELD_NAME,
    DFA_STATE_FIELD_VALUE,
    DFA_STATE_VALIDATION,
    DFA_STATE_ACCEPT,
    DFA_STATE_REJECT,
    DFA_STATE_MAX
} dfa_state_t;

/* Schema validation patterns (from secure_message.yaml) */
static const obi_field_schema_t default_schema[] = {
    {"id", OBI_FIELD_UINT64, true, 0, "^[1-9][0-9]*$", {0}, false},
    {"timestamp", OBI_FIELD_TIMESTAMP, true, 0, "^[0-9]+$", {0}, false},
    {"payload", OBI_FIELD_BINARY, true, 4096, "^[A-Za-z0-9+/]*={0,2}$", {0}, false},
    {"signature", OBI_FIELD_SHA256_DIGEST, true, 64, "^[a-fA-F0-9]{64}$", {0}, false},
    {"message_type", OBI_FIELD_STRING, true, 32, "^(DATA|CONTROL|AUDIT)$", {0}, false},
    {"source_id", OBI_FIELD_STRING, true, 256, "^[A-Za-z0-9_-]+$", {0}, false}
};

#define DEFAULT_SCHEMA_COUNT (sizeof(default_schema) / sizeof(default_schema[0]))

/* DFA transition function */
static dfa_state_t dfa_transition(dfa_state_t current, char input, const obi_field_schema_t *field) {
    switch (current) {
        case DFA_STATE_INIT:
            if (input == '{' || input == '"') {
                return DFA_STATE_FIELD_NAME;
            }
            break;
            
        case DFA_STATE_FIELD_NAME:
            if (input == ':' || input == '"') {
                return DFA_STATE_FIELD_VALUE;
            }
            break;
            
        case DFA_STATE_FIELD_VALUE:
            /* Validate against field pattern */
            if (field && field->regex_compiled) {
                return DFA_STATE_VALIDATION;
            }
            break;
            
        case DFA_STATE_VALIDATION:
            return DFA_STATE_ACCEPT; /* Successful validation */
            
        default:
            break;
    }
    
    return DFA_STATE_REJECT;
}

/* Field validation functions */
static bool validate_uint64_field(const char *value) {
    if (!value || strlen(value) == 0) return false;
    
    for (const char *p = value; *p; p++) {
        if (*p < '0' || *p > '9') return false;
    }
    
    /* Check for leading zeros (not allowed for non-zero numbers) */
    if (strlen(value) > 1 && value[0] == '0') return false;
    
    return true;
}

static bool validate_timestamp_field(const char *value) {
    if (!validate_uint64_field(value)) return false;
    
    /* Additional timestamp validation */
    uint64_t timestamp = strtoull(value, NULL, 10);
    uint64_t current_time = (uint64_t)time(NULL);
    
    /* Reject timestamps too far in the future (1 year) */
    return timestamp <= current_time + (365 * 24 * 3600);
}

static bool validate_sha256_digest(const char *value) {
    if (!value || strlen(value) != 64) return false;
    
    for (size_t i = 0; i < 64; i++) {
        char c = value[i];
        if (!((c >= '0' && c <= '9') || 
              (c >= 'a' && c <= 'f') || 
              (c >= 'A' && c <= 'F'))) {
            return false;
        }
    }
    
    return true;
}

static bool validate_base64_binary(const char *value, size_t max_length) {
    if (!value) return false;
    
    size_t len = strlen(value);
    if (len > max_length) return false;
    
    /* Check base64 character set */
    size_t padding_count = 0;
    for (size_t i = 0; i < len; i++) {
        char c = value[i];
        if (c == '=') {
            padding_count++;
            continue;
        }
        if (padding_count > 0) return false; /* Padding must be at end */
        
        if (!((c >= 'A' && c <= 'Z') || 
              (c >= 'a' && c <= 'z') || 
              (c >= '0' && c <= '9') || 
              c == '+' || c == '/')) {
            return false;
        }
    }
    
    return padding_count <= 2;
}

/* Validator API Implementation */
obi_result_t obi_validator_create(obi_validator_t **validator, bool zero_trust_mode) {
    if (!validator) {
        return OBI_ERROR_INVALID_INPUT;
    }
    
    *validator = malloc(sizeof(obi_validator_t));
    if (!*validator) {
        return OBI_ERROR_BUFFER_OVERFLOW;
    }
    
    /* Initialize validator state */
    (*validator)->initialized = true;
    (*validator)->zero_trust_mode = zero_trust_mode;
    (*validator)->field_count = 0;
    (*validator)->validation_count = 0;
    
    /* Initialize normalizer */
    obi_result_t result = obi_normalizer_create(&(*validator)->normalizer);
    if (result != OBI_SUCCESS) {
        free(*validator);
        *validator = NULL;
        return result;
    }
    
    /* Initialize automaton */
    result = obi_automaton_create(&(*validator)->automaton);
    if (result != OBI_SUCCESS) {
        obi_normalizer_destroy((*validator)->normalizer);
        free(*validator);
        *validator = NULL;
        return result;
    }
    
    /* Load default schema */
    for (size_t i = 0; i < DEFAULT_SCHEMA_COUNT && i < OBI_MAX_FIELD_COUNT; i++) {
        (*validator)->fields[i] = default_schema[i];
        
        /* Compile regex patterns */
        if (strlen((*validator)->fields[i].validation_pattern) > 0) {
            int regex_result = regcomp(&(*validator)->fields[i].compiled_regex,
                                     (*validator)->fields[i].validation_pattern,
                                     REG_EXTENDED | REG_NOSUB);
            (*validator)->fields[i].regex_compiled = (regex_result == 0);
        }
    }
    (*validator)->field_count = DEFAULT_SCHEMA_COUNT;
    
    return OBI_SUCCESS;
}

obi_result_t obi_validator_destroy(obi_validator_t *validator) {
    if (!validator) {
        return OBI_ERROR_INVALID_INPUT;
    }
    
    /* Clean up compiled regexes */
    for (size_t i = 0; i < validator->field_count; i++) {
        if (validator->fields[i].regex_compiled) {
            regfree(&validator->fields[i].compiled_regex);
        }
    }
    
    /* Destroy associated components */
    if (validator->normalizer) {
        obi_normalizer_destroy(validator->normalizer);
    }
    if (validator->automaton) {
        obi_automaton_destroy(validator->automaton);
    }
    
    /* Clear sensitive data */
    memset(validator, 0, sizeof(obi_validator_t));
    free(validator);
    
    return OBI_SUCCESS;
}

obi_result_t obi_validate_buffer(obi_validator_t *validator, obi_buffer_t *buffer) {
    if (!validator || !buffer || !validator->initialized) {
        return OBI_ERROR_INVALID_INPUT;
    }
    
    /* Zero Trust enforcement */
    if (validator->zero_trust_mode && !buffer->normalized) {
        return OBI_ERROR_ZERO_TRUST_VIOLATION;
    }
    
    /* Phase 1: Normalize buffer content */
    obi_result_t result = obi_normalize_buffer(validator->normalizer, buffer);
    if (result != OBI_SUCCESS) {
        return result;
    }
    
    /* Phase 2: DFA automaton processing */
    result = obi_automaton_process(validator->automaton, buffer);
    if (result != OBI_SUCCESS) {
        return result;
    }
    
    /* Phase 3: Field-level validation */
    const char *json_data = (const char*)buffer->data;
    bool field_found[OBI_MAX_FIELD_COUNT] = {false};
    
    /* Parse JSON fields (simplified parser for validation) */
    for (size_t field_idx = 0; field_idx < validator->field_count; field_idx++) {
        const obi_field_schema_t *field = &validator->fields[field_idx];
        
        /* Search for field in normalized JSON */
        char field_pattern[OBI_MAX_PATTERN_LEN];
        snprintf(field_pattern, sizeof(field_pattern), "\"%s\":", field->name);
        
        const char *field_pos = strstr(json_data, field_pattern);
        if (field_pos) {
            field_found[field_idx] = true;
            
            /* Extract field value for validation */
            const char *value_start = field_pos + strlen(field_pattern);
            while (*value_start == ' ' || *value_start == '"') value_start++;
            
            const char *value_end = value_start;
            while (*value_end && *value_end != '"' && *value_end != ',' && 
                   *value_end != '}') value_end++;
            
            size_t value_len = value_end - value_start;
            char field_value[OBI_MAX_PATTERN_LEN];
            if (value_len < sizeof(field_value)) {
                strncpy(field_value, value_start, value_len);
                field_value[value_len] = '\0';
                
                /* Type-specific validation */
                bool valid = false;
                switch (field->type) {
                    case OBI_FIELD_UINT64:
                        valid = validate_uint64_field(field_value);
                        break;
                    case OBI_FIELD_TIMESTAMP:
                        valid = validate_timestamp_field(field_value);
                        break;
                    case OBI_FIELD_SHA256_DIGEST:
                        valid = validate_sha256_digest(field_value);
                        break;
                    case OBI_FIELD_BINARY:
                        valid = validate_base64_binary(field_value, field->max_length);
                        break;
                    case OBI_FIELD_STRING:
                        valid = (strlen(field_value) <= field->max_length);
                        break;
                }
                
                /* Regex pattern validation */
                if (valid && field->regex_compiled) {
                    valid = (regexec(&field->compiled_regex, field_value, 0, NULL, 0) == 0);
                }
                
                if (!valid) {
                    return OBI_ERROR_VALIDATION_FAILED;
                }
            }
        }
    }
    
    /* Phase 4: Required field enforcement */
    for (size_t i = 0; i < validator->field_count; i++) {
        if (validator->fields[i].required && !field_found[i]) {
            return OBI_ERROR_VALIDATION_FAILED;
        }
    }
    
    /* Phase 5: Generate validation audit */
    uint8_t validation_hash[OBI_VALIDATION_HASH_SIZE];
    result = obi_generate_canonical_hash(validator->normalizer, 
                                       validation_hash, 
                                       OBI_VALIDATION_HASH_SIZE);
    if (result != OBI_SUCCESS) {
        return result;
    }
    
    /* Log validation event */
    result = obi_audit_log_operation("BUFFER_VALIDATION", 
                                   validation_hash, 
                                   OBI_VALIDATION_HASH_SIZE);
    if (result != OBI_SUCCESS) {
        return OBI_ERROR_AUDIT_REQUIRED;
    }
    
    /* Mark buffer as validated */
    buffer->validated = true;
    validator->validation_count++;
    
    return OBI_SUCCESS;
}

/* Schema introspection functions */
size_t obi_validator_get_field_count(obi_validator_t *validator) {
    if (!validator || !validator->initialized) {
        return 0;
    }
    return validator->field_count;
}

const char* obi_validator_get_field_name(obi_validator_t *validator, size_t index) {
    if (!validator || !validator->initialized || index >= validator->field_count) {
        return NULL;
    }
    return validator->fields[index].name;
}

bool obi_validator_is_zero_trust_enabled(obi_validator_t *validator) {
    if (!validator || !validator->initialized) {
        return false;
    }
    return validator->zero_trust_mode;
}

uint64_t obi_validator_get_validation_count(obi_validator_t *validator) {
    if (!validator || !validator->initialized) {
        return 0;
    }
    return validator->validation_count;
}