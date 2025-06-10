/*
 * OBI Buffer Protocol - Automaton Implementation
 * DFA State Minimization with USCN Isomorphic Reduction
 * Implements character-set level state minimization per Nnamdi Okpala's research
 */

#include "obibuf/core/obibuf.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* DFA Configuration Constants */
#define DFA_MAX_STATES 256
#define DFA_MAX_TRANSITIONS 1024
#define DFA_ALPHABET_SIZE 256
#define DFA_PATTERN_BUFFER_SIZE 4096

/* DFA State Types */
typedef enum {
    DFA_STATE_INITIAL = 0,
    DFA_STATE_PROCESSING,
    DFA_STATE_ACCEPTING,
    DFA_STATE_REJECTING,
    DFA_STATE_CANONICAL
} dfa_state_type_t;

/* DFA Transition Structure */
typedef struct {
    uint8_t input_char;
    int from_state;
    int to_state;
    bool is_canonical_transition;
} dfa_transition_t;

/* DFA State Structure */
typedef struct {
    int state_id;
    dfa_state_type_t type;
    bool is_accepting;
    bool is_minimal;
    char canonical_pattern[256];
    uint32_t pattern_hash;
} dfa_state_t;

/* Automaton Internal Structure */
struct obi_automaton {
    bool initialized;
    int current_state;
    int state_count;
    int transition_count;
    
    dfa_state_t states[DFA_MAX_STATES];
    dfa_transition_t transitions[DFA_MAX_TRANSITIONS];
    int transition_table[DFA_MAX_STATES][DFA_ALPHABET_SIZE];
    
    char pattern_buffer[DFA_PATTERN_BUFFER_SIZE];
    size_t buffer_position;
    uint32_t canonical_hash;
};

/* Character Classification for USCN */
typedef struct {
    char input_char;
    char canonical_char;
    bool requires_normalization;
} uscn_char_mapping_t;

/* USCN Character Mapping Table */
static const uscn_char_mapping_t uscn_mappings[] = {
    /* Whitespace normalization */
    {'\t', ' ', true},
    {'\n', ' ', true},
    {'\r', ' ', true},
    {'\v', ' ', true},
    {'\f', ' ', true},
    
    /* Case normalization */
    {'A', 'a', true}, {'B', 'b', true}, {'C', 'c', true}, {'D', 'd', true},
    {'E', 'e', true}, {'F', 'f', true}, {'G', 'g', true}, {'H', 'h', true},
    {'I', 'i', true}, {'J', 'j', true}, {'K', 'k', true}, {'L', 'l', true},
    {'M', 'm', true}, {'N', 'n', true}, {'O', 'o', true}, {'P', 'p', true},
    {'Q', 'q', true}, {'R', 'r', true}, {'S', 's', true}, {'T', 't', true},
    {'U', 'u', true}, {'V', 'v', true}, {'W', 'w', true}, {'X', 'x', true},
    {'Y', 'y', true}, {'Z', 'z', true},
    
    /* Preserve canonical characters */
    {'a', 'a', false}, {'b', 'b', false}, {'c', 'c', false}, {'d', 'd', false},
    {'e', 'e', false}, {'f', 'f', false}, {'g', 'g', false}, {'h', 'h', false},
    {'i', 'i', false}, {'j', 'j', false}, {'k', 'k', false}, {'l', 'l', false},
    {'m', 'm', false}, {'n', 'n', false}, {'o', 'o', false}, {'p', 'p', false},
    {'q', 'q', false}, {'r', 'r', false}, {'s', 's', false}, {'t', 't', false},
    {'u', 'u', false}, {'v', 'v', false}, {'w', 'w', false}, {'x', 'x', false},
    {'y', 'y', false}, {'z', 'z', false},
    
    {'0', '0', false}, {'1', '1', false}, {'2', '2', false}, {'3', '3', false},
    {'4', '4', false}, {'5', '5', false}, {'6', '6', false}, {'7', '7', false},
    {'8', '8', false}, {'9', '9', false},
    
    /* End marker */
    {'\0', '\0', false}
};

/* Hash function for pattern generation */
static uint32_t calculate_pattern_hash(const char *pattern, size_t length) {
    uint32_t hash = 0x811C9DC5; /* FNV-1a offset basis */
    const uint32_t prime = 0x01000193; /* FNV-1a prime */
    
    for (size_t i = 0; i < length; i++) {
        hash ^= (uint8_t)pattern[i];
        hash *= prime;
    }
    
    return hash;
}

/* USCN Character Normalization */
static char normalize_character(char input) {
    for (const uscn_char_mapping_t *mapping = uscn_mappings; 
         mapping->input_char != '\0'; mapping++) {
        if (mapping->input_char == input) {
            return mapping->canonical_char;
        }
    }
    
    /* If not in mapping table, return as-is (but mark as potentially non-canonical) */
    return input;
}

/* Initialize DFA states for schema validation */
static void initialize_default_states(obi_automaton_t *automaton) {
    /* State 0: Initial state */
    automaton->states[0].state_id = 0;
    automaton->states[0].type = DFA_STATE_INITIAL;
    automaton->states[0].is_accepting = false;
    automaton->states[0].is_minimal = true;
    strcpy(automaton->states[0].canonical_pattern, "INIT");
    
    /* State 1: JSON object start */
    automaton->states[1].state_id = 1;
    automaton->states[1].type = DFA_STATE_PROCESSING;
    automaton->states[1].is_accepting = false;
    automaton->states[1].is_minimal = true;
    strcpy(automaton->states[1].canonical_pattern, "JSON_START");
    
    /* State 2: Field name processing */
    automaton->states[2].state_id = 2;
    automaton->states[2].type = DFA_STATE_PROCESSING;
    automaton->states[2].is_accepting = false;
    automaton->states[2].is_minimal = true;
    strcpy(automaton->states[2].canonical_pattern, "FIELD_NAME");
    
    /* State 3: Field value processing */
    automaton->states[3].state_id = 3;
    automaton->states[3].type = DFA_STATE_PROCESSING;
    automaton->states[3].is_accepting = false;
    automaton->states[3].is_minimal = true;
    strcpy(automaton->states[3].canonical_pattern, "FIELD_VALUE");
    
    /* State 4: Canonical accepting state */
    automaton->states[4].state_id = 4;
    automaton->states[4].type = DFA_STATE_ACCEPTING;
    automaton->states[4].is_accepting = true;
    automaton->states[4].is_minimal = true;
    strcpy(automaton->states[4].canonical_pattern, "CANONICAL_ACCEPT");
    
    /* State 5: Rejection state */
    automaton->states[5].state_id = 5;
    automaton->states[5].type = DFA_STATE_REJECTING;
    automaton->states[5].is_accepting = false;
    automaton->states[5].is_minimal = true;
    strcpy(automaton->states[5].canonical_pattern, "REJECT");
    
    automaton->state_count = 6;
}

/* Build transition table for DFA */
static void build_transition_table(obi_automaton_t *automaton) {
    /* Initialize transition table to rejection state */
    for (int i = 0; i < DFA_MAX_STATES; i++) {
        for (int j = 0; j < DFA_ALPHABET_SIZE; j++) {
            automaton->transition_table[i][j] = 5; /* Reject state */
        }
    }
    
    /* Define transitions for JSON message validation */
    
    /* From INIT state (0) */
    automaton->transition_table[0]['{'] = 1; /* JSON start */
    automaton->transition_table[0][' '] = 0; /* Skip whitespace */
    automaton->transition_table[0]['\t'] = 0;
    automaton->transition_table[0]['\n'] = 0;
    automaton->transition_table[0]['\r'] = 0;
    
    /* From JSON_START state (1) */
    automaton->transition_table[1]['"'] = 2; /* Field name start */
    automaton->transition_table[1][' '] = 1; /* Skip whitespace */
    automaton->transition_table[1]['}'] = 4; /* Empty object - accept */
    
    /* From FIELD_NAME state (2) */
    for (int i = 'a'; i <= 'z'; i++) {
        automaton->transition_table[2][i] = 2; /* Field name characters */
    }
    for (int i = 'A'; i <= 'Z'; i++) {
        automaton->transition_table[2][i] = 2; /* Uppercase (will be normalized) */
    }
    for (int i = '0'; i <= '9'; i++) {
        automaton->transition_table[2][i] = 2; /* Numbers in field names */
    }
    automaton->transition_table[2]['_'] = 2; /* Underscore */
    automaton->transition_table[2]['"'] = 3; /* End field name */
    
    /* From FIELD_VALUE state (3) */
    automaton->transition_table[3][':'] = 3; /* Colon after field name */
    automaton->transition_table[3][' '] = 3; /* Whitespace */
    automaton->transition_table[3]['"'] = 3; /* String value start/end */
    for (int i = 32; i <= 126; i++) { /* Printable ASCII */
        if (i != '"' && i != '\\') {
            automaton->transition_table[3][i] = 3;
        }
    }
    automaton->transition_table[3][','] = 1; /* Next field */
    automaton->transition_table[3]['}'] = 4; /* End object - accept */
}

