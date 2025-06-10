"""
OBI Buffer Protocol - IOC Configuration
Zero Trust enforcement and integrity validation
"""

class IOCContainer:
    """
    Inversion of Control container for OBI Buffer Protocol.
    Enforces Zero Trust principles and maintains system integrity.
    """
    
    def __init__(self):
        self.zero_trust_mode = True  # Default: Zero Trust enforced
        self.integrity_valid = True
        self.audit_enabled = True
        self.schema_validation_required = True
        self.context = {
            'protocol_version': '1.0.0',
            'compliance_level': 'NASA-STD-8739.8',
            'normalization': 'USCN_REQUIRED'
        }
    
    def validate_integrity(self):
        """Validate system integrity - mandatory for all operations."""
        if not self.integrity_valid:
            raise RuntimeError("IOC integrity violation detected!")
        return True
    
    def enforce_zero_trust(self):
        """Enforce Zero Trust validation requirements."""
        if not self.zero_trust_mode:
            raise RuntimeError("Zero Trust mode cannot be disabled!")
        return True
    
    def get_context(self, key):
        """Retrieve configuration context safely."""
        self.validate_integrity()
        return self.context.get(key)
    
    def set_context(self, key, value):
        """Set configuration context with validation."""
        self.validate_integrity()
        if key in ['zero_trust_mode', 'integrity_valid']:
            raise ValueError(f"Cannot modify protected setting: {key}")
        self.context[key] = value

# Global IOC instance
ioc = IOCContainer()
