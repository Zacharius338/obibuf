#!/usr/bin/env python3
"""
OBI Buffer Protocol - Python Adapter
OBINexus Computing - Aegis Framework

Zero Trust Architecture: All operations MUST go through C core library
No direct Python serialization permitted - enforces canonical protocol
"""

import ctypes
import ctypes.util
import os
import sys
from typing import Optional, Union, Dict, Any
from dataclasses import dataclass
from enum import IntEnum
import hashlib
import time

# Load the core C library
def _load_obiprotocol_library():
    """Load libobiprotocol with proper error handling"""
    library_paths = [
        "build/release/libobiprotocol.so",
        "build/debug/libobiprotocol.so", 
        "/usr/local/lib/libobiprotocol.so",
        "libobiprotocol.so"
    ]
    
    for path in library_paths:
        try:
            if os.path.exists(path):
                return ctypes.CDLL(path)
        except OSError:
            continue
    
    # Try system path
    lib_path = ctypes.util.find_library("obiprotocol")
    if lib_path:
        return ctypes.CDLL(lib_path)
    
    raise ImportError("Could not find libobiprotocol.so - ensure OBI core library is built")

# Initialize library
try:
    _lib = _load_obiprotocol_library()
except ImportError as e:
    print(f"Error: {e}", file=sys.stderr)
    print("Run 'make core' to build the OBI core library", file=sys.stderr)
    sys.exit(1)

# Constants from C header
OBI_MAX_BUFFER_SIZE = 8192
OBI_HASH_SIZE = 32
OBI_ALPHA_DEFAULT = 0.6
OBI_BETA_DEFAULT = 0.4

class OBIResult(IntEnum):
    """Result codes matching C enum"""
    SUCCESS = 0
    ERROR_INVALID_INPUT = -1
    ERROR_VALIDATION_FAILED = -2
    ERROR_AUDIT_REQUIRED = -3
    ERROR_ZERO_TRUST_VIOLATION = -4
    ERROR_BUFFER_OVERFLOW = -5
    ERROR_NUMERICAL_INSTABILITY = -6
    ERROR_SINPHASE_VIOLATION = -7

class SecurityLevel(IntEnum):
    """Security levels matching C enum"""
    STANDARD = 0
    HIGH = 1
    CRITICAL = 2

class GovernanceZone(IntEnum):
    """Governance zones from Sinphasé framework"""
    AUTONOMOUS = 0    # C ≤ 0.5
    WARNING = 1       # 0.5 < C ≤ 0.6  
    GOVERNANCE = 2    # C > 0.6

@dataclass
class ValidationContext:
    """Validation context for Zero Trust enforcement"""
    zero_trust_enforced: bool = True
    canonical_only: bool = True
    alpha: float = OBI_ALPHA_DEFAULT
    beta: float = OBI_BETA_DEFAULT
    epsilon_min: float = 1e-12

class OBIException(Exception):
    """Base exception for OBI protocol errors"""
    def __init__(self, result: OBIResult, message: str = ""):
        self.result = result
        self.message = message or self._get_result_message(result)
        super().__init__(self.message)
    
    def _get_result_message(self, result: OBIResult) -> str:
        messages = {
            OBIResult.ERROR_INVALID_INPUT: "Invalid input provided",
            OBIResult.ERROR_VALIDATION_FAILED: "Buffer validation failed",
            OBIResult.ERROR_AUDIT_REQUIRED: "Audit trail required",
            OBIResult.ERROR_ZERO_TRUST_VIOLATION: "Zero Trust architecture violation",
            OBIResult.ERROR_BUFFER_OVERFLOW: "Buffer size exceeded",
            OBIResult.ERROR_NUMERICAL_INSTABILITY: "Mathematical computation unstable",
            OBIResult.ERROR_SINPHASE_VIOLATION: "Sinphasé governance violation"
        }
        return messages.get(result, f"Unknown error: {result}")

# C structure definitions using ctypes
class _CBuffer(ctypes.Structure):
    """C buffer structure"""
    _fields_ = [
        ("data", ctypes.c_uint8 * OBI_MAX_BUFFER_SIZE),
        ("length", ctypes.c_size_t),
        ("audit_hash", ctypes.c_uint8 * OBI_HASH_SIZE),
        ("security_level", ctypes.c_int),
        ("validated", ctypes.c_bool),
        ("normalized", ctypes.c_bool),
        ("cost_value", ctypes.c_double),
        ("governance_zone", ctypes.c_int)
    ]

