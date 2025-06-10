/*
 * OBI Buffer Protocol - Validator Implementation
 * Zero Trust Architecture Enforcement
 * Mathematical Foundation: AEGIS-PROOF-1.2
 */

#include "obiprotocol.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* Internal Validator Structure */
struct obi_validator {
    obi_validation_context_t context;
    obi_pattern_registry_t *pattern_registry;
    obi_audit_t *audit;
    bool initialized;
    uint64_t validation_count;
};

/* Mathematical Functions from AEGIS-PROOF-1.2 */

double obi_kl_divergence(const double *p, const double *q, size_t n) {
    if (!p || !q || n == 0) return -1.0;
    
    double kl_div = 0.0;
    for (size_t i = 0; i < n; i++) {
        if (p[i] <= 0.0) continue;
        
        double q_safe = (q[i] < OBI_EPSILON_MIN) ? OBI_EPSILON_MIN : q[i];
        kl_div += p[i] * log2(p[i] / q_safe);
    }
    
    return kl_div;
}

double obi_entropy_change(double entropy_i, double entropy_j) {
    return entropy_i - entropy_j;
}

double obi_calculate_traversal_cost(const double *pi, const double *pj, size_t n, 
                                   double alpha, double beta) {
    if (!pi || !pj || n == 0) return -1.0;
    
    /* Validate parameters (Theorem 1 constraints) */
    if (alpha < 0.0 || beta < 0.0 || (alpha + beta) > 1.0001) {
        return -1.0;  /* Parameter constraint violation */
    }
    
    double kl_component = obi_kl_divergence(pi, pj, n);
    if (kl_component < 0.0) return -1.0;
    
    /* For entropy change, we need entropy values */
    double entropy_i = 0.0, entropy_j = 0.0;
    for (size_t i = 0; i < n; i++) {
        if (pi[i] > 0.0) entropy_i -= pi[i] * log2(pi[i]);
        if (pj[i] > 0.0) entropy_j -= pj[i] * log2(pj[i]);
    }
    
    double entropy_delta = obi_entropy_change(entropy_i, entropy_j);
    
    /* Cost function: C = α·KL(Pi||Pj) + β·ΔH(Si,j) */
    return alpha * kl_component + beta * entropy_delta;
}

/* Governance Assessment */
obi_governance_zone_t obi_assess_governance_zone(double cost) {
    if (cost <= OBI_COST_THRESHOLD) {
        return OBI_ZONE_AUTONOMOUS;
    } else if (cost <= OBI_WARNING_THRESHOLD) {
        return OBI_ZONE_WARNING;
    } else {
        return OBI_ZONE_GOVERNANCE;
    }
}

/* Validator Implementation */

obi_result_t obi_validator_create(obi_validator_t **validator, 
                                 const obi_validation_context_t *context) {
    if (!validator || !context) {
        return OBI_ERROR_INVALID_INPUT;
    }
    
    *validator = (obi_validator_t*)calloc(1, sizeof(obi_validator_t));
    if (!*validator) {
        return OBI_ERROR_BUFFER_OVERFLOW;
    }
    
    /* Copy and validate context */
    (*validator)->context = *context;
    
    /* Enforce mathematical constraints */
    if ((*validator)->context.alpha < 0.0 || (*validator)->context.beta < 0.0 ||
        ((*validator)->context.alpha + (*validator)->context.beta) > 1.0001) {
        free(*validator);
        *validator = NULL;
        return OBI_ERROR_NUMERICAL_INSTABILITY;
    }
    
    /* Zero Trust enforcement check */
    if (!context->zero_trust_enforced) {
        free(*validator);
        *validator = NULL;
        return OBI_ERROR_ZERO_TRUST_VIOLATION;
    }
    
    (*validator)->initialized = true;
    (*validator)->validation_count = 0;
    
    return OBI_SUCCESS;
}

obi_result_t obi_validator_destroy(obi_validator_t *validator) {
    if (!validator) {
        return OBI_ERROR_INVALID_INPUT;
    }
    
    /* Audit cleanup if audit trail exists */
    if (validator->audit) {
        obi_audit_record_t cleanup_record = {
            .timestamp = (uint64_t)time(NULL),
            .result = OBI_SUCCESS
        };
        strncpy(cleanup_record.context, "VALIDATOR_CLEANUP", sizeof(cleanup_record.context) - 1);
        
        obi_audit_log(validator->audit, &cleanup_record);
    }
    
    memset(validator, 0, sizeof(obi_validator_t));
    free(validator);
    
    return OBI_SUCCESS;
}

static obi_result_t validate_buffer_structure(const obi_buffer_t *buffer) {
    if (!buffer) {
        return OBI_ERROR_INVALID_INPUT;
    }
    
    /* Check buffer length constraints */
    if (buffer->length == 0 || buffer->length > OBI_MAX_BUFFER_SIZE) {
        return OBI_ERROR_BUFFER_OVERFLOW;
    }
    
    /* Verify security level is valid */
    if (buffer->security_level > OBI_SECURITY_CRITICAL) {
        return OBI_ERROR_VALIDATION_FAILED;
    }
    
    return OBI_SUCCESS;
}

