"""
ConfigLang - Python wrapper for the embedded configuration language library

This module provides a Python interface to the ConfigLang C library using ctypes.

Example usage:
    >>> from configlang import ConfigLang
    >>> cfg = ConfigLang()
    >>> cfg.load_string('set x = 10\\nset name = "Hello"')
    >>> print(cfg.get_int('x'))
    10
    >>> print(cfg.get_string('name'))
    'Hello'
"""

import ctypes
import os
from typing import Optional, Union
from pathlib import Path


# Error codes (matching C library)
ERR_CFG_OK = 0
ERR_CFG_NULL_POINTER = -1
ERR_CFG_FILE_ERROR = -2
ERR_CFG_PARSE_ERROR = -3
ERR_CFG_VARIABLE_NOT_FOUND = -4
ERR_CFG_CONST_VIOLATION = -5
ERR_CFG_OUT_OF_MEMORY = -6
ERR_CFG_TYPE_MISMATCH = -7
ERR_CFG_UNKNOWN_ERROR = -8


class ConfigLangError(Exception):
    """Base exception for ConfigLang errors"""
    pass


class ParseError(ConfigLangError):
    """Raised when parsing fails"""
    pass


class VariableNotFoundError(ConfigLangError):
    """Raised when a variable is not found"""
    pass


class ConstViolationError(ConfigLangError):
    """Raised when attempting to modify a const variable"""
    pass


class TypeMismatchError(ConfigLangError):
    """Raised when variable type doesn't match expected type"""
    pass