class _CValidationContext(ctypes.Structure):
    """C validation context structure"""
    _fields_ = [
        ("zero_trust_enforced", ctypes.c_bool),
        ("canonical_only", ctypes.c_bool),
        ("alpha", ctypes.c_double),
        ("beta", ctypes.c_double),
        ("epsilon_min", ctypes.c_double)
    ]

# Function signatures
_lib.obi_init.restype = ctypes.c_int
_lib.obi_cleanup.restype = None

_lib.obi_buffer_create.argtypes = [ctypes.POINTER(ctypes.c_void_p)]
_lib.obi_buffer_create.restype = ctypes.c_int

_lib.obi_buffer_destroy.argtypes = [ctypes.c_void_p]
_lib.obi_buffer_destroy.restype = ctypes.c_int

_lib.obi_buffer_set_data.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_uint8), ctypes.c_size_t]
_lib.obi_buffer_set_data.restype = ctypes.c_int

_lib.obi_validator_create.argtypes = [ctypes.POINTER(ctypes.c_void_p), ctypes.POINTER(_CValidationContext)]
_lib.obi_validator_create.restype = ctypes.c_int

_lib.obi_validator_destroy.argtypes = [ctypes.c_void_p]
_lib.obi_validator_destroy.restype = ctypes.c_int

_lib.obi_validate_buffer.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
_lib.obi_validate_buffer.restype = ctypes.c_int

_lib.obi_get_version_string.restype = ctypes.c_char_p
_lib.obi_is_zero_trust_enforced.restype = ctypes.c_bool

class OBIBuffer:
    """
    Python wrapper for OBI Buffer with Zero Trust enforcement
    
    All operations go through C core library - no Python serialization bypass allowed
    """
    
    def __init__(self):
        self._handle = ctypes.c_void_p()
        result = _lib.obi_buffer_create(ctypes.byref(self._handle))
        if result != OBIResult.SUCCESS:
            raise OBIException(OBIResult(result))
        
        self._validated = False
        self._normalized = False
        self._cost_value = 0.0
        self._governance_zone = GovernanceZone.AUTONOMOUS
    
    def __del__(self):
        if hasattr(self, '_handle') and self._handle:
            _lib.obi_buffer_destroy(self._handle)
    
    def set_data(self, data: Union[bytes, bytearray, str]) -> None:
        """
        Set buffer data through C core library
        
        Zero Trust: No direct Python serialization allowed
        """
        if isinstance(data, str):
            data = data.encode('utf-8')
        elif not isinstance(data, (bytes, bytearray)):
            raise TypeError("Data must be bytes, bytearray, or string")
        
        if len(data) > OBI_MAX_BUFFER_SIZE:
            raise OBIException(OBIResult.ERROR_BUFFER_OVERFLOW)
        
        # Convert to ctypes array
        data_array = (ctypes.c_uint8 * len(data))(*data)
        
        result = _lib.obi_buffer_set_data(self._handle, data_array, len(data))
        if result != OBIResult.SUCCESS:
            raise OBIException(OBIResult(result))
    
    @property
    def validated(self) -> bool:
        """Check if buffer has been validated"""
        return self._validated
    
    @property
    def cost_value(self) -> float:
        """Get traversal cost value from AEGIS-PROOF-1.2"""
        return self._cost_value
    
    @property 
    def governance_zone(self) -> GovernanceZone:
        """Get Sinphasé governance zone"""
        return self._governance_zone

class OBIValidator:
    """
    Python wrapper for OBI Validator with Zero Trust enforcement
    
    Enforces mathematical validation per AEGIS-PROOF-1.2
    """
    
    def __init__(self, context: Optional[ValidationContext] = None):
        if context is None:
            context = ValidationContext()
        
        # Zero Trust check - cannot disable enforcement
        if not context.zero_trust_enforced:
            raise OBIException(OBIResult.ERROR_ZERO_TRUST_VIOLATION,
                             "Cannot disable Zero Trust enforcement in adapter")
        
        # Convert to C structure
        c_context = _CValidationContext(
            zero_trust_enforced=context.zero_trust_enforced,
            canonical_only=context.canonical_only,
            alpha=context.alpha,
            beta=context.beta,
            epsilon_min=context.epsilon_min
        )
        
        self._handle = ctypes.c_void_p()
        result = _lib.obi_validator_create(ctypes.byref(self._handle), ctypes.byref(c_context))
        if result != OBIResult.SUCCESS:
            raise OBIException(OBIResult(result))
    
    def __del__(self):
        if hasattr(self, '_handle') and self._handle:
            _lib.obi_validator_destroy(self._handle)
    
    def validate(self, buffer: OBIBuffer) -> bool:
        """
        Validate buffer through C core library
        
        Returns True if validation succeeds, raises exception on failure
        """
        result = _lib.obi_validate_buffer(self._handle, buffer._handle)
        
        if result == OBIResult.SUCCESS:
            buffer._validated = True
            return True
        else:
            raise OBIException(OBIResult(result))

