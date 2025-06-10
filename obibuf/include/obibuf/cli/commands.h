/*
 * OBI Buffer Protocol - CLI Commands Header
 */

#ifndef OBIBUF_CLI_COMMANDS_H
#define OBIBUF_CLI_COMMANDS_H

#include "obibuf/core/obibuf.h"

#ifdef __cplusplus
extern "C" {
#endif

/* CLI command structure */
typedef struct {
    const char *name;
    const char *description;
    int (*handler)(int argc, char *argv[]);
} obi_cli_command_t;

/* Command handlers */
int cmd_validate(int argc, char *argv[]);
int cmd_normalize(int argc, char *argv[]);
int cmd_audit(int argc, char *argv[]);
int cmd_version(int argc, char *argv[]);
int cmd_help(int argc, char *argv[]);

/* Utility functions */
void print_usage(const char *program_name);
void print_version(void);

#ifdef __cplusplus
}
#endif

#endif /* OBIBUF_CLI_COMMANDS_H */
