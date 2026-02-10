# ConfigLang

An embedded configuration and automation language library written in pure C99 with Python bindings. ConfigLang provides a simple, readable syntax for configuration files with support for variables, conditionals, and type safety.

## Features

- **Pure C99** - No external dependencies
- **Python Bindings** - Easy integration with Python projects
- **Type System** - Integer and string types with type checking
- **Const Variables** - Immutable configuration values
- **Conditionals** - If logic for dynamic configuration
- **Variable References** - Copy values between variables
- **Multiline Strings** - Support for complex text values
- **Comments** - Single-line comments with `#`
- **Save/Load** - Persist configuration state to files

## Language Syntax

### Critical Syntax Rules

**Important constraints to remember:**

1. **Variables must be defined before use** - You cannot reference a variable in a conditional before it has been declared with `set`
2. **Single statement per if block** - Each if block can contain only ONE `set` statement
3. **Closing brace on same line** - The `}` must appear on the same line as the statement inside the if block
4. **No else clause** - Use chained if statements instead of if-else

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

**CORRECT:** Basic if statement with closing brace on same line:

```
set value = 60
if value > 50 { set value = 50 }
```

**CORRECT:** Variable must be defined before use in condition:

```
set debug = 0
set log_level = "info"
if debug == 1 { set log_level = "debug" }
```

**CORRECT:** Chained if statements for multiple conditions:

```
set environment = "production"
set timeout = 30

if environment == "development" { set timeout = 300 }
if environment == "staging" { set timeout = 100 }
if environment == "production" { set timeout = 30 }
```

**CORRECT:** Chained if statements (if-if pattern):

```
set x = 55
if x > 50 { set x = 50 } if x < 10 { set x = 10 }
```

**INCORRECT:** Closing brace on separate line:

```
# This will fail to parse!
if value > 50 { 
    set value = 50
}
```

**INCORRECT:** Multiple statements in if block:

```
# This will fail - only ONE set per if block!
if debug == 1 {
    set verbose = 1
    set trace = 1
}
```

**INCORRECT:** Variable used before definition:

```
# This will fail - x is not defined yet!
if x > 50 { set x = 50 }
set x = 55
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
set x = 10  # Inline comment
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
        "const set max_connections = 100\n"
        "if port < 1024 { set port = 8080 }\n";
    
    int result = cfg_load_string(cfg, code);
    if (result != ERR_CFG_OK) {
        fprintf(stderr, "Error: %s\n", cfg_get_error(cfg));
        cfg_destroy(cfg);
        return 1;
    }
    
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
set debug = 0
set port = 8080
set host = "localhost"
const set max_connections = 100
if debug == 1 { set port = 3000 }
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
| 1 | `ERR_CFG_PARSE_ERROR` | Syntax error in configuration |
| 2 | `ERR_CFG_FILE_ERROR` | File I/O error |
| 3 | `ERR_CFG_VARIABLE_NOT_FOUND` | Variable doesn't exist |
| 4 | `ERR_CFG_CONST_VIOLATION` | Attempted to modify const variable |
| 5 | `ERR_CFG_TYPE_MISMATCH` | Variable type mismatch |
| 6 | `ERR_CFG_NULL_POINTER` | NULL pointer passed |
| 7 | `ERR_CFG_OUT_OF_MEMORY` | Memory allocation failed |

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
set debug = 0
set host = "0.0.0.0"
set port = 8080

# Apply production settings
if debug == 0 { set port = 80 }
if debug == 0 { set workers = 4 }

# Apply development settings
if debug == 1 { set port = 8080 }
if debug == 1 { set workers = 1 }

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

## Complete Working Example

This example demonstrates all features with correct syntax:

```
# ============================================
# Complete ConfigLang Example
# ============================================

# Constants (immutable)
const set APP_NAME = "WebServer"
const set VERSION = "2.0.0"
const set MAX_USERS = 1000

# Environment variable
set environment = "production"

# Default values
set port = 8080
set host = "localhost"
set workers = 2
set timeout = 30
set log_level = "info"

# Production configuration
if environment == "production" { set port = 80 }
if environment == "production" { set host = "0.0.0.0" }
if environment == "production" { set workers = 8 }
if environment == "production" { set log_level = "warning" }