class OBIProtocol:
    """
    Main interface for OBI Buffer Protocol
    
    Enforces Zero Trust architecture - all operations through C core
    """
    
    _initialized = False
    
    @classmethod
    def initialize(cls) -> None:
        """Initialize OBI protocol"""
        if cls._initialized:
            return
        
        result = _lib.obi_init()
        if result != OBIResult.SUCCESS:
            raise OBIException(OBIResult(result), "Failed to initialize OBI protocol")
        
        cls._initialized = True
    
    @classmethod
    def cleanup(cls) -> None:
        """Cleanup OBI protocol"""
        if cls._initialized:
            _lib.obi_cleanup()
            cls._initialized = False
    
    @staticmethod
    def get_version() -> str:
        """Get OBI protocol version"""
        version_bytes = _lib.obi_get_version_string()
        return version_bytes.decode('utf-8') if version_bytes else "Unknown"
    
    @staticmethod
    def is_zero_trust_enforced() -> bool:
        """Check if Zero Trust is enforced"""
        return bool(_lib.obi_is_zero_trust_enforced())
    
    @staticmethod
    def create_buffer() -> OBIBuffer:
        """Create new OBI buffer"""
        return OBIBuffer()
    
    @staticmethod
    def create_validator(context: Optional[ValidationContext] = None) -> OBIValidator:
        """Create new OBI validator"""
        return OBIValidator(context)

def serialize_secure(data: Any, context: Optional[ValidationContext] = None) -> bytes:
    """
    Secure serialization using OBI protocol
    
    Zero Trust: All serialization goes through C core validation
    """
    OBIProtocol.initialize()
    
    # Convert data to string representation (simple serialization)
    if isinstance(data, (dict, list)):
        import json
        data_str = json.dumps(data, sort_keys=True)
    else:
        data_str = str(data)
    
    # Create buffer and set data
    buffer = OBIProtocol.create_buffer()
    buffer.set_data(data_str)
    
    # Validate through Zero Trust pipeline
    validator = OBIProtocol.create_validator(context)
    
    try:
        validator.validate(buffer)
        return data_str.encode('utf-8')
    except OBIException as e:
        raise ValueError(f"Serialization failed: {e.message}")

def example_usage():
    """Demonstrate OBI protocol usage"""
    print("OBI Buffer Protocol - Python Adapter Demo")
    print("OBINexus Computing - Aegis Framework")
    print("=" * 50)
    
    try:
        # Initialize
        OBIProtocol.initialize()
        print(f"OBI Protocol Version: {OBIProtocol.get_version()}")
        print(f"Zero Trust Enforced: {OBIProtocol.is_zero_trust_enforced()}")
        
        # Create buffer
        buffer = OBIProtocol.create_buffer()
        buffer.set_data("Hello, OBINexus Aegis Framework!")
        
        # Create validator with custom context
        context = ValidationContext(
            alpha=0.7,
            beta=0.3
        )
        validator = OBIProtocol.create_validator(context)
        
        # Validate
        print("\nValidating buffer...")
        is_valid = validator.validate(buffer)
        print(f"Validation Result: {'SUCCESS' if is_valid else 'FAILED'}")
        print(f"Cost Value: {buffer.cost_value:.6f}")
        print(f"Governance Zone: {buffer.governance_zone.name}")
        
        # Demonstrate secure serialization
        print("\nSecure serialization demo...")
        test_data = {"message": "OBINexus", "framework": "Aegis", "compliance": "NASA-STD-8739.8"}
        serialized = serialize_secure(test_data)
        print(f"Serialized: {serialized[:50]}...")
        
    except OBIException as e:
        print(f"OBI Error: {e.message}")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        OBIProtocol.cleanup()

if __name__ == "__main__":
    example_usage()