# OBI Buffer Protocol - Sinphasé Build System
# OBINexus Computing - Aegis Framework
# Single-Pass Compilation Architecture

# Project Configuration
PROJECT_NAME = obiprotocol
VERSION = 1.0.0
PREFIX ?= /usr/local

# Compiler Configuration
CC = gcc
AR = ar
CFLAGS = -std=c11 -Wall -Wextra -O2 -fPIC
CFLAGS += -DOBI_VERSION_MAJOR=1 -DOBI_VERSION_MINOR=0 -DOBI_VERSION_PATCH=0
LDFLAGS = -lm

# Debug/Release Configuration  
DEBUG ?= 0
ifeq ($(DEBUG), 1)
    CFLAGS += -g -DDEBUG -O0
    BUILD_DIR = build/debug
else
    CFLAGS += -DNDEBUG
    BUILD_DIR = build/release
endif

# Thread Safety (optional)
THREAD_SAFE ?= 0
ifeq ($(THREAD_SAFE), 1)
    CFLAGS += -DOBI_THREAD_SAFE
    LDFLAGS += -lpthread
endif

# Directory Structure
CORE_DIR = core
SRC_DIR = $(CORE_DIR)/src
INCLUDE_DIR = $(CORE_DIR)/include
CLI_DIR = cli
ADAPTERS_DIR = adapters
TESTS_DIR = tests

