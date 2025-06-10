/*
 * OBI Buffer Protocol - Core Header
 * OBINexus Computing - Protocol Engine Division
 * 
 * Polyglot-safe, schema-driven protocol engine
 * Zero Trust architecture with mandatory validation
 */

#ifndef OBI_BUFFER_H
#define OBI_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Version Information */
#define OBI_BUFFER_VERSION_MAJOR 1
#define OBI_BUFFER_VERSION_MINOR 0
#define OBI_BUFFER_VERSION_PATCH 0

/* Protocol Constants */
#define OBI_MAX_BUFFER_SIZE 8192
#define OBI_MAX_SCHEMA_FIELDS 64
#define OBI_HASH_SIZE 32

/* Result Codes */
typedef enum {
    OBI_SUCCESS = 0,
    OBI_ERROR_INVALID_INPUT,
    OBI_ERROR_VALIDATION_FAILED,
    OBI_ERROR_NORMALIZATION_FAILED,
    OBI_ERROR_AUDIT_REQUIRED,
    OBI_ERROR_ZERO_TRUST_VIOLATION,
    OBI_ERROR_BUFFER_OVERFLOW,
    OBI_ERROR_SCHEMA_MISMATCH
} obi_result_t;

/* Forward declarations */
typedef struct obi_buffer obi_buffer_t;
typedef struct obi_schema obi_schema_t;
typedef struct obi_automaton obi_automaton_t;
typedef struct obi_validator obi_validator_t;
typedef struct obi_normalizer obi_normalizer_t;

/* Core API Functions */
obi_result_t obi_init(void);
void obi_cleanup(void);

/* Buffer Management */
obi_result_t obi_buffer_create(obi_buffer_t **buffer);
obi_result_t obi_buffer_destroy(obi_buffer_t *buffer);
obi_result_t obi_buffer_set_data(obi_buffer_t *buffer, const uint8_t *data, size_t length);

/* Schema Operations */
obi_result_t obi_schema_load_yaml(const char *yaml_path, obi_schema_t **schema);
obi_result_t obi_schema_validate_buffer(obi_schema_t *schema, obi_buffer_t *buffer);

/* Protocol State Machine */
obi_result_t obi_automaton_create(obi_automaton_t **automaton);
obi_result_t obi_automaton_process(obi_automaton_t *automaton, obi_buffer_t *buffer);

/* Normalization (USCN Implementation) */
obi_result_t obi_normalizer_create(obi_normalizer_t **normalizer);
obi_result_t obi_normalize_buffer(obi_normalizer_t *normalizer, obi_buffer_t *buffer);

/* Validation */
obi_result_t obi_validator_create(obi_validator_t **validator, bool zero_trust_mode);
obi_result_t obi_validate_buffer(obi_validator_t *validator, obi_buffer_t *buffer);

/* Audit Trail */
obi_result_t obi_audit_log_operation(const char *operation, const uint8_t *hash, size_t hash_len);

/* Utility Functions */
const char* obi_result_to_string(obi_result_t result);
bool obi_is_zero_trust_enforced(void);

#ifdef __cplusplus
}
#endif

#endif /* OBI_BUFFER_H */