class ConfigLang:
    """
    Python wrapper for ConfigLang C library.
    
    This class provides a Pythonic interface to the embedded configuration
    language, handling memory management and error handling automatically.
    
    Attributes:
        _lib: The loaded C library
        _cfg: Pointer to the C ConfigLang structure
    """
    
    def __init__(self, lib_path: Optional[str] = None):
        """
        Initialize ConfigLang instance.
        
        Args:
            lib_path: Path to the compiled shared library. If None, searches
                     for 'libconfiglang.so' (Linux), 'libconfiglang.dylib' (macOS),
                     or 'configlang.dll' (Windows) in common locations.
        
        Raises:
            FileNotFoundError: If the library cannot be found
            ConfigLangError: If initialization fails
        """
        # Load the shared library
        if lib_path is None:
            lib_path = self._find_library()
        
        if not os.path.exists(lib_path):
            raise FileNotFoundError(f"ConfigLang library not found: {lib_path}")
        
        self._lib = ctypes.CDLL(lib_path)
        self._setup_functions()
        
        # Create ConfigLang instance
        self._cfg = self._lib.cfg_create()
        if not self._cfg:
            raise ConfigLangError("Failed to create ConfigLang instance")
    
    def __del__(self):
        """Cleanup when object is destroyed"""
        if hasattr(self, '_cfg') and self._cfg:
            self._lib.cfg_destroy(self._cfg)
    
    def __enter__(self):
        """Context manager entry"""
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager exit"""
        if self._cfg:
            self._lib.cfg_destroy(self._cfg)
            self._cfg = None
        return False
    
    def _find_library(self) -> str:
        """
        Attempt to find the ConfigLang shared library.
        
        Returns:
            Path to the library
            
        Raises:
            FileNotFoundError: If library cannot be found
        """
        # Determine library name based on platform
        import platform
        system = platform.system()
        
        if system == 'Linux':
            lib_names = ['libconfiglang.so', 'configlang.so']
        elif system == 'Darwin':  # macOS
            lib_names = ['libconfiglang.dylib', 'configlang.dylib']
        elif system == 'Windows':
            lib_names = ['configlang.dll', 'libconfiglang.dll']
        else:
            lib_names = ['libconfiglang.so']
        
        # Search locations
        search_paths = [
            os.path.dirname(__file__),  # Same directory as this file
            os.getcwd(),  # Current working directory
            '/usr/local/lib',
            '/usr/lib',
        ]
        
        for path in search_paths:
            for lib_name in lib_names:
                lib_path = os.path.join(path, lib_name)
                if os.path.exists(lib_path):
                    return lib_path
        
        raise FileNotFoundError(
            f"ConfigLang library not found. Searched for {lib_names} in {search_paths}. "
            "Please compile the C library and specify its path."
        )
    
    def _setup_functions(self):
        """Setup C function signatures using ctypes"""
        # cfg_create
        self._lib.cfg_create.argtypes = []
        self._lib.cfg_create.restype = ctypes.c_void_p
        
        # cfg_destroy
        self._lib.cfg_destroy.argtypes = [ctypes.c_void_p]
        self._lib.cfg_destroy.restype = None
        
        # cfg_load_file
        self._lib.cfg_load_file.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        self._lib.cfg_load_file.restype = ctypes.c_int
        
        # cfg_load_string
        self._lib.cfg_load_string.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        self._lib.cfg_load_string.restype = ctypes.c_int
        
        # cfg_get_int
        self._lib.cfg_get_int.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.POINTER(ctypes.c_int)]
        self._lib.cfg_get_int.restype = ctypes.c_int
        
        # cfg_get_string
        self._lib.cfg_get_string.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.POINTER(ctypes.c_char_p)]
        self._lib.cfg_get_string.restype = ctypes.c_int
        
        # cfg_set_int
        self._lib.cfg_set_int.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_int]
        self._lib.cfg_set_int.restype = ctypes.c_int
        
        # cfg_save_file
        self._lib.cfg_save_file.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        self._lib.cfg_save_file.restype = ctypes.c_int
        
        # cfg_get_error
        self._lib.cfg_get_error.argtypes = [ctypes.c_void_p]
        self._lib.cfg_get_error.restype = ctypes.c_char_p
    
    def _check_error(self, error_code: int):
        """
        Check error code and raise appropriate exception if needed.
        
        Args:
            error_code: Return code from C function
            
        Raises:
            Appropriate ConfigLangError subclass based on error code
        """
        if error_code == ERR_CFG_OK:
            return
        
        error_msg = self.get_error()
        
        if error_code == ERR_CFG_PARSE_ERROR:
            raise ParseError(error_msg)
        elif error_code == ERR_CFG_VARIABLE_NOT_FOUND:
            raise VariableNotFoundError(error_msg)
        elif error_code == ERR_CFG_CONST_VIOLATION:
            raise ConstViolationError(error_msg)
        elif error_code == ERR_CFG_TYPE_MISMATCH:
            raise TypeMismatchError(error_msg)
        else:
            raise ConfigLangError(f"Error {error_code}: {error_msg}")
    
    def load_file(self, path: Union[str, Path]) -> None:
        """
        Load and execute configuration from a file.
        
        Args:
            path: Path to the configuration file
            
        Raises:
            ParseError: If parsing fails
            ConfigLangError: For other errors
        """
        path_str = str(path).encode('utf-8')
        result = self._lib.cfg_load_file(self._cfg, path_str)
        self._check_error(result)
    
    def load_string(self, code: str) -> None:
        """
        Load and execute configuration from a string.
        
        Args:
            code: Configuration code as a string
            
        Raises:
            ParseError: If parsing fails
            ConfigLangError: For other errors
        """
        code_bytes = code.encode('utf-8')
        result = self._lib.cfg_load_string(self._cfg, code_bytes)
        self._check_error(result)
    
    def get_int(self, name: str) -> int:
        """
        Get the integer value of a variable.
        
        Args:
            name: Variable name
            
        Returns:
            Integer value
            
        Raises:
            VariableNotFoundError: If variable doesn't exist
            TypeMismatchError: If variable is not an integer
        """
        name_bytes = name.encode('utf-8')
        value = ctypes.c_int()
        result = self._lib.cfg_get_int(self._cfg, name_bytes, ctypes.byref(value))
        self._check_error(result)
        return value.value
    
    def get_string(self, name: str) -> str:
        """
        Get the string value of a variable.
        
        Args:
            name: Variable name
            
        Returns:
            String value
            
        Raises:
            VariableNotFoundError: If variable doesn't exist
            TypeMismatchError: If variable is not a string
        """
        name_bytes = name.encode('utf-8')
        value = ctypes.c_char_p()
        result = self._lib.cfg_get_string(self._cfg, name_bytes, ctypes.byref(value))
        self._check_error(result)
        return value.value.decode('utf-8')
    
    def set_int(self, name: str, value: int) -> None:
        """
        Set the integer value of a variable.
        
        Args:
            name: Variable name
            value: New integer value
            
        Raises:
            VariableNotFoundError: If variable doesn't exist
            ConstViolationError: If variable is const
            TypeMismatchError: If variable is not an integer
        """
        name_bytes = name.encode('utf-8')
        result = self._lib.cfg_set_int(self._cfg, name_bytes, value)
        self._check_error(result)
    
    def save_file(self, path: Union[str, Path]) -> None:
        """
        Save current configuration state to a file.
        
        Args:
            path: Path where to save the configuration
            
        Raises:
            ConfigLangError: If save fails
        """
        path_str = str(path).encode('utf-8')
        result = self._lib.cfg_save_file(self._cfg, path_str)
        self._check_error(result)
    
    def get_error(self) -> str:
        """
        Get the last error message.
        
        Returns:
            Error message string
        """
        error_msg = self._lib.cfg_get_error(self._cfg)
        return error_msg.decode('utf-8')
    
    def get(self, name: str) -> Union[int, str]:
        """
        Get a variable value (auto-detects type).
        
        Attempts to get as integer first, falls back to string.
        
        Args:
            name: Variable name
            
        Returns:
            Variable value (int or str)
            
        Raises:
            VariableNotFoundError: If variable doesn't exist
        """
        try:
            return self.get_int(name)
        except TypeMismatchError:
            return self.get_string(name)
    
    def __getitem__(self, name: str) -> Union[int, str]:
        """
        Dictionary-style access to variables.
        
        Args:
            name: Variable name
            
        Returns:
            Variable value
        """
        return self.get(name)
    
    def __setitem__(self, name: str, value: int):
        """
        Dictionary-style setting of variables.
        
        Currently only supports integers.
        
        Args:
            name: Variable name
            value: New value
        """
        if not isinstance(value, int):
            raise TypeError("Only integer values are supported via setitem")
        self.set_int(name, value)


def main():
    """Example usage and tests"""
    print("ConfigLang Python Wrapper - Example Usage\n")
    
    # Create instance
    cfg = ConfigLang()
    
    # Load configuration
    code = """
    set x = 10
    set name = "Python Test"
    const set max = 100
    if x < 20 { set x = 15 }
    """
    
    cfg.load_string(code)
    
    # Get values
    print(f"x = {cfg.get_int('x')}")
    print(f"name = {cfg.get_string('name')}")
    print(f"max = {cfg.get_int('max')}")
    
    # Modify value
    cfg.set_int('x', 42)
    print(f"x (modified) = {cfg.get_int('x')}")
    
    # Try to modify const (will fail)
    try:
        cfg.set_int('max', 200)
    except ConstViolationError as e:
        print(f"✓ Const protection works: {e}")
    
    # Dictionary-style access
    print(f"\nDictionary-style access:")
    print(f"cfg['x'] = {cfg['x']}")
    cfg['x'] = 99
    print(f"cfg['x'] = {cfg['x']}")
    
    # Save configuration
    cfg.save_file('python_test.cfg')
    print(f"\n✓ Saved to python_test.cfg")


if __name__ == '__main__':
    main()