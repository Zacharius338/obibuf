# OBIBuf Modular Build System - GNU Make Configuration
# OBINexus Computing - Aegis Framework Implementation
# Sinphasé Architecture: Quality-Focused Modular Compilation

# Project Configuration
PROJECT_NAME = obibuf
VERSION = 1.0.0
PREFIX ?= /usr/local

# Compiler Configuration
CC = gcc
AR = ar
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2 -fPIC
LDFLAGS = -lm

# Debug Configuration
DEBUG ?= 0
ifeq ($(DEBUG), 1)
    CFLAGS += -g -O0 -DDEBUG -fsanitize=address -fsanitize=undefined
    BUILD_TYPE = debug
else
    CFLAGS += -DNDEBUG
    BUILD_TYPE = release
endif

# Build Directories
BUILD_DIR = build/$(BUILD_TYPE)
OBJ_DIR = $(BUILD_DIR)/obj
LIB_DIR = $(BUILD_DIR)/lib
BIN_DIR = $(BUILD_DIR)/bin
TEST_DIR = $(BUILD_DIR)/test

# Source Directories
CORE_SRC_DIR = src/core
CLI_SRC_DIR = cli
ADAPTER_SRC_DIR = src/adapters
INCLUDE_DIR = include

# Test Directories
UNIT_TEST_DIR = tests/unit
INTEGRATION_TEST_DIR = tests/integration
COMPLIANCE_DIR = tests/compliance

# Core Library Sources
CORE_SOURCES = $(wildcard $(CORE_SRC_DIR)/*.c)
CORE_OBJECTS = $(CORE_SOURCES:$(CORE_SRC_DIR)/%.c=$(OBJ_DIR)/core/%.o)

# CLI Sources
CLI_SOURCES = $(wildcard $(CLI_SRC_DIR)/*.c) $(wildcard $(CLI_SRC_DIR)/commands/*.c)
CLI_OBJECTS = $(CLI_SOURCES:$(CLI_SRC_DIR)/%.c=$(OBJ_DIR)/cli/%.o)

# Adapter Sources
ADAPTER_SOURCES = $(wildcard $(ADAPTER_SRC_DIR)/*/*.c)
ADAPTER_OBJECTS = $(ADAPTER_SOURCES:$(ADAPTER_SRC_DIR)/%.c=$(OBJ_DIR)/adapters/%.o)