# Development configuration  
if environment == "development" { set port = 3000 }
if environment == "development" { set host = "127.0.0.1" }
if environment == "development" { set workers = 1 }
if environment == "development" { set log_level = "debug" }

# Staging configuration
if environment == "staging" { set port = 8080 }
if environment == "staging" { set workers = 4 }

# Database settings
set db_enabled = 1
set db_host = "localhost"
set db_port = 5432

if db_enabled == 1 { set db_name = "production_db" }
if db_enabled == 0 { set db_name = "test_db" }

# Feature flags
set feature_cache = 1
set feature_logging = 1

if feature_cache == 1 { set cache_size = 1024 }
if feature_logging == 1 { set log_size = 100 }

# Variable references
set backup_host = host
set backup_port = port

# Multiline content
set welcome = #%%%
=========================================
  Welcome to WebServer v2.0.0
  Environment: production
  Running on: 0.0.0.0:80
=========================================
%%%#
```

## Language Grammar

```
program      → statement*
statement    → comment | set_stmt | if_stmt
comment      → '#' text '\n'
set_stmt     → 'const'? 'set' identifier '=' value
if_stmt      → 'if' condition '{' set_stmt '}' if_stmt?
condition    → identifier operator value
operator     → '>' | '<' | '>=' | '<=' | '==' | '!='
value        → integer | string | identifier | multiline
integer      → '-'? [0-9]+
string       → '"' [^"]* '"'
multiline    → '#%%%' .* '%%%#'
identifier   → [a-zA-Z_][a-zA-Z0-9_]*
```

**Key Grammar Notes:**
- `if_stmt` can only contain ONE `set_stmt` inside the braces
- The closing `}` must be on the same line as the `set_stmt`
- Chaining is done by following one `if_stmt` with another `if_stmt`
- There is no separate `else` keyword - use chained `if` statements

## Type System

ConfigLang has a simple type system with two types:

1. **Integer** - Signed integers (e.g., `42`, `-10`, `0`)
2. **String** - Text values (e.g., `"Hello"`, `"Config"`)

Types are checked at runtime. Attempting to get or set a variable with the wrong type will result in a `TYPE_MISMATCH` error.

## Implementation Limits

The C implementation defines these limits (configurable in source):

- `MAX_VARIABLES` - 128 variables maximum
- `MAX_VAR_NAME` - 32 characters for variable names
- `MAX_STRING_VALUE` - 1024 characters for string values
- `MAX_LINE_LENGTH` - 2048 characters per line
- `MAX_ERROR_MSG` - 256 characters for error messages

## Limitations

- No floating-point numbers (integers only)
- No arithmetic operations (use for configuration, not computation)
- No loops (conditionals only)
- No functions or procedures
- No arrays or complex data structures
- String operations are limited to assignment
- **Only one `set` statement per `if` block**
- **No `else` keyword** - use chained `if` statements instead
- **Closing brace must be on same line** as the statement
- **Variables must be declared before use** in conditionals

## Use Cases

ConfigLang is designed for:

- Application configuration files
- Build system configuration
- Environment-specific settings
- Feature flags and toggles
- Embedded system configuration
- Game settings and preferences
- Simple automation scripts

## Best Practices

1. **Define variables before conditionals**: Always declare variables with `set` before using them in `if` conditions
2. **Use chained if statements**: Instead of if-else, use multiple if statements
3. **Keep conditionals simple**: Each if block should modify only one variable
4. **Use const for fixed values**: Mark configuration values that shouldn't change as `const`
5. **Group related settings**: Use comments to organize your configuration into logical sections
6. **One line per if block**: Keep the syntax compact with `if condition { set var = value }`

## Common Patterns

### Environment-Based Configuration

```
set env = "production"
set port = 80

if env == "development" { set port = 3000 }
if env == "staging" { set port = 8080 }
if env == "production" { set port = 80 }
```

### Feature Flags

```
set feature_x = 1
set feature_y = 0

set module_x_enabled = 0
set module_y_enabled = 0

if feature_x == 1 { set module_x_enabled = 1 }
if feature_y == 1 { set module_y_enabled = 1 }
```

### Conditional Defaults

```
set mode = 1
set timeout = 30

if mode == 0 { set timeout = 10 }
if mode == 1 { set timeout = 30 }
if mode == 2 { set timeout = 60 }
```

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