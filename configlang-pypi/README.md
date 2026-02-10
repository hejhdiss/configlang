# ConfigLang

An embedded configuration and automation language library written in pure C99 with Python bindings. ConfigLang provides a simple, readable syntax for configuration files with support for variables, conditionals, and type safety.

## Features

- **Pure C99** - No external dependencies
- **Python Bindings** - Easy integration with Python projects
- **Type System** - Integer and string types with type checking
- **Const Variables** - Immutable configuration values
- **Conditionals** - If/else logic for dynamic configuration
- **Variable References** - Copy values between variables
- **Multiline Strings** - Support for complex text values
- **Comments** - Single-line comments with `#`
- **Save/Load** - Persist configuration state to files

## Language Syntax

### Variables

Set integer or string variables:

```
set x = 10
set name = "Hello World"
set count = 42
```

### Const Variables

Create immutable variables:

```
const set max_users = 100
const set app_name = "MyApp"
```

Attempting to modify a const variable will raise an error.

### Variable References

Copy values from one variable to another:

```
set original = 42
set copy = original

set name = "Config"
set backup = name
```

### Conditionals

Basic if statement:

```
set value = 60
if value > 50 { set value = 50 }
```

If-else statement:

```
set score = 5
if score > 10 { set score = 10 } { set score = 90 }
```

Nested conditionals:

```
set x = 55
if x > 50 { set x = 50 } if x < 10 { set x = 10 } { set x = 20 }
```

### Comparison Operators

All standard comparison operators are supported:

- `>` - Greater than
- `<` - Less than
- `>=` - Greater than or equal
- `<=` - Less than or equal
- `==` - Equal to
- `!=` - Not equal to

Example:

```
set x = 10
if x >= 10 { set ready = 1 }
if x != 0 { set active = 1 }
```

### Multiline Strings

For complex text content, use multiline delimiters:

```
set config = #%%%
line 1
line 2
line 3
%%%#
```

### Comments

Single-line comments start with `#`:

```
# This is a comment
set x = 10  # Another comment
```

## Installation

### C Library

Compile the C library:

```bash
# Linux/macOS
gcc -shared -fPIC -std=c99 -o libconfiglang.so configlang.c

# macOS (alternative)
gcc -shared -fPIC -std=c99 -o libconfiglang.dylib configlang.c

# Windows
gcc -shared -o configlang.dll configlang.c
```

Compile the test program:

```bash
gcc -std=c99 -o test test.c configlang.c
./test
```

### Python Package

Install from source:

```bash
pip install .
```

Or install in development mode:

```bash
pip install -e .
```

## Usage Examples

### C API

```c
#include "configlang.h"
#include <stdio.h>

int main(void) {
    // Create instance
    ConfigLang* cfg = cfg_create();
    
    // Load configuration
    const char* code = 
        "set port = 8080\n"
        "set host = \"localhost\"\n"
        "const set max_connections = 100\n";
    
    cfg_load_string(cfg, code);
    
    // Get values
    int port;
    const char* host;
    
    cfg_get_int(cfg, "port", &port);
    cfg_get_string(cfg, "host", &host);
    
    printf("Server: %s:%d\n", host, port);
    
    // Modify value
    cfg_set_int(cfg, "port", 9000);
    
    // Save configuration
    cfg_save_file(cfg, "server.cfg");
    
    // Cleanup
    cfg_destroy(cfg);
    
    return 0;
}
```

### Python API

```python
from configlang import ConfigLang

# Create instance
cfg = ConfigLang()

# Load configuration
code = """
set port = 8080
set host = "localhost"
const set max_connections = 100
if port < 1024 { set port = 8080 }
"""

cfg.load_string(code)

# Get values
print(f"Port: {cfg.get_int('port')}")
print(f"Host: {cfg.get_string('host')}")

# Modify value
cfg.set_int('port', 9000)

# Dictionary-style access
cfg['port'] = 3000
print(f"Port: {cfg['port']}")

# Save configuration
cfg.save_file('server.cfg')
```

### Context Manager (Python)

```python
with ConfigLang() as cfg:
    cfg.load_string('set x = 10\nset name = "Test"')
    print(cfg.get_int('x'))
    # Automatically cleaned up
```

## API Reference

### C Functions

