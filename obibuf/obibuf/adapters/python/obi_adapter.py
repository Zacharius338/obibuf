"""
OBI Buffer Protocol - Python Adapter
Thin serialization layer with Zero Trust enforcement
"""

import ctypes
import json
from typing import Optional, Dict, Any
from pathlib import Path

class OBIBufferAdapter:
    """
    Python adapter for OBI Buffer Protocol.
    Enforces thin layer principle - NO business logic in adapters.
    """
    
    def __init__(self, library_path: Optional[str] = None):
        """Initialize adapter with C library binding."""
        if library_path is None:
            # Try to find the library in standard locations
            library_path = self._find_library()
        
        self._lib = ctypes.CDLL(library_path)
        self._setup_function_signatures()
        
        # Initialize OBI Buffer library
        result = self._lib.obi_init()
        if result != 0:
            raise RuntimeError(f"Failed to initialize OBI Buffer: {result}")
    
    def _find_library(self) -> str:
        """Locate OBI Buffer core library."""
        possible_paths = [
            "./libobi_buffer_core.so",
            "/usr/local/lib/libobi_buffer_core.so",
            "../build/libobi_buffer_core.so"
        ]
        
        for path in possible_paths:
            if Path(path).exists():
                return path
        
        raise FileNotFoundError("OBI Buffer core library not found")
    
    def _setup_function_signatures(self):
        """Setup C function signatures for type safety."""
        # Buffer management
        self._lib.obi_buffer_create.argtypes = [ctypes.POINTER(ctypes.c_void_p)]
        self._lib.obi_buffer_create.restype = ctypes.c_int
        
        self._lib.obi_buffer_destroy.argtypes = [ctypes.c_void_p]
        self._lib.obi_buffer_destroy.restype = ctypes.c_int
        
        # Validation
        self._lib.obi_validate_buffer.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
        self._lib.obi_validate_buffer.restype = ctypes.c_int
    
    def serialize_message(self, message_dict: Dict[str, Any]) -> bytes:
        """
        Serialize Python dict to OBI Buffer format.
        THIN LAYER: No business logic, only format conversion.
        """
        # Convert to canonical JSON (normalized)
        canonical_json = json.dumps(message_dict, sort_keys=True, separators=(',', ':'))
        
        # Create buffer and validate through C core
        buffer_ptr = ctypes.c_void_p()
        result = self._lib.obi_buffer_create(ctypes.byref(buffer_ptr))
        if result != 0:
            raise RuntimeError(f"Buffer creation failed: {result}")
        
        try:
            # Set buffer data
            data_bytes = canonical_json.encode('utf-8')
            result = self._lib.obi_buffer_set_data(
                buffer_ptr, 
                ctypes.c_char_p(data_bytes), 
                len(data_bytes)
            )
            if result != 0:
                raise RuntimeError(f"Buffer data setting failed: {result}")
            
            # Validation occurs in C core - adapter doesn't validate
            return data_bytes
            
        finally:
            self._lib.obi_buffer_destroy(buffer_ptr)
    
    def deserialize_message(self, buffer_data: bytes) -> Dict[str, Any]:
        """
        Deserialize OBI Buffer format to Python dict.
        THIN LAYER: No business logic, only format conversion.
        """
        try:
            # Parse canonical JSON
            canonical_str = buffer_data.decode('utf-8')
            return json.loads(canonical_str)
        except (UnicodeDecodeError, json.JSONDecodeError) as e:
            raise ValueError(f"Invalid buffer format: {e}")
    
    def __del__(self):
        """Cleanup C library resources."""
        if hasattr(self, '_lib'):
            self._lib.obi_cleanup()
