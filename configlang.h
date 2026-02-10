/**
 * configlang.h
 * 
 * Embedded configuration and automation language library
 * Pure C99, no external dependencies
 */

#ifndef CONFIGLANG_H
#define CONFIGLANG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Error codes */
#define ERR_CFG_OK                  0
#define ERR_CFG_NULL_POINTER       -1
#define ERR_CFG_FILE_ERROR         -2
#define ERR_CFG_PARSE_ERROR        -3
#define ERR_CFG_VARIABLE_NOT_FOUND -4
#define ERR_CFG_CONST_VIOLATION    -5
#define ERR_CFG_OUT_OF_MEMORY      -6
#define ERR_CFG_TYPE_MISMATCH      -7
#define ERR_CFG_UNKNOWN_ERROR      -8

/* Opaque type - internal structure hidden */
typedef struct ConfigLang ConfigLang;

/**
 * Create a new ConfigLang instance
 * Returns: pointer to ConfigLang or NULL on failure
 */
ConfigLang* cfg_create(void);

/**
 * Destroy a ConfigLang instance and free resources
 */
void cfg_destroy(ConfigLang* cfg);

/**
 * Load and execute configuration from a file
 * Returns: ERR_CFG_OK on success, error code otherwise
 */
int cfg_load_file(ConfigLang* cfg, const char* path);

/**
 * Load and execute configuration from a string
 * Returns: ERR_CFG_OK on success, error code otherwise
 */
int cfg_load_string(ConfigLang* cfg, const char* code);

/**
 * Get integer value of a variable
 * Returns: ERR_CFG_OK on success, error code otherwise
 */
int cfg_get_int(ConfigLang* cfg, const char* name, int* out);

/**
 * Get string value of a variable
 * Returns: ERR_CFG_OK on success, error code otherwise
 * Note: returned string is internal - do not free
 */
int cfg_get_string(ConfigLang* cfg, const char* name, const char** out);

/**
 * Set integer value of a variable
 * Returns: ERR_CFG_OK on success, ERR_CFG_CONST_VIOLATION if variable is const
 */
int cfg_set_int(ConfigLang* cfg, const char* name, int value);

/**
 * Save current configuration state to a file
 * Returns: ERR_CFG_OK on success, error code otherwise
 */
int cfg_save_file(ConfigLang* cfg, const char* path);

/**
 * Get last error message (useful for debugging)
 * Returns: pointer to error message string
 */
const char* cfg_get_error(ConfigLang* cfg);

#ifdef __cplusplus
}
#endif

#endif /* CONFIGLANG_H */