| Function | Description |
|----------|-------------|
| `cfg_create()` | Create a new ConfigLang instance |
| `cfg_destroy(cfg)` | Destroy instance and free resources |
| `cfg_load_file(cfg, path)` | Load configuration from file |
| `cfg_load_string(cfg, code)` | Load configuration from string |
| `cfg_get_int(cfg, name, out)` | Get integer variable value |
| `cfg_get_string(cfg, name, out)` | Get string variable value |
| `cfg_set_int(cfg, name, value)` | Set integer variable value |
| `cfg_save_file(cfg, path)` | Save configuration to file |
| `cfg_get_error(cfg)` | Get last error message |

### Python Methods

| Method | Description |
|--------|-------------|
| `ConfigLang()` | Create new instance |
| `load_file(path)` | Load configuration from file |
| `load_string(code)` | Load configuration from string |
| `get_int(name)` | Get integer variable value |
| `get_string(name)` | Get string variable value |
| `set_int(name, value)` | Set integer variable value |
| `save_file(path)` | Save configuration to file |
| `get_error()` | Get last error message |
| `get(name)` | Auto-detect type and get value |

### Error Codes

| Code | Constant | Description |
|------|----------|-------------|
| 0 | `ERR_CFG_OK` | Success |
| -1 | `ERR_CFG_NULL_POINTER` | NULL pointer passed |
| -2 | `ERR_CFG_FILE_ERROR` | File I/O error |
| -3 | `ERR_CFG_PARSE_ERROR` | Syntax error in configuration |
| -4 | `ERR_CFG_VARIABLE_NOT_FOUND` | Variable doesn't exist |
| -5 | `ERR_CFG_CONST_VIOLATION` | Attempted to modify const variable |
| -6 | `ERR_CFG_OUT_OF_MEMORY` | Memory allocation failed |
| -7 | `ERR_CFG_TYPE_MISMATCH` | Variable type mismatch |
| -8 | `ERR_CFG_UNKNOWN_ERROR` | Unknown error |

### Python Exceptions

- `ConfigLangError` - Base exception class
- `ParseError` - Syntax error in configuration
- `VariableNotFoundError` - Variable doesn't exist
- `ConstViolationError` - Attempted to modify const variable
- `TypeMismatchError` - Variable type mismatch

## Example Configuration File

```
# Application Configuration
const set app_name = "MyApplication"
const set version = "1.0.0"

# Server Settings
set host = "0.0.0.0"
set port = 8080
set debug = 0

# Apply production settings
if debug == 0 {
    set port = 80
    set workers = 4
}

# Apply development settings
if debug == 1 {
    set port = 8080
    set workers = 1
}

# Database Configuration
set db_host = "localhost"
set db_port = 5432
set db_name = "myapp"

# Limits
const set max_connections = 100
const set timeout = 30

# Multiline description
set description = #%%%
This is a multi-line
application description
that spans several lines.
%%%#
```

## Language Grammar

```
program      → statement*
statement    → comment | set_stmt | if_stmt
comment      → '#' text '\n'
set_stmt     → 'const'? 'set' identifier '=' value
if_stmt      → 'if' condition '{' statement* '}' else_block?
else_block   → 'if' condition '{' statement* '}' else_block?
              | '{' statement* '}'
condition    → identifier operator value
operator     → '>' | '<' | '>=' | '<=' | '==' | '!='
value        → integer | string | identifier | multiline
integer      → [0-9]+
string       → '"' [^"]* '"'
multiline    → '#%%%' .* '%%%#'
identifier   → [a-zA-Z_][a-zA-Z0-9_]*
```

## Type System

ConfigLang has a simple type system with two types:

1. **Integer** - Signed integers (e.g., `42`, `-10`, `0`)
2. **String** - Text values (e.g., `"Hello"`, `"Config"`)

Types are checked at runtime. Attempting to get or set a variable with the wrong type will result in a `TYPE_MISMATCH` error.

## Limitations

- No floating-point numbers (integers only)
- No arithmetic operations (use for configuration, not computation)
- No loops (conditionals only)
- No functions or procedures
- No arrays or complex data structures
- String operations are limited to assignment

## Use Cases

ConfigLang is designed for:

- Application configuration files
- Build system configuration
- Environment-specific settings
- Feature flags and toggles
- Embedded system configuration
- Game settings and preferences
- Simple automation scripts

## License

MIT License - See LICENSE file for details

## Author

hejhdiss

## Links

- **GitHub**: https://github.com/hejhdiss/configlang
- **Issues**: https://github.com/hejhdiss/configlang/issues

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

## Changelog

### Version 1.0.0
- Initial release
- C library implementation
- Python bindings
- Full test suite
- Documentation