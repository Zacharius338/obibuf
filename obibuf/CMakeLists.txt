cmake_minimum_required(VERSION 3.10)
project(obibuf C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic")

# Include directories
include_directories(include)
include_directories(cli/include)

# Core OBI Buffer library
add_library(obibuf_core STATIC
    src/core/automaton.c
    src/core/normalizer.c
    src/core/validator.c
    src/core/audit.c
    src/core/protocol_state_machine.c
    src/core/buffer.c
)

# C Adapter library
add_library(obibuf_c_adapter STATIC
    src/adapters/c/obi_c_adapter.c
)
target_link_libraries(obibuf_c_adapter obibuf_core)

# CLI executable
add_executable(obibuf_cli
    cli/main.c
    cli/commands/validate.c
    cli/commands/audit.c
    cli/commands/normalize.c
)
target_link_libraries(obibuf_cli obibuf_core)

# Unit tests
add_executable(test_validator tests/unit/test_validator.c)
target_link_libraries(test_validator obibuf_core)

add_executable(test_normalizer tests/unit/test_normalizer.c)
target_link_libraries(test_normalizer obibuf_core)

add_executable(test_automaton tests/unit/test_automaton.c)
target_link_libraries(test_automaton obibuf_core)

# Integration tests
add_executable(test_integration tests/integration/test_full_pipeline.c)
target_link_libraries(test_integration obibuf_core)

# Installation
install(TARGETS obibuf_core DESTINATION lib)
install(TARGETS obibuf_cli DESTINATION bin)
install(DIRECTORY include/obibuf DESTINATION include)

# Testing target
enable_testing()
add_test(NAME validator_tests COMMAND test_validator)
add_test(NAME normalizer_tests COMMAND test_normalizer)
add_test(NAME automaton_tests COMMAND test_automaton)
add_test(NAME integration_tests COMMAND test_integration)