# Source Files
CORE_SOURCES = $(wildcard $(SRC_DIR)/*.c)
CLI_SOURCES = $(wildcard $(CLI_DIR)/*.c) $(wildcard $(CLI_DIR)/commands/*.c)
TEST_SOURCES = $(wildcard $(TESTS_DIR)/*.c)

# Object Files
CORE_OBJECTS = $(CORE_SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/core/%.o)
CLI_OBJECTS = $(CLI_SOURCES:$(CLI_DIR)/%.c=$(BUILD_DIR)/cli/%.o)
TEST_OBJECTS = $(TEST_SOURCES:$(TESTS_DIR)/%.c=$(BUILD_DIR)/tests/%.o)

# Output Files
STATIC_LIB = $(BUILD_DIR)/lib$(PROJECT_NAME).a
SHARED_LIB = $(BUILD_DIR)/lib$(PROJECT_NAME).so.$(VERSION)
CLI_BINARY = $(BUILD_DIR)/obi_cli
TEST_BINARY = $(BUILD_DIR)/test_suite

# Include Paths
INCLUDES = -I$(INCLUDE_DIR)

# Default Target
.PHONY: all
all: directories $(STATIC_LIB) $(SHARED_LIB) $(CLI_BINARY)

# Sinphasé Architecture: Single-Pass Build Targets
.PHONY: core
core: directories $(STATIC_LIB) $(SHARED_LIB)

.PHONY: cli
cli: core $(CLI_BINARY)

.PHONY: test
test: core $(TEST_BINARY)
	@echo "Running OBI Protocol Test Suite..."
	@$(TEST_BINARY)

# Directory Creation
.PHONY: directories
directories:
	@mkdir -p $(BUILD_DIR)/core
	@mkdir -p $(BUILD_DIR)/cli/commands
	@mkdir -p $(BUILD_DIR)/tests
	@mkdir -p $(BUILD_DIR)/lib

# Core Library Build (Static)
$(STATIC_LIB): $(CORE_OBJECTS)
	@echo "Building static library: $@"
	@$(AR) rcs $@ $^

# Core Library Build (Shared)
$(SHARED_LIB): $(CORE_OBJECTS)
	@echo "Building shared library: $@"
	@$(CC) -shared -Wl,-soname,lib$(PROJECT_NAME).so.1 -o $@ $^ $(LDFLAGS)
	@cd $(BUILD_DIR) && ln -sf lib$(PROJECT_NAME).so.$(VERSION) lib$(PROJECT_NAME).so.1
	@cd $(BUILD_DIR) && ln -sf lib$(PROJECT_NAME).so.$(VERSION) lib$(PROJECT_NAME).so

# CLI Binary Build
$(CLI_BINARY): $(CLI_OBJECTS) $(STATIC_LIB)
	@echo "Building CLI binary: $@"
	@$(CC) -o $@ $(CLI_OBJECTS) -L$(BUILD_DIR) -l$(PROJECT_NAME) $(LDFLAGS)

# Test Suite Build
$(TEST_BINARY): $(TEST_OBJECTS) $(STATIC_LIB)
	@echo "Building test suite: $@"
	@$(CC) -o $@ $(TEST_OBJECTS) -L$(BUILD_DIR) -l$(PROJECT_NAME) $(LDFLAGS)

# Object File Compilation Rules
$(BUILD_DIR)/core/%.o: $(SRC_DIR)/%.c
	@echo "Compiling core: $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/cli/%.o: $(CLI_DIR)/%.c
	@echo "Compiling CLI: $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/cli/commands/%.o: $(CLI_DIR)/commands/%.c
	@echo "Compiling CLI command: $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/tests/%.o: $(TESTS_DIR)/%.c
	@echo "Compiling test: $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Installation Targets
.PHONY: install
install: all
	@echo "Installing OBI Protocol..."
	@install -d $(PREFIX)/lib
	@install -d $(PREFIX)/include
	@install -d $(PREFIX)/bin
	@install -m 644 $(STATIC_LIB) $(PREFIX)/lib/
	@install -m 755 $(SHARED_LIB) $(PREFIX)/lib/
	@install -m 644 $(INCLUDE_DIR)/*.h $(PREFIX)/include/
	@install -m 755 $(CLI_BINARY) $(PREFIX)/bin/
	@ldconfig

.PHONY: uninstall
uninstall:
	@echo "Uninstalling OBI Protocol..."
	@rm -f $(PREFIX)/lib/lib$(PROJECT_NAME).*
	@rm -f $(PREFIX)/include/obiprotocol.h
	@rm -f $(PREFIX)/include/validator.h
	@rm -f $(PREFIX)/include/normalizer.h
	@rm -f $(PREFIX)/include/audit.h
	@rm -f $(PREFIX)/bin/obi_cli

# Cross-Language Adapter Validation
.PHONY: validate-adapters
validate-adapters: core
	@echo "Validating cross-language adapters..."
	@if command -v python3 >/dev/null 2>&1; then \
		echo "Testing Python adapter..."; \
		python3 $(ADAPTERS_DIR)/python_adapter.py; \
	fi
	@if command -v lua >/dev/null 2>&1; then \
		echo "Testing Lua adapter..."; \
		lua $(ADAPTERS_DIR)/lua_adapter.lua; \
	fi
	@if command -v node >/dev/null 2>&1; then \
		echo "Testing JavaScript adapter..."; \
		node $(ADAPTERS_DIR)/js_adapter.js; \
	fi

# Documentation Generation
.PHONY: docs
docs:
	@echo "Generating documentation..."
	@if command -v doxygen >/dev/null 2>&1; then \
		doxygen Doxyfile; \
	else \
		echo "Doxygen not found, skipping documentation generation"; \
	fi

# Compliance Verification
.PHONY: verify-nasa
verify-nasa: test
	@echo "Verifying NASA-STD-8739.8 compliance..."
	@$(TEST_BINARY) --nasa-compliance

.PHONY: verify-zero-trust
verify-zero-trust: test
	@echo "Verifying Zero Trust architecture..."
	@$(TEST_BINARY) --zero-trust-validation

.PHONY: verify-sinphase
verify-sinphase: test
	@echo "Verifying Sinphasé governance..."
	@$(TEST_BINARY) --sinphase-compliance

# Performance Benchmarks
.PHONY: benchmark
benchmark: core
	@echo "Running performance benchmarks..."
	@$(BUILD_DIR)/benchmark_suite

# Clean Targets
.PHONY: clean
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)

.PHONY: distclean
distclean: clean
	@echo "Deep cleaning..."
	@rm -rf build/
	@rm -f *.log
	@rm -f audit.log

# Package Creation
.PHONY: package
package: all
	@echo "Creating distribution package..."
	@mkdir -p dist/$(PROJECT_NAME)-$(VERSION)
	@cp -r $(INCLUDE_DIR) dist/$(PROJECT_NAME)-$(VERSION)/
	@cp $(STATIC_LIB) $(SHARED_LIB) dist/$(PROJECT_NAME)-$(VERSION)/
	@cp $(CLI_BINARY) dist/$(PROJECT_NAME)-$(VERSION)/
	@cp README.md LICENSE dist/$(PROJECT_NAME)-$(VERSION)/
	@cd dist && tar -czf $(PROJECT_NAME)-$(VERSION).tar.gz $(PROJECT_NAME)-$(VERSION)
	@echo "Package created: dist/$(PROJECT_NAME)-$(VERSION).tar.gz"

# Development Helpers
.PHONY: format
format:
	@echo "Formatting source code..."
	@if command -v clang-format >/dev/null 2>&1; then \
		find $(SRC_DIR) $(INCLUDE_DIR) $(CLI_DIR) $(TESTS_DIR) -name "*.c" -o -name "*.h" | xargs clang-format -i; \
	fi

.PHONY: static-analysis
static-analysis:
	@echo "Running static analysis..."
	@if command -v cppcheck >/dev/null 2>&1; then \
		cppcheck --enable=all --std=c11 $(SRC_DIR) $(CLI_DIR); \
	fi

# GitHub Workflow Integration
.PHONY: ci-build
ci-build: all test verify-nasa verify-zero-trust verify-sinphase

.PHONY: ci-package
ci-package: ci-build package

# Help Target
.PHONY: help
help:
	@echo "OBI Buffer Protocol - Build System"
	@echo "OBINexus Computing - Aegis Framework"
	@echo ""
	@echo "Available targets:"
	@echo "  all                - Build everything (default)"
	@echo "  core               - Build core library only"
	@echo "  cli                - Build CLI tools"
	@echo "  test               - Build and run tests"
	@echo "  install            - Install to system"
	@echo "  uninstall          - Remove from system"
	@echo "  validate-adapters  - Test cross-language adapters"
	@echo "  verify-nasa        - NASA-STD-8739.8 compliance check"
	@echo "  verify-zero-trust  - Zero Trust validation"
	@echo "  verify-sinphase    - Sinphasé governance check"
	@echo "  benchmark          - Performance benchmarks"
	@echo "  package            - Create distribution package"
	@echo "  clean              - Remove build artifacts"
	@echo "  help               - Show this help"
	@echo ""
	@echo "Configuration options:"
	@echo "  DEBUG=1            - Enable debug build"
	@echo "  THREAD_SAFE=1      - Enable thread safety"
	@echo "  PREFIX=/path       - Installation prefix"

# Version Information
.PHONY: version
version:
	@echo "OBI Buffer Protocol v$(VERSION)"
	@echo "OBINexus Computing - Aegis Framework"
	@echo "Compliance: NASA-STD-8739.8, Zero Trust Architecture"
	@echo "Architecture: Sinphasé Single-Pass Compilation"