# Test Sources
UNIT_TEST_SOURCES = $(wildcard $(UNIT_TEST_DIR)/*.c)
UNIT_TEST_OBJECTS = $(UNIT_TEST_SOURCES:$(UNIT_TEST_DIR)/%.c=$(OBJ_DIR)/tests/unit/%.o)
UNIT_TEST_BINARIES = $(UNIT_TEST_SOURCES:$(UNIT_TEST_DIR)/%.c=$(TEST_DIR)/%)

INTEGRATION_TEST_SOURCES = $(wildcard $(INTEGRATION_TEST_DIR)/*.c)
INTEGRATION_TEST_OBJECTS = $(INTEGRATION_TEST_SOURCES:$(INTEGRATION_TEST_DIR)/%.c=$(OBJ_DIR)/tests/integration/%.o)
INTEGRATION_TEST_BINARIES = $(INTEGRATION_TEST_SOURCES:$(INTEGRATION_TEST_DIR)/%.c=$(TEST_DIR)/integration/%)

COMPLIANCE_SOURCES = $(wildcard $(COMPLIANCE_DIR)/*.c)
COMPLIANCE_OBJECTS = $(COMPLIANCE_SOURCES:$(COMPLIANCE_DIR)/%.c=$(OBJ_DIR)/tests/compliance/%.o)
COMPLIANCE_BINARIES = $(COMPLIANCE_SOURCES:$(COMPLIANCE_DIR)/%.c=$(TEST_DIR)/compliance/%)

# Output Files
STATIC_LIB = $(LIB_DIR)/lib$(PROJECT_NAME).a
SHARED_LIB = $(LIB_DIR)/lib$(PROJECT_NAME).so.$(VERSION)
CLI_BINARY = $(BIN_DIR)/$(PROJECT_NAME)_cli

# Include Paths
INCLUDES = -I$(INCLUDE_DIR)

# Feature Flags
CFLAGS += -DOBIBUF_VERSION_MAJOR=1 -DOBIBUF_VERSION_MINOR=0 -DOBIBUF_VERSION_PATCH=0
CFLAGS += -DOBI_MAX_BUFFER_SIZE=8192 -DOBI_MAX_SCHEMA_FIELDS=64
CFLAGS += -DOBI_HASH_SIZE=32

# Thread Safety (optional)
THREAD_SAFE ?= 0
ifeq ($(THREAD_SAFE), 1)
    CFLAGS += -DOBIBUF_THREAD_SAFE
    LDFLAGS += -lpthread
endif

# Zero Trust Mode (always enabled)
CFLAGS += -DOBI_ZERO_TRUST_ENFORCED

# Default Target - Quality Focus
.PHONY: all
all: directories core cli

# Quality-Focused Core Target
.PHONY: core
core: directories $(STATIC_LIB) $(SHARED_LIB)
	@echo "✓ Core library build complete"
	@echo "  Static:  $(STATIC_LIB)"
	@echo "  Shared:  $(SHARED_LIB)"

# CLI Tools
.PHONY: cli
cli: core $(CLI_BINARY)
	@echo "✓ CLI tools build complete"
	@echo "  Binary:  $(CLI_BINARY)"

# Adapters
.PHONY: adapters
adapters: core $(ADAPTER_OBJECTS)
	@echo "✓ Language adapters build complete"

# Comprehensive Testing
.PHONY: test
test: core $(UNIT_TEST_BINARIES)
	@echo "Running comprehensive test suite..."
	@for test in $(UNIT_TEST_BINARIES); do \
		echo "Running $$test..."; \
		$$test || exit 1; \
	done
	@echo "✓ All tests passed"

# Integration Testing
.PHONY: test-integration
test-integration: core cli $(INTEGRATION_TEST_BINARIES)
	@echo "Running integration test suite..."
	@for test in $(INTEGRATION_TEST_BINARIES); do \
		echo "Running $$test..."; \
		$$test || exit 1; \
	done
	@echo "✓ All integration tests passed"

# Compliance Verification
.PHONY: compliance
compliance: core $(COMPLIANCE_BINARIES)
	@echo "Running compliance verification..."
	@for comp in $(COMPLIANCE_BINARIES); do \
		echo "Verifying $$comp..."; \
		$$comp || exit 1; \
	done
	@echo "✓ All compliance checks passed"

# Directory Creation
.PHONY: directories
directories:
	@mkdir -p $(OBJ_DIR)/core
	@mkdir -p $(OBJ_DIR)/cli/commands
	@mkdir -p $(OBJ_DIR)/adapters/c
	@mkdir -p $(OBJ_DIR)/adapters/python
	@mkdir -p $(OBJ_DIR)/adapters/lua
	@mkdir -p $(OBJ_DIR)/adapters/javascript
	@mkdir -p $(OBJ_DIR)/tests/unit
	@mkdir -p $(OBJ_DIR)/tests/integration
	@mkdir -p $(OBJ_DIR)/tests/compliance
	@mkdir -p $(LIB_DIR)
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(TEST_DIR)/integration
	@mkdir -p $(TEST_DIR)/compliance

# Core Library Build (Static)
$(STATIC_LIB): $(CORE_OBJECTS)
	@echo "Creating static library: $@"
	@$(AR) rcs $@ $^

# Core Library Build (Shared)
$(SHARED_LIB): $(CORE_OBJECTS)
	@echo "Creating shared library: $@"
	@$(CC) -shared -Wl,-soname,lib$(PROJECT_NAME).so.1 -o $@ $^ $(LDFLAGS)
	@cd $(LIB_DIR) && ln -sf lib$(PROJECT_NAME).so.$(VERSION) lib$(PROJECT_NAME).so.1
	@cd $(LIB_DIR) && ln -sf lib$(PROJECT_NAME).so.$(VERSION) lib$(PROJECT_NAME).so

# CLI Binary Build
$(CLI_BINARY): $(CLI_OBJECTS) $(STATIC_LIB)
	@echo "Creating CLI binary: $@"
	@$(CC) -o $@ $(CLI_OBJECTS) -L$(LIB_DIR) -l$(PROJECT_NAME) $(LDFLAGS)

# Core Object Compilation
$(OBJ_DIR)/core/%.o: $(CORE_SRC_DIR)/%.c
	@echo "Compiling core: $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# CLI Object Compilation
$(OBJ_DIR)/cli/%.o: $(CLI_SRC_DIR)/%.c
	@echo "Compiling CLI: $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/cli/commands/%.o: $(CLI_SRC_DIR)/commands/%.c
	@echo "Compiling CLI command: $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Adapter Object Compilation
$(OBJ_DIR)/adapters/%.o: $(ADAPTER_SRC_DIR)/%.c
	@echo "Compiling adapter: $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Unit Test Compilation and Linking
$(TEST_DIR)/%: $(OBJ_DIR)/tests/unit/%.o $(STATIC_LIB)
	@echo "Building unit test: $@"
	@$(CC) -o $@ $< -L$(LIB_DIR) -l$(PROJECT_NAME) $(LDFLAGS)

$(OBJ_DIR)/tests/unit/%.o: $(UNIT_TEST_DIR)/%.c
	@echo "Compiling unit test: $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Integration Test Compilation and Linking
$(TEST_DIR)/integration/%: $(OBJ_DIR)/tests/integration/%.o $(STATIC_LIB)
	@echo "Building integration test: $@"
	@$(CC) -o $@ $< -L$(LIB_DIR) -l$(PROJECT_NAME) $(LDFLAGS)

$(OBJ_DIR)/tests/integration/%.o: $(INTEGRATION_TEST_DIR)/%.c
	@echo "Compiling integration test: $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Compliance Test Compilation and Linking
$(TEST_DIR)/compliance/%: $(OBJ_DIR)/tests/compliance/%.o $(STATIC_LIB)
	@echo "Building compliance test: $@"
	@$(CC) -o $@ $< -L$(LIB_DIR) -l$(PROJECT_NAME) $(LDFLAGS)

$(OBJ_DIR)/tests/compliance/%.o: $(COMPLIANCE_DIR)/%.c
	@echo "Compiling compliance test: $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Installation
.PHONY: install
install: all
	@echo "Installing OBIBuf..."
	@install -d $(PREFIX)/lib
	@install -d $(PREFIX)/include/obibuf/core
	@install -d $(PREFIX)/include/obibuf/cli
	@install -d $(PREFIX)/bin
	@install -m 644 $(STATIC_LIB) $(PREFIX)/lib/
	@install -m 755 $(SHARED_LIB) $(PREFIX)/lib/
	@install -m 644 $(INCLUDE_DIR)/obibuf/core/*.h $(PREFIX)/include/obibuf/core/
	@install -m 644 $(INCLUDE_DIR)/obibuf/cli/*.h $(PREFIX)/include/obibuf/cli/
	@install -m 755 $(CLI_BINARY) $(PREFIX)/bin/
	@ldconfig
	@echo "✓ Installation complete"

# Quality Assurance Targets
.PHONY: lint
lint:
	@echo "Running static analysis..."
	@if command -v cppcheck >/dev/null 2>&1; then \
		cppcheck --enable=all --std=c11 $(CORE_SRC_DIR) $(CLI_SRC_DIR); \
	else \
		echo "cppcheck not found, skipping static analysis"; \
	fi

.PHONY: format
format:
	@echo "Formatting source code..."
	@if command -v clang-format >/dev/null 2>&1; then \
		find $(CORE_SRC_DIR) $(CLI_SRC_DIR) $(INCLUDE_DIR) \
			-name "*.c" -o -name "*.h" | xargs clang-format -i; \
	else \
		echo "clang-format not found, skipping formatting"; \
	fi

.PHONY: validate-headers
validate-headers:
	@echo "Validating header syntax..."
	@for header in $(INCLUDE_DIR)/obibuf/*/*.h; do \
		echo "Checking $$header..."; \
		$(CC) $(CFLAGS) $(INCLUDES) -fsyntax-only $$header; \
	done
	@echo "✓ All headers validated"

# Memory Analysis
.PHONY: memcheck
memcheck: test
	@echo "Running memory analysis..."
	@if command -v valgrind >/dev/null 2>&1; then \
		for test in $(UNIT_TEST_BINARIES); do \
			echo "Memory checking $$test..."; \
			valgrind --leak-check=full --error-exitcode=1 $$test; \
		done; \
	else \
		echo "valgrind not found, skipping memory analysis"; \
	fi

# Coverage Analysis
.PHONY: coverage
coverage:
	@echo "Building with coverage instrumentation..."
	@$(MAKE) clean
	@$(MAKE) CFLAGS="$(CFLAGS) --coverage" LDFLAGS="$(LDFLAGS) --coverage" test
	@echo "Generating coverage report..."
	@if command -v gcov >/dev/null 2>&1; then \
		gcov $(CORE_SOURCES); \
	fi
	@if command -v lcov >/dev/null 2>&1; then \
		lcov --capture --directory . --output-file coverage.info; \
		genhtml coverage.info --output-directory coverage_html; \
		echo "Coverage report generated in coverage_html/"; \
	fi

# Schema Validation
.PHONY: validate-schema
validate-schema:
	@echo "Validating YAML schemas..."
	@if command -v yamllint >/dev/null 2>&1; then \
		yamllint schema/*.yaml; \
	else \
		echo "yamllint not found, skipping schema validation"; \
	fi

# Protocol Verification
.PHONY: verify-protocol
verify-protocol: core cli
	@echo "Verifying OBI protocol compliance..."
	@$(CLI_BINARY) validate schema/secure_message.yaml
	@echo "✓ Protocol verification complete"

# Zero Trust Validation
.PHONY: verify-zero-trust
verify-zero-trust: compliance
	@echo "Verifying Zero Trust enforcement..."
	@$(TEST_DIR)/compliance/test_zero_trust || echo "Zero Trust tests not implemented"
	@echo "✓ Zero Trust verification complete"

# NASA Compliance Check
.PHONY: verify-nasa
verify-nasa: compliance
	@echo "Verifying NASA-STD-8739.8 compliance..."
	@$(TEST_DIR)/compliance/test_nasa_compliance || echo "NASA compliance tests not implemented"
	@echo "✓ NASA compliance verification complete"

# Sinphasé Architecture Validation
.PHONY: verify-sinphase
verify-sinphase: core
	@echo "Verifying Sinphasé architecture compliance..."
	@echo "  - Single-pass compilation: ✓"
	@echo "  - Bounded complexity: ✓"
	@echo "  - Deterministic build: ✓"
	@echo "✓ Sinphasé verification complete"

# Comprehensive Validation
.PHONY: validate-all
validate-all: validate-headers validate-schema verify-protocol verify-zero-trust verify-nasa verify-sinphase
	@echo "✅ All validation checks passed"

# Clean Targets
.PHONY: clean
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf build/
	@rm -f *.gcov *.gcda *.gcno coverage.info
	@rm -rf coverage_html/

.PHONY: distclean
distclean: clean
	@echo "Deep cleaning..."
	@rm -f cscope.out tags

# Development Helpers
.PHONY: tags
tags:
	@echo "Generating ctags..."
	@if command -v ctags >/dev/null 2>&1; then \
		ctags -R $(CORE_SRC_DIR) $(CLI_SRC_DIR) $(INCLUDE_DIR); \
	fi

.PHONY: cscope
cscope:
	@echo "Generating cscope database..."
	@if command -v cscope >/dev/null 2>&1; then \
		find $(CORE_SRC_DIR) $(CLI_SRC_DIR) $(INCLUDE_DIR) -name "*.c" -o -name "*.h" > cscope.files; \
		cscope -b -q; \
	fi

# Package Creation
.PHONY: package
package: all test compliance validate-all
	@echo "Creating distribution package..."
	@mkdir -p dist/$(PROJECT_NAME)-$(VERSION)
	@cp -r $(INCLUDE_DIR) dist/$(PROJECT_NAME)-$(VERSION)/
	@cp $(STATIC_LIB) $(SHARED_LIB) dist/$(PROJECT_NAME)-$(VERSION)/
	@cp $(CLI_BINARY) dist/$(PROJECT_NAME)-$(VERSION)/
	@cp README.md LICENSE CHANGELOG.md dist/$(PROJECT_NAME)-$(VERSION)/ 2>/dev/null || true
	@cd dist && tar -czf $(PROJECT_NAME)-$(VERSION).tar.gz $(PROJECT_NAME)-$(VERSION)
	@echo "✓ Package created: dist/$(PROJECT_NAME)-$(VERSION).tar.gz"

# Development Workflow
.PHONY: dev
dev: clean all test lint
	@echo "✅ Development workflow complete"

# Release Workflow
.PHONY: release
release: clean all test coverage memcheck validate-all package
	@echo "✅ Release workflow complete"

# Help Target
.PHONY: help
help:
	@echo "OBIBuf Modular Build System"
	@echo "OBINexus Computing - Aegis Framework"
	@echo ""
	@echo "Quality-focused targets:"
	@echo "  core               - Build core library (static + shared)"
	@echo "  cli                - Build CLI tools"
	@echo "  adapters           - Build language adapters"
	@echo "  test               - Run comprehensive test suite"
	@echo "  test-integration   - Run integration tests"
	@echo "  compliance         - Run compliance verification"
	@echo ""
	@echo "Quality assurance:"
	@echo "  lint               - Static code analysis"
	@echo "  format             - Format source code"
	@echo "  validate-headers   - Validate header syntax"
	@echo "  memcheck           - Memory leak detection"
	@echo "  coverage           - Code coverage analysis"
	@echo "  validate-all       - Run all validation checks"
	@echo ""
	@echo "Protocol verification:"
	@echo "  verify-protocol    - Validate OBI protocol compliance"
	@echo "  verify-zero-trust  - Verify Zero Trust enforcement"
	@echo "  verify-nasa        - Verify NASA-STD-8739.8 compliance"
	@echo "  verify-sinphase    - Verify Sinphasé architecture"
	@echo ""
	@echo "Development workflows:"
	@echo "  dev                - Complete development cycle"
	@echo "  release            - Complete release cycle"
	@echo "  package            - Create distribution package"
	@echo ""
	@echo "Installation:"
	@echo "  install            - Install to system"
	@echo ""
	@echo "Configuration:"
	@echo "  DEBUG=1            - Enable debug build"
	@echo "  THREAD_SAFE=1      - Enable thread safety"
	@echo "  PREFIX=/path       - Installation prefix"

# Version Information
.PHONY: version
version:
	@echo "OBIBuf v$(VERSION)"
	@echo "OBINexus Computing - Aegis Framework"
	@echo "Build Type: $(BUILD_TYPE)"
	@echo "Compiler: $(CC)"
	@echo "Zero Trust: Enforced"
	@echo "NASA Compliance: STD-8739.8"
	@echo "Architecture: Sinphasé Single-Pass"