/*
 * OBI Buffer Protocol - CLI Implementation
 * OBINexus Computing - Aegis Framework
 * 
 * Command-line interface for OBI protocol operations
 * Links against libobiprotocol.a for Zero Trust enforcement
 */

#include "obiprotocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>

#define CLI_VERSION "1.0.0"
#define MAX_FILE_SIZE 8192

/* CLI Command Structure */
typedef enum {
    CMD_VALIDATE,
    CMD_NORMALIZE,
    CMD_AUDIT,
    CMD_BENCHMARK,
    CMD_VERSION,
    CMD_HELP,
    CMD_UNKNOWN
} cli_command_t;

typedef struct {
    cli_command_t command;
    char *input_file;
    char *output_file;
    char *schema_file;
    char *audit_log;
    bool verbose;
    bool zero_trust;
    bool nasa_compliance;
    double alpha;
    double beta;
} cli_options_t;

/* Function Prototypes */
static void print_usage(const char *program_name);
static void print_version(void);
static cli_command_t parse_command(const char *cmd);
static int parse_options(int argc, char *argv[], cli_options_t *options);
static int cmd_validate(const cli_options_t *options);
static int cmd_normalize(const cli_options_t *options);
static int cmd_audit(const cli_options_t *options);
static int cmd_benchmark(const cli_options_t *options);
static obi_result_t load_file_to_buffer(const char *filename, obi_buffer_t *buffer);
static obi_result_t save_buffer_to_file(const char *filename, const obi_buffer_t *buffer);

/* Main Entry Point */
int main(int argc, char *argv[]) {
    cli_options_t options = {
        .command = CMD_UNKNOWN,
        .input_file = NULL,
        .output_file = NULL,
        .schema_file = "schema/obiprotocol_schema.yaml",
        .audit_log = "audit.log",
        .verbose = false,
        .zero_trust = true,  /* Default to Zero Trust enforcement */
        .nasa_compliance = true,
        .alpha = OBI_ALPHA_DEFAULT,
        .beta = OBI_BETA_DEFAULT
    };
    
    /* Parse command line arguments */
    if (parse_options(argc, argv, &options) != 0) {
        return EXIT_FAILURE;
    }
    
    /* Initialize OBI protocol */
    obi_result_t init_result = obi_init();
    if (init_result != OBI_SUCCESS) {
        fprintf(stderr, "Error: Failed to initialize OBI protocol: %s\n", 
                obi_result_to_string(init_result));
        return EXIT_FAILURE;
    }
    
    int result = EXIT_SUCCESS;
    
    /* Execute command */
    switch (options.command) {
        case CMD_VALIDATE:
            result = cmd_validate(&options);
            break;
            
        case CMD_NORMALIZE:
            result = cmd_normalize(&options);
            break;
            
        case CMD_AUDIT:
            result = cmd_audit(&options);
            break;
            
        case CMD_BENCHMARK:
            result = cmd_benchmark(&options);
            break;
            
        case CMD_VERSION:
            print_version();
            break;
            
        case CMD_HELP:
            print_usage(argv[0]);
            break;
            
        default:
            fprintf(stderr, "Error: Unknown command\n");
            print_usage(argv[0]);
            result = EXIT_FAILURE;
    }
    
    /* Cleanup */
    obi_cleanup();
    
    return result;
}

