# OBI Buffer Protocol

**Protocol-first, polyglot-safe message interchange**

## Directory Structure

```
obibuf/
├── include/obibuf/
│   ├── core/obibuf.h          # Core protocol API
│   └── cli/commands.h         # CLI interface
├── src/
│   ├── core/                  # Core C implementation
│   └── adapters/              # Language adapters
├── cli/                       # Command-line tools
├── tests/                     # Unit and integration tests
├── schema/                    # YAML schema definitions
└── CMakeLists.txt            # Build configuration
```

## Quick Start

```bash
cd obibuf
./build.sh
./build/obibuf_cli version
```

## Core Principles

- **Zero Trust**: Mandatory validation at protocol boundaries
- **Schema-Driven**: YAML-defined message contracts
- **Canonical Forms**: USCN normalization prevents encoding exploits
- **Audit Compliance**: NASA-STD-8739.8 cryptographic trails

## Implementation Status

- ✅ Directory structure
- ✅ Core headers
- ✅ Build system
- 🔄 DFA automaton implementation
- 🔄 Full validation pipeline
- ⏳ Cross-language adapters