static obi_result_t validate_mathematical_properties(obi_validator_t *validator, 
                                                    obi_buffer_t *buffer) {
    /* For demonstration, create probability distributions from buffer data */
    size_t n = (buffer->length < 16) ? buffer->length : 16;
    double pi[16] = {0}, pj[16] = {0};
    
    /* Normalize buffer data to probability distribution */
    double sum = 0.0;
    for (size_t i = 0; i < n; i++) {
        pi[i] = (double)(buffer->data[i] + 1) / 256.0;  /* Avoid zero probabilities */
        sum += pi[i];
    }
    
    /* Normalize to valid probability distribution */
    for (size_t i = 0; i < n; i++) {
        pi[i] /= sum;
        pj[i] = 1.0 / n;  /* Uniform distribution for comparison */
    }
    
    /* Calculate traversal cost using AEGIS-PROOF-1.2 */
    double cost = obi_calculate_traversal_cost(pi, pj, n, 
                                              validator->context.alpha,
                                              validator->context.beta);
    
    if (cost < 0.0) {
        return OBI_ERROR_NUMERICAL_INSTABILITY;
    }
    
    buffer->cost_value = cost;
    buffer->governance_zone = obi_assess_governance_zone(cost);
    
    /* Sinphasé governance check */
    if (buffer->governance_zone == OBI_ZONE_GOVERNANCE) {
        return OBI_ERROR_SINPHASE_VIOLATION;
    }
    
    return OBI_SUCCESS;
}

static obi_result_t enforce_canonical_validation(obi_validator_t *validator,
                                               obi_buffer_t *buffer) {
    /* Zero Trust: Only canonical forms allowed */
    if (!validator->context.canonical_only) {
        return OBI_ERROR_ZERO_TRUST_VIOLATION;
    }
    
    /* Check if buffer is normalized (USCN requirement) */
    if (!buffer->normalized) {
        return OBI_ERROR_VALIDATION_FAILED;
    }
    
    return OBI_SUCCESS;
}

obi_result_t obi_validate_buffer(obi_validator_t *validator, obi_buffer_t *buffer) {
    if (!validator || !buffer) {
        return OBI_ERROR_INVALID_INPUT;
    }
    
    if (!validator->initialized) {
        return OBI_ERROR_VALIDATION_FAILED;
    }
    
    obi_result_t result = OBI_SUCCESS;
    obi_audit_record_t audit_record = {
        .timestamp = (uint64_t)time(NULL)
    };
    
    /* Step 1: Structural validation */
    result = validate_buffer_structure(buffer);
    if (result != OBI_SUCCESS) {
        audit_record.result = result;
        goto audit_and_return;
    }
    
    /* Step 2: Zero Trust canonical enforcement */
    result = enforce_canonical_validation(validator, buffer);
    if (result != OBI_SUCCESS) {
        audit_record.result = result;
        goto audit_and_return;
    }
    
    /* Step 3: Mathematical property validation */
    result = validate_mathematical_properties(validator, buffer);
    if (result != OBI_SUCCESS) {
        audit_record.result = result;
        goto audit_and_return;
    }
    
    /* Success: Mark buffer as validated */
    buffer->validated = true;
    validator->validation_count++;
    
    audit_record.result = OBI_SUCCESS;
    strncpy(audit_record.context, "VALIDATION_SUCCESS", sizeof(audit_record.context) - 1);
    
audit_and_return:
    /* NASA compliance: Mandatory audit trail */
    if (validator->audit) {
        obi_audit_log(validator->audit, &audit_record);
    }
    
    return result;
}

/* Compliance Checks */

bool obi_check_sinphase_compliance(const obi_buffer_t *buffer) {
    if (!buffer) return false;
    
    /* Single-pass compilation principle: buffer should be processed once */
    return buffer->validated && 
           buffer->normalized && 
           buffer->governance_zone != OBI_ZONE_GOVERNANCE;
}

bool obi_check_nasa_compliance(const obi_buffer_t *buffer) {
    if (!buffer) return false;
    
    /* NASA-STD-8739.8 requirements */
    return buffer->validated &&
           buffer->length > 0 &&
           buffer->length <= OBI_MAX_BUFFER_SIZE &&
           buffer->cost_value >= 0.0;
}

/* Utility Functions */

const char* obi_result_to_string(obi_result_t result) {
    switch (result) {
        case OBI_SUCCESS: return "SUCCESS";
        case OBI_ERROR_INVALID_INPUT: return "INVALID_INPUT";
        case OBI_ERROR_VALIDATION_FAILED: return "VALIDATION_FAILED";
        case OBI_ERROR_AUDIT_REQUIRED: return "AUDIT_REQUIRED";
        case OBI_ERROR_ZERO_TRUST_VIOLATION: return "ZERO_TRUST_VIOLATION";
        case OBI_ERROR_BUFFER_OVERFLOW: return "BUFFER_OVERFLOW";
        case OBI_ERROR_NUMERICAL_INSTABILITY: return "NUMERICAL_INSTABILITY";
        case OBI_ERROR_SINPHASE_VIOLATION: return "SINPHASE_VIOLATION";
        default: return "UNKNOWN_ERROR";
    }
}