/* State minimization using equivalence classes */
static void minimize_dfa_states(obi_automaton_t *automaton) {
    bool equivalence_matrix[DFA_MAX_STATES][DFA_MAX_STATES];
    
    /* Initialize equivalence matrix */
    for (int i = 0; i < automaton->state_count; i++) {
        for (int j = 0; j < automaton->state_count; j++) {
            equivalence_matrix[i][j] = true; /* Assume equivalent initially */
        }
    }
    
    /* Mark obviously non-equivalent states */
    for (int i = 0; i < automaton->state_count; i++) {
        for (int j = i + 1; j < automaton->state_count; j++) {
            if (automaton->states[i].is_accepting != automaton->states[j].is_accepting) {
                equivalence_matrix[i][j] = false;
                equivalence_matrix[j][i] = false;
            }
        }
    }
    
    /* Iteratively refine equivalence classes */
    bool changed = true;
    while (changed) {
        changed = false;
        
        for (int i = 0; i < automaton->state_count; i++) {
            for (int j = i + 1; j < automaton->state_count; j++) {
                if (equivalence_matrix[i][j]) {
                    /* Check if states have equivalent transitions */
                    for (int input = 0; input < DFA_ALPHABET_SIZE; input++) {
                        int next_i = automaton->transition_table[i][input];
                        int next_j = automaton->transition_table[j][input];
                        
                        if (next_i < automaton->state_count && next_j < automaton->state_count) {
                            if (!equivalence_matrix[next_i][next_j]) {
                                equivalence_matrix[i][j] = false;
                                equivalence_matrix[j][i] = false;
                                changed = true;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    
    /* Mark minimal states based on equivalence classes */
    for (int i = 0; i < automaton->state_count; i++) {
        automaton->states[i].is_minimal = true;
        for (int j = 0; j < i; j++) {
            if (equivalence_matrix[i][j]) {
                automaton->states[i].is_minimal = false;
                break;
            }
        }
    }
}

/* Public API Implementation */

obi_result_t obi_automaton_create(obi_automaton_t **automaton) {
    if (!automaton) {
        return OBI_ERROR_INVALID_INPUT;
    }
    
    *automaton = malloc(sizeof(obi_automaton_t));
    if (!*automaton) {
        return OBI_ERROR_BUFFER_OVERFLOW;
    }
    
    /* Initialize automaton state */
    memset(*automaton, 0, sizeof(obi_automaton_t));
    (*automaton)->initialized = true;
    (*automaton)->current_state = 0;
    (*automaton)->buffer_position = 0;
    
    /* Initialize DFA states and transitions */
    initialize_default_states(*automaton);
    build_transition_table(*automaton);
    minimize_dfa_states(*automaton);
    
    return OBI_SUCCESS;
}

obi_result_t obi_automaton_destroy(obi_automaton_t *automaton) {
    if (!automaton) {
        return OBI_ERROR_INVALID_INPUT;
    }
    
    /* Clear sensitive data */
    memset(automaton->pattern_buffer, 0, DFA_PATTERN_BUFFER_SIZE);
    automaton->initialized = false;
    
    free(automaton);
    return OBI_SUCCESS;
}

obi_result_t obi_automaton_process(obi_automaton_t *automaton, obi_buffer_t *buffer) {
    if (!automaton || !buffer || !automaton->initialized) {
        return OBI_ERROR_INVALID_INPUT;
    }
    
    /* Reset automaton state */
    automaton->current_state = 0;
    automaton->buffer_position = 0;
    memset(automaton->pattern_buffer, 0, DFA_PATTERN_BUFFER_SIZE);
    
    const char *input = (const char*)buffer->data;
    size_t input_length = buffer->length;
    
    /* Process each character through DFA */
    for (size_t i = 0; i < input_length; i++) {
        char original_char = input[i];
        char canonical_char = normalize_character(original_char);
        
        /* Record canonical character in pattern buffer */
        if (automaton->buffer_position < DFA_PATTERN_BUFFER_SIZE - 1) {
            automaton->pattern_buffer[automaton->buffer_position++] = canonical_char;
        }
        
        /* Perform state transition */
        int next_state = automaton->transition_table[automaton->current_state][(uint8_t)canonical_char];
        
        if (next_state >= automaton->state_count) {
            /* Invalid transition - reject */
            automaton->current_state = 5; /* Reject state */
            return OBI_ERROR_DFA_TRANSITION_FAILED;
        }
        
        automaton->current_state = next_state;
        
        /* Check for rejection state */
        if (automaton->current_state == 5) {
            return OBI_ERROR_VALIDATION_FAILED;
        }
    }
    
    /* Verify final state is accepting */
    if (!automaton->states[automaton->current_state].is_accepting) {
        return OBI_ERROR_VALIDATION_FAILED;
    }
    
    /* Generate canonical pattern hash */
    automaton->canonical_hash = calculate_pattern_hash(
        automaton->pattern_buffer, 
        automaton->buffer_position
    );
    
    /* Store hash in buffer for audit trail */
    uint32_t hash_bytes = automaton->canonical_hash;
    for (int i = 0; i < 4 && i < OBI_HASH_SIZE; i++) {
        buffer->pattern_hash[i] = (uint8_t)(hash_bytes >> (i * 8));
    }
    
    return OBI_SUCCESS;
}

/* Diagnostic and introspection functions */

int obi_automaton_get_current_state(obi_automaton_t *automaton) {
    if (!automaton || !automaton->initialized) {
        return -1;
    }
    return automaton->current_state;
}

const char* obi_automaton_get_canonical_pattern(obi_automaton_t *automaton) {
    if (!automaton || !automaton->initialized) {
        return NULL;
    }
    return automaton->pattern_buffer;
}

uint32_t obi_automaton_get_pattern_hash(obi_automaton_t *automaton) {
    if (!automaton || !automaton->initialized) {
        return 0;
    }
    return automaton->canonical_hash;
}

bool obi_automaton_is_state_minimal(obi_automaton_t *automaton, int state_id) {
    if (!automaton || !automaton->initialized || 
        state_id < 0 || state_id >= automaton->state_count) {
        return false;
    }
    return automaton->states[state_id].is_minimal;
}

const char* obi_automaton_get_state_pattern(obi_automaton_t *automaton, int state_id) {
    if (!automaton || !automaton->initialized || 
        state_id < 0 || state_id >= automaton->state_count) {
        return NULL;
    }
    return automaton->states[state_id].canonical_pattern;
}