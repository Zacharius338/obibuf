# OBI Buffer Protocol

**Polyglot-safe, schema-driven protocol engine for secure inter-system communication**

## Overview

OBI Buffer Protocol is a **protocol-first** communication framework designed to replace fragile approaches like Google Protobuf, unsafe serialization formats like Pickle, and untyped formats like raw JSON. 

### Key Principles

- **Protocol ≠ Language**: Canonical wire format enforced across all language adapters
- **Schema Required**: YAML-driven validation with mandatory compliance
- **Zero Trust Default**: No unvalidated data crosses system boundaries
- **Cryptographic Audit Trail**: NASA-STD-8739.8 compliant logging
- **Thin Adapters**: No business logic in language-specific adapters
- **Cross-Language**: Support for C, Python, Lua, JavaScript

### Architecture

```
┌─────────────────────────────────────────────────┐
│                Schema Definition                │
│            (secure_message.yaml)               │
└─────────────────┬───────────────────────────────┘
                  │
┌─────────────────▼───────────────────────────────┐
│           Protocol State Machine               │
│  ┌─────────────┬─────────────┬─────────────┐    │
│  │  Validator  │ Normalizer  │ Automaton   │    │
│  │             │   (USCN)    │   (CPA)     │    │
│  └─────────────┴─────────────┴─────────────┘    │
└─────────────────┬───────────────────────────────┘
                  │
┌─────────────────▼───────────────────────────────┐
│              Language Adapters                  │
│     Python │ C │ Lua │ JavaScript │ ...        │
│    (Thin serialization layer only)             │
└─────────────────────────────────────────────────┘
```

## Quick Start

1. **Build the core library:**
   ```bash
   cd obibuf
   mkdir -p build && cd build
   cmake ..
   make
   ```

2. **Test the CLI:**
   ```bash
   ./obibuf_cli version
   ```

3. **Use Python adapter:**
   ```python
   from obibuf.adapters.python.obi_adapter import OBIBufferAdapter
   
   adapter = OBIBufferAdapter()
   message = {"id": 12345, "payload": "Hello, World!"}
   serialized = adapter.serialize_message(message)
   ```

## Core Components

### Protocol State Machine
- **Automaton**: Character-set level state minimization (DFA)
- **Normalizer**: USCN isomorphic reduction to canonical forms
- **Validator**: Schema compliance and Zero Trust enforcement
- **Audit**: Cryptographic hash trail (no raw data exposure)

### Language Adapters
- **Python**: `obibuf/adapters/python/obi_adapter.py`
- **C**: `obibuf/adapters/c/obi_c_adapter.h`
- **Lua**: `obibuf/adapters/lua/` (planned)
- **JavaScript**: `obibuf/adapters/javascript/` (planned)

### Schema Format
```yaml
message_type: SecureMessage
fields:
  - name: id
    type: uint64
    required: true
  - name: payload
    type: binary
    required: true
    max_length: 4096
```

## Compliance

- **NASA-STD-8739.8**: Safety-critical system requirements
- **Zero Trust Architecture**: Mandatory validation at all boundaries
- **USCN Normalization**: Isomorphic reduction prevents encoding exploits

## Development Status

- ✅ Core architecture defined
- ✅ C library headers and build system
- ✅ Python adapter template
- ✅ Schema validation framework
- 🔄 Protocol State Machine implementation
- 🔄 USCN Normalizer implementation
- ⏳ Cross-language adapter completion
- ⏳ Integration testing framework

## License

OBINexus Computing - Protocol Engine Division
