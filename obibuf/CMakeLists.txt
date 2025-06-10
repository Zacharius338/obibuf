cmake_minimum_required(VERSION 3.10)
project(obi_buffer C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic")

# Include directories
include_directories(obibuf/core/include)

# Core OBI Buffer library
add_library(obi_buffer_core STATIC
    obibuf/core/src/automaton.c
    obibuf/core/src/normalizer.c
    obibuf/core/src/validator.c
    obibuf/core/src/audit.c
    obibuf/core/src/protocol_state_machine.c
)

# CLI executable
add_executable(obibuf_cli
    obibuf/cli/main.c
)
target_link_libraries(obibuf_cli obi_buffer_core)

# Test executables
add_executable(test_validator obibuf/core/tests/test_validator.c)
target_link_libraries(test_validator obi_buffer_core)

add_executable(test_normalizer obibuf/core/tests/test_normalizer.c)
target_link_libraries(test_normalizer obi_buffer_core)

add_executable(test_automaton obibuf/core/tests/test_automaton.c)
target_link_libraries(test_automaton obi_buffer_core)

# Installation targets
install(TARGETS obi_buffer_core DESTINATION lib)
install(DIRECTORY obibuf/core/include/ DESTINATION include)
install(TARGETS obibuf_cli DESTINATION bin)