/* Parse command line options */
static int parse_options(int argc, char *argv[], cli_options_t *options) {
    if (argc < 2) {
        print_usage(argv[0]);
        return -1;
    }
    
    /* First argument is the command */
    options->command = parse_command(argv[1]);
    
    static struct option long_options[] = {
        {"input",      required_argument, 0, 'i'},
        {"output",     required_argument, 0, 'o'},
        {"schema",     required_argument, 0, 's'},
        {"audit-log",  required_argument, 0, 'a'},
        {"verbose",    no_argument,       0, 'v'},
        {"no-zero-trust", no_argument,    0, 'z'},
        {"no-nasa",    no_argument,       0, 'n'},
        {"alpha",      required_argument, 0, 'A'},
        {"beta",       required_argument, 0, 'B'},
        {"help",       no_argument,       0, 'h'},
        {"version",    no_argument,       0, 'V'},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    /* Skip command argument */
    optind = 2;
    
    while ((opt = getopt_long(argc, argv, "i:o:s:a:vznA:B:hV", 
                             long_options, &option_index)) != -1) {
        switch (opt) {
            case 'i':
                options->input_file = optarg;
                break;
            case 'o':
                options->output_file = optarg;
                break;
            case 's':
                options->schema_file = optarg;
                break;
            case 'a':
                options->audit_log = optarg;
                break;
            case 'v':
                options->verbose = true;
                break;
            case 'z':
                options->zero_trust = false;
                fprintf(stderr, "Warning: Zero Trust enforcement disabled\n");
                break;
            case 'n':
                options->nasa_compliance = false;
                fprintf(stderr, "Warning: NASA compliance checking disabled\n");
                break;
            case 'A':
                options->alpha = strtod(optarg, NULL);
                if (options->alpha < 0.0 || options->alpha > 1.0) {
                    fprintf(stderr, "Error: Alpha must be between 0.0 and 1.0\n");
                    return -1;
                }
                break;
            case 'B':
                options->beta = strtod(optarg, NULL);
                if (options->beta < 0.0 || options->beta > 1.0) {
                    fprintf(stderr, "Error: Beta must be between 0.0 and 1.0\n");
                    return -1;
                }
                break;
            case 'h':
                print_usage(argv[0]);
                exit(EXIT_SUCCESS);
            case 'V':
                print_version();
                exit(EXIT_SUCCESS);
            default:
                print_usage(argv[0]);
                return -1;
        }
    }
    
    /* Validate mathematical constraints */
    if ((options->alpha + options->beta) > 1.0001) {
        fprintf(stderr, "Error: Alpha + Beta must not exceed 1.0\n");
        return -1;
    }
    
    return 0;
}

/* Validate Command Implementation */
static int cmd_validate(const cli_options_t *options) {
    if (!options->input_file) {
        fprintf(stderr, "Error: Input file required for validation\n");
        return EXIT_FAILURE;
    }
    
    if (options->verbose) {
        printf("Validating buffer: %s\n", options->input_file);
        printf("Zero Trust: %s\n", options->zero_trust ? "Enabled" : "Disabled");
        printf("NASA Compliance: %s\n", options->nasa_compliance ? "Enabled" : "Disabled");
        printf("Mathematical params: α=%.3f, β=%.3f\n", options->alpha, options->beta);
    }
    
    /* Create buffer and load data */
    obi_buffer_t *buffer;
    obi_result_t result = obi_buffer_create(&buffer);
    if (result != OBI_SUCCESS) {
        fprintf(stderr, "Error: Failed to create buffer: %s\n", 
                obi_result_to_string(result));
        return EXIT_FAILURE;
    }
    
    /* Load file into buffer */
    result = load_file_to_buffer(options->input_file, buffer);
    if (result != OBI_SUCCESS) {
        fprintf(stderr, "Error: Failed to load file: %s\n", 
                obi_result_to_string(result));
        obi_buffer_destroy(buffer);
        return EXIT_FAILURE;
    }
    
    /* Create validator with specified context */
    obi_validation_context_t context = {
        .zero_trust_enforced = options->zero_trust,
        .canonical_only = true,
        .alpha = options->alpha,
        .beta = options->beta,
        .epsilon_min = OBI_EPSILON_MIN
    };
    
    obi_validator_t *validator;
    result = obi_validator_create(&validator, &context);
    if (result != OBI_SUCCESS) {
        fprintf(stderr, "Error: Failed to create validator: %s\n", 
                obi_result_to_string(result));
        obi_buffer_destroy(buffer);
        return EXIT_FAILURE;
    }
    
    /* Perform validation */
    result = obi_validate_buffer(validator, buffer);
    
    /* Report results */
    printf("Validation Result: %s\n", obi_result_to_string(result));
    
    if (result == OBI_SUCCESS) {
        printf("Buffer validated successfully\n");
        printf("Cost value: %.6f\n", buffer->cost_value);
        printf("Governance zone: ");
        switch (buffer->governance_zone) {
            case OBI_ZONE_AUTONOMOUS:
                printf("AUTONOMOUS (C ≤ 0.5)\n");
                break;
            case OBI_ZONE_WARNING:
                printf("WARNING (0.5 < C ≤ 0.6)\n");
                break;
            case OBI_ZONE_GOVERNANCE:
                printf("GOVERNANCE (C > 0.6)\n");
                break;
        }
        
        if (options->nasa_compliance) {
            printf("NASA Compliance: %s\n", 
                   obi_check_nasa_compliance(buffer) ? "PASS" : "FAIL");
        }
        
        if (options->verbose) {
            printf("Sinphasé Compliance: %s\n", 
                   obi_check_sinphase_compliance(buffer) ? "PASS" : "FAIL");
        }
    } else {
        printf("Validation failed: %s\n", obi_result_to_string(result));
    }
    
    /* Cleanup */
    obi_validator_destroy(validator);
    obi_buffer_destroy(buffer);
    
    return (result == OBI_SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* Utility Functions */
static obi_result_t load_file_to_buffer(const char *filename, obi_buffer_t *buffer) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        return OBI_ERROR_INVALID_INPUT;
    }
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (size <= 0 || size > MAX_FILE_SIZE) {
        fclose(file);
        return OBI_ERROR_BUFFER_OVERFLOW;
    }
    
    uint8_t data[MAX_FILE_SIZE];
    size_t bytes_read = fread(data, 1, size, file);
    fclose(file);
    
    if (bytes_read != (size_t)size) {
        return OBI_ERROR_INVALID_INPUT;
    }
    
    return obi_buffer_set_data(buffer, data, bytes_read);
}

static cli_command_t parse_command(const char *cmd) {
    if (strcmp(cmd, "validate") == 0) return CMD_VALIDATE;
    if (strcmp(cmd, "normalize") == 0) return CMD_NORMALIZE;
    if (strcmp(cmd, "audit") == 0) return CMD_AUDIT;
    if (strcmp(cmd, "benchmark") == 0) return CMD_BENCHMARK;
    if (strcmp(cmd, "version") == 0) return CMD_VERSION;
    if (strcmp(cmd, "help") == 0) return CMD_HELP;
    return CMD_UNKNOWN;
}

static void print_usage(const char *program_name) {
    printf("OBI Buffer Protocol CLI v%s\n", CLI_VERSION);
    printf("OBINexus Computing - Aegis Framework\n\n");
    printf("Usage: %s <command> [options]\n\n", program_name);
    printf("Commands:\n");
    printf("  validate     Validate buffer against OBI protocol\n");
    printf("  normalize    Normalize buffer using USCN\n");
    printf("  audit        Generate audit report\n");
    printf("  benchmark    Run performance benchmarks\n");
    printf("  version      Show version information\n");
    printf("  help         Show this help message\n\n");
    printf("Options:\n");
    printf("  -i, --input <file>       Input file\n");
    printf("  -o, --output <file>      Output file\n");
    printf("  -s, --schema <file>      Schema file (default: schema/obiprotocol_schema.yaml)\n");
    printf("  -a, --audit-log <file>   Audit log file (default: audit.log)\n");
    printf("  -v, --verbose            Verbose output\n");
    printf("  -z, --no-zero-trust      Disable Zero Trust enforcement\n");
    printf("  -n, --no-nasa           Disable NASA compliance checking\n");
    printf("  -A, --alpha <value>      Alpha parameter (default: %.3f)\n", OBI_ALPHA_DEFAULT);
    printf("  -B, --beta <value>       Beta parameter (default: %.3f)\n", OBI_BETA_DEFAULT);
    printf("  -h, --help              Show this help message\n");
    printf("  -V, --version           Show version information\n\n");
    printf("Examples:\n");
    printf("  %s validate -i data.bin -v\n", program_name);
    printf("  %s normalize -i input.bin -o output.bin\n", program_name);
    printf("  %s audit -a system_audit.log\n", program_name);
}

static void print_version(void) {
    printf("OBI Buffer Protocol CLI v%s\n", CLI_VERSION);
    printf("OBINexus Computing - Aegis Framework\n");
    printf("Core Library: %s\n", obi_get_version_string());
    printf("Compliance: NASA-STD-8739.8\n");
    printf("Architecture: Sinphasé Single-Pass Compilation\n");
    printf("Zero Trust: %s\n", obi_is_zero_trust_enforced() ? "Enforced" : "Optional");
}

/* Placeholder implementations for other commands */
static int cmd_normalize(const cli_options_t *options) {
    printf("Normalize command not yet implemented\n");
    return EXIT_SUCCESS;
}

static int cmd_audit(const cli_options_t *options) {
    printf("Audit command not yet implemented\n");
    return EXIT_SUCCESS;
}

static int cmd_benchmark(const cli_options_t *options) {
    printf("Benchmark command not yet implemented\n");
    return EXIT_SUCCESS;
}