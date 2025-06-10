/*
 * OBI Buffer Protocol - C Adapter
 * Direct C interface with minimal overhead
 */

#ifndef OBI_C_ADAPTER_H
#define OBI_C_ADAPTER_H

#include "obi_buffer/obi_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

/* C Adapter API */
typedef struct {
    obi_buffer_t *buffer;
    obi_validator_t *validator;
    obi_normalizer_t *normalizer;
    bool initialized;
} obi_c_context_t;

/* Context Management */
obi_result_t obi_c_context_create(obi_c_context_t **context);
obi_result_t obi_c_context_destroy(obi_c_context_t *context);

/* Message Operations */
obi_result_t obi_c_serialize_message(obi_c_context_t *context, 
                                    const char *json_data, 
                                    uint8_t **output, 
                                    size_t *output_len);

obi_result_t obi_c_deserialize_message(obi_c_context_t *context,
                                      const uint8_t *input,
                                      size_t input_len,
                                      char **json_output);

#ifdef __cplusplus
}
#endif

#endif /* OBI_C_ADAPTER_H */
