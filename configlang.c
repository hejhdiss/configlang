/**
 * configlang.c
 * 
 * Implementation of embedded configuration language
 */

#include "configlang.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Configuration constants */
#define MAX_VARIABLES 128
#define MAX_VAR_NAME 32
#define MAX_STRING_VALUE 1024
#define MAX_LINE_LENGTH 2048
#define MAX_ERROR_MSG 256

/* Variable types */
typedef enum {
    VAR_TYPE_INT,
    VAR_TYPE_STRING
} VarType;

/* Variable storage */
typedef struct {
    char name[MAX_VAR_NAME];
    VarType type;
    int is_const;
    union {
        int int_val;
        char str_val[MAX_STRING_VALUE];
    } value;
    int in_use;
} Variable;

/* Token types */
typedef enum {
    TOK_EOF,
    TOK_IDENT,
    TOK_NUMBER,
    TOK_STRING,
    TOK_SET,
    TOK_CONST,
    TOK_IF,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_ASSIGN,
    TOK_GT,
    TOK_LT,
    TOK_GTE,
    TOK_LTE,
    TOK_EQ,
    TOK_NEQ,
    TOK_NEWLINE,
    TOK_MULTILINE_START,
    TOK_ERROR
} TokenType;

/* Token structure */
typedef struct {
    TokenType type;
    char text[MAX_STRING_VALUE];
    int int_value;
} Token;

/* Lexer state */
typedef struct {
    const char* input;
    size_t pos;
    size_t length;
    int line_number;
} Lexer;

/* ConfigLang structure */
struct ConfigLang {
    Variable variables[MAX_VARIABLES];
    int var_count;
    char error_msg[MAX_ERROR_MSG];
    int last_error;
};

/* ========================================================================
 * UTILITY FUNCTIONS
 * ======================================================================== */

static void set_error(ConfigLang* cfg, int error_code, const char* msg, int line) {
    cfg->last_error = error_code;
    if (line > 0) {
        snprintf(cfg->error_msg, MAX_ERROR_MSG, "Line %d: %s", line, msg);
    } else {
        snprintf(cfg->error_msg, MAX_ERROR_MSG, "%s", msg);
    }
}

static Variable* find_variable(ConfigLang* cfg, const char* name) {
    for (int i = 0; i < MAX_VARIABLES; i++) {
        if (cfg->variables[i].in_use && strcmp(cfg->variables[i].name, name) == 0) {
            return &cfg->variables[i];
        }
    }
    return NULL;
}

static Variable* create_variable(ConfigLang* cfg, const char* name) {
    /* Find first unused slot */
    for (int i = 0; i < MAX_VARIABLES; i++) {
        if (!cfg->variables[i].in_use) {
            strncpy(cfg->variables[i].name, name, MAX_VAR_NAME - 1);
            cfg->variables[i].name[MAX_VAR_NAME - 1] = '\0';
            cfg->variables[i].in_use = 1;
            cfg->variables[i].is_const = 0;
            if (cfg->var_count <= i) {
                cfg->var_count = i + 1;
            }
            return &cfg->variables[i];
        }
    }
    return NULL;
}

/* ========================================================================
 * LEXER
 * ======================================================================== */

static void lexer_init(Lexer* lex, const char* input) {
    lex->input = input;
    lex->pos = 0;
    lex->length = strlen(input);
    lex->line_number = 1;
}

static char lexer_peek(Lexer* lex) {
    if (lex->pos >= lex->length) return '\0';
    return lex->input[lex->pos];
}

static char lexer_advance(Lexer* lex) {
    if (lex->pos >= lex->length) return '\0';
    char c = lex->input[lex->pos++];
    if (c == '\n') lex->line_number++;
    return c;
}

static void lexer_skip_whitespace(Lexer* lex) {
    while (lexer_peek(lex) == ' ' || lexer_peek(lex) == '\t' || lexer_peek(lex) == '\r') {
        lexer_advance(lex);
    }
}

static int lexer_read_multiline(Lexer* lex, Token* tok) {
    /* Already consumed #%%% */
    char* dest = tok->text;
    size_t max_len = MAX_STRING_VALUE - 1;
    size_t written = 0;
    
    while (lex->pos < lex->length && written < max_len) {
        /* Check for end marker %%%# */
        if (lex->pos + 3 < lex->length &&
            lex->input[lex->pos] == '%' &&
            lex->input[lex->pos + 1] == '%' &&
            lex->input[lex->pos + 2] == '%' &&
            lex->input[lex->pos + 3] == '#') {
            /* Found end marker */
            lex->pos += 4;
            dest[written] = '\0';
            tok->type = TOK_STRING;
            return 1;
        }
        
        char c = lexer_advance(lex);
        dest[written++] = c;
    }
    
    /* Missing end marker */
    tok->type = TOK_ERROR;
    return 0;
}

static Token lexer_next_token(Lexer* lex) {
    Token tok;
    memset(&tok, 0, sizeof(Token));
    
    lexer_skip_whitespace(lex);
    
    char c = lexer_peek(lex);
    
    /* End of input */
    if (c == '\0') {
        tok.type = TOK_EOF;
        return tok;
    }
    
    /* Newline */
    if (c == '\n') {
        lexer_advance(lex);
        tok.type = TOK_NEWLINE;
        return tok;
    }
    
    /* Comment */
    if (c == '#') {
        /* Check for multiline marker #%%% */
        if (lex->pos + 3 < lex->length &&
            lex->input[lex->pos + 1] == '%' &&
            lex->input[lex->pos + 2] == '%' &&
            lex->input[lex->pos + 3] == '%') {
            lex->pos += 4;
            tok.type = TOK_MULTILINE_START;
            lexer_read_multiline(lex, &tok);
            return tok;
        }
        
        /* Regular comment - skip to end of line */
        while (lexer_peek(lex) != '\n' && lexer_peek(lex) != '\0') {
            lexer_advance(lex);
        }
        return lexer_next_token(lex);
    }
    
    /* String literal */
    if (c == '"') {
        lexer_advance(lex);
        size_t i = 0;
        while (lexer_peek(lex) != '"' && lexer_peek(lex) != '\0' && lexer_peek(lex) != '\n') {
            if (i < MAX_STRING_VALUE - 1) {
                tok.text[i++] = lexer_advance(lex);
            } else {
                lexer_advance(lex);
            }
        }
        if (lexer_peek(lex) == '"') {
            lexer_advance(lex);
        }
        tok.text[i] = '\0';
        tok.type = TOK_STRING;
        return tok;
    }
    
    /* Number */
    if (isdigit(c) || (c == '-' && isdigit(lex->input[lex->pos + 1]))) {
        size_t i = 0;
        if (c == '-') {
            tok.text[i++] = lexer_advance(lex);
        }
        while (isdigit(lexer_peek(lex))) {
            if (i < MAX_STRING_VALUE - 1) {
                tok.text[i++] = lexer_advance(lex);
            } else {
                lexer_advance(lex);
            }
        }
        tok.text[i] = '\0';
        tok.int_value = atoi(tok.text);
        tok.type = TOK_NUMBER;
        return tok;
    }
    
    /* Operators */
    if (c == '=') {
        lexer_advance(lex);
        if (lexer_peek(lex) == '=') {
            lexer_advance(lex);
            tok.type = TOK_EQ;
        } else {
            tok.type = TOK_ASSIGN;
        }
        return tok;
    }
    
    if (c == '>') {
        lexer_advance(lex);
        if (lexer_peek(lex) == '=') {
            lexer_advance(lex);
            tok.type = TOK_GTE;
        } else {
            tok.type = TOK_GT;
        }
        return tok;
    }
    
    if (c == '<') {
        lexer_advance(lex);
        if (lexer_peek(lex) == '=') {
            lexer_advance(lex);
            tok.type = TOK_LTE;
        } else {
            tok.type = TOK_LT;
        }
        return tok;
    }
    
    if (c == '!') {
        lexer_advance(lex);
        if (lexer_peek(lex) == '=') {
            lexer_advance(lex);
            tok.type = TOK_NEQ;
        } else {
            tok.type = TOK_ERROR;
        }
        return tok;
    }
    
    if (c == '{') {
        lexer_advance(lex);
        tok.type = TOK_LBRACE;
        return tok;
    }
    
    if (c == '}') {
        lexer_advance(lex);
        tok.type = TOK_RBRACE;
        return tok;
    }
    
    /* Identifier or keyword */
    if (isalpha(c) || c == '_') {
        size_t i = 0;
        while ((isalnum(lexer_peek(lex)) || lexer_peek(lex) == '_') && i < MAX_VAR_NAME - 1) {
            tok.text[i++] = lexer_advance(lex);
        }
        tok.text[i] = '\0';
        
        /* Check for keywords */
        if (strcmp(tok.text, "set") == 0) {
            tok.type = TOK_SET;
        } else if (strcmp(tok.text, "const") == 0) {
            tok.type = TOK_CONST;
        } else if (strcmp(tok.text, "if") == 0) {
            tok.type = TOK_IF;
        } else {
            tok.type = TOK_IDENT;
        }
        return tok;
    }
    
    /* Unknown character */
    tok.type = TOK_ERROR;
    return tok;
}

/* ========================================================================
 * PARSER / INTERPRETER
 * ======================================================================== */

typedef struct {
    Lexer* lexer;
    Token current;
    Token peek;
    ConfigLang* cfg;
} Parser;

static void parser_advance(Parser* p) {
    p->current = p->peek;
    p->peek = lexer_next_token(p->lexer);
}

static void parser_init(Parser* p, Lexer* lex, ConfigLang* cfg) {
    p->lexer = lex;
    p->cfg = cfg;
    p->peek = lexer_next_token(lex);
    parser_advance(p);
}

static int eval_condition(Parser* p, int* result);
static int parse_statement(Parser* p);

/* Parse expression (for assignment values) */
static int parse_value(Parser* p, Variable* var) {
    if (p->current.type == TOK_NUMBER) {
        var->type = VAR_TYPE_INT;
        var->value.int_val = p->current.int_value;
        parser_advance(p);
        return ERR_CFG_OK;
    } else if (p->current.type == TOK_STRING) {
        var->type = VAR_TYPE_STRING;
        strncpy(var->value.str_val, p->current.text, MAX_STRING_VALUE - 1);
        var->value.str_val[MAX_STRING_VALUE - 1] = '\0';
        parser_advance(p);
        return ERR_CFG_OK;
    } else if (p->current.type == TOK_IDENT) {
        /* Reference to another variable */
        Variable* src = find_variable(p->cfg, p->current.text);
        if (!src) {
            set_error(p->cfg, ERR_CFG_VARIABLE_NOT_FOUND, "Variable not found", p->lexer->line_number);
            return ERR_CFG_VARIABLE_NOT_FOUND;
        }
        var->type = src->type;
        if (src->type == VAR_TYPE_INT) {
            var->value.int_val = src->value.int_val;
        } else {
            strncpy(var->value.str_val, src->value.str_val, MAX_STRING_VALUE - 1);
            var->value.str_val[MAX_STRING_VALUE - 1] = '\0';
        }
        parser_advance(p);
        return ERR_CFG_OK;
    }
    
    set_error(p->cfg, ERR_CFG_PARSE_ERROR, "Expected value", p->lexer->line_number);
    return ERR_CFG_PARSE_ERROR;
}

/* Parse set statement: set name = value */
static int parse_set(Parser* p, int is_const) {
    parser_advance(p); /* consume 'set' */
    
    if (p->current.type != TOK_IDENT) {
        set_error(p->cfg, ERR_CFG_PARSE_ERROR, "Expected variable name", p->lexer->line_number);
        return ERR_CFG_PARSE_ERROR;
    }
    
    char var_name[MAX_VAR_NAME];
    strncpy(var_name, p->current.text, MAX_VAR_NAME - 1);
    var_name[MAX_VAR_NAME - 1] = '\0';
    
    parser_advance(p);
    
    if (p->current.type != TOK_ASSIGN) {
        set_error(p->cfg, ERR_CFG_PARSE_ERROR, "Expected '='", p->lexer->line_number);
        return ERR_CFG_PARSE_ERROR;
    }
    
    parser_advance(p);
    
    /* Check if variable exists */
    Variable* var = find_variable(p->cfg, var_name);
    if (var) {
        /* Variable exists - check if const */
        if (var->is_const) {
            set_error(p->cfg, ERR_CFG_CONST_VIOLATION, "Cannot modify const variable", p->lexer->line_number);
            return ERR_CFG_CONST_VIOLATION;
        }
    } else {
        /* Create new variable */
        var = create_variable(p->cfg, var_name);
        if (!var) {
            set_error(p->cfg, ERR_CFG_OUT_OF_MEMORY, "Too many variables", p->lexer->line_number);
            return ERR_CFG_OUT_OF_MEMORY;
        }
        var->is_const = is_const;
    }
    
    return parse_value(p, var);
}

/* Evaluate condition and return result */
static int eval_condition(Parser* p, int* result) {
    /* Get left side */
    if (p->current.type != TOK_IDENT && p->current.type != TOK_NUMBER) {
        set_error(p->cfg, ERR_CFG_PARSE_ERROR, "Expected identifier or number", p->lexer->line_number);
        return ERR_CFG_PARSE_ERROR;
    }
    
    int left_val;
    if (p->current.type == TOK_NUMBER) {
        left_val = p->current.int_value;
    } else {
        Variable* var = find_variable(p->cfg, p->current.text);
        if (!var) {
            set_error(p->cfg, ERR_CFG_VARIABLE_NOT_FOUND, "Variable not found in condition", p->lexer->line_number);
            return ERR_CFG_VARIABLE_NOT_FOUND;
        }
        if (var->type != VAR_TYPE_INT) {
            set_error(p->cfg, ERR_CFG_TYPE_MISMATCH, "Condition requires integer", p->lexer->line_number);
            return ERR_CFG_TYPE_MISMATCH;
        }
        left_val = var->value.int_val;
    }
    
    parser_advance(p);
    
    /* Get operator */
    TokenType op = p->current.type;
    if (op != TOK_GT && op != TOK_LT && op != TOK_GTE && op != TOK_LTE && op != TOK_EQ && op != TOK_NEQ) {
        set_error(p->cfg, ERR_CFG_PARSE_ERROR, "Expected comparison operator", p->lexer->line_number);
        return ERR_CFG_PARSE_ERROR;
    }
    
    parser_advance(p);
    
    /* Get right side */
    if (p->current.type != TOK_IDENT && p->current.type != TOK_NUMBER) {
        set_error(p->cfg, ERR_CFG_PARSE_ERROR, "Expected identifier or number", p->lexer->line_number);
        return ERR_CFG_PARSE_ERROR;
    }
    
    int right_val;
    if (p->current.type == TOK_NUMBER) {
        right_val = p->current.int_value;
    } else {
        Variable* var = find_variable(p->cfg, p->current.text);
        if (!var) {
            set_error(p->cfg, ERR_CFG_VARIABLE_NOT_FOUND, "Variable not found in condition", p->lexer->line_number);
            return ERR_CFG_VARIABLE_NOT_FOUND;
        }
        if (var->type != VAR_TYPE_INT) {
            set_error(p->cfg, ERR_CFG_TYPE_MISMATCH, "Condition requires integer", p->lexer->line_number);
            return ERR_CFG_TYPE_MISMATCH;
        }
        right_val = var->value.int_val;
    }
    
    parser_advance(p);
    
    /* Evaluate */
    switch (op) {
        case TOK_GT:  *result = (left_val > right_val); break;
        case TOK_LT:  *result = (left_val < right_val); break;
        case TOK_GTE: *result = (left_val >= right_val); break;
        case TOK_LTE: *result = (left_val <= right_val); break;
        case TOK_EQ:  *result = (left_val == right_val); break;
        case TOK_NEQ: *result = (left_val != right_val); break;
        default: *result = 0;
    }
    
    return ERR_CFG_OK;
}

/* Parse if statement */
static int parse_if(Parser* p) {
    parser_advance(p); /* consume 'if' */
    
    int condition;
    int err = eval_condition(p, &condition);
    if (err != ERR_CFG_OK) return err;
    
    /* Expect { */
    if (p->current.type != TOK_LBRACE) {
        set_error(p->cfg, ERR_CFG_PARSE_ERROR, "Expected '{'", p->lexer->line_number);
        return ERR_CFG_PARSE_ERROR;
    }
    parser_advance(p);
    
    /* Parse then block */
    if (condition) {
        err = parse_statement(p);
        if (err != ERR_CFG_OK) return err;
    } else {
        /* Skip then block */
        while (p->current.type != TOK_RBRACE && p->current.type != TOK_EOF) {
            parser_advance(p);
        }
    }
    
    /* Expect } */
    if (p->current.type != TOK_RBRACE) {
        set_error(p->cfg, ERR_CFG_PARSE_ERROR, "Expected '}'", p->lexer->line_number);
        return ERR_CFG_PARSE_ERROR;
    }
    parser_advance(p);
    
    /* Check for else block or nested if */
    if (p->current.type == TOK_IF) {
        /* Nested if - this is a chained condition */
        return parse_if(p);
    } else if (p->current.type == TOK_LBRACE) {
        /* Else block */
        parser_advance(p);
        
        if (!condition) {
            err = parse_statement(p);
            if (err != ERR_CFG_OK) return err;
        } else {
            /* Skip else block */
            while (p->current.type != TOK_RBRACE && p->current.type != TOK_EOF) {
                parser_advance(p);
            }
        }
        
        if (p->current.type != TOK_RBRACE) {
            set_error(p->cfg, ERR_CFG_PARSE_ERROR, "Expected '}'", p->lexer->line_number);
            return ERR_CFG_PARSE_ERROR;
        }
        parser_advance(p);
    }
    
    return ERR_CFG_OK;
}

/* Parse single statement */
static int parse_statement(Parser* p) {
    /* Skip empty lines */
    while (p->current.type == TOK_NEWLINE) {
        parser_advance(p);
    }
    
    if (p->current.type == TOK_EOF) {
        return ERR_CFG_OK;
    }
    
    if (p->current.type == TOK_CONST) {
        parser_advance(p);
        if (p->current.type != TOK_SET) {
            set_error(p->cfg, ERR_CFG_PARSE_ERROR, "Expected 'set' after 'const'", p->lexer->line_number);
            return ERR_CFG_PARSE_ERROR;
        }
        return parse_set(p, 1);
    }
    
    if (p->current.type == TOK_SET) {
        return parse_set(p, 0);
    }
    
    if (p->current.type == TOK_IF) {
        return parse_if(p);
    }
    
    set_error(p->cfg, ERR_CFG_PARSE_ERROR, "Unexpected token", p->lexer->line_number);
    return ERR_CFG_PARSE_ERROR;
}

/* Parse entire program */
static int parse_program(Parser* p) {
    while (p->current.type != TOK_EOF) {
        int err = parse_statement(p);
        if (err != ERR_CFG_OK) {
            return err;
        }
        
        /* Consume newlines */
        while (p->current.type == TOK_NEWLINE) {
            parser_advance(p);
        }
    }
    
    return ERR_CFG_OK;
}

/* ========================================================================
 * PUBLIC API IMPLEMENTATION
 * ======================================================================== */

ConfigLang* cfg_create(void) {
    ConfigLang* cfg = (ConfigLang*)malloc(sizeof(ConfigLang));
    if (!cfg) return NULL;
    
    memset(cfg, 0, sizeof(ConfigLang));
    cfg->var_count = 0;
    cfg->last_error = ERR_CFG_OK;
    strcpy(cfg->error_msg, "No error");
    
    return cfg;
}

void cfg_destroy(ConfigLang* cfg) {
    if (cfg) {
        free(cfg);
    }
}

int cfg_load_string(ConfigLang* cfg, const char* code) {
    if (!cfg || !code) return ERR_CFG_NULL_POINTER;
    
    Lexer lex;
    lexer_init(&lex, code);
    
    Parser parser;
    parser_init(&parser, &lex, cfg);
    
    int result = parse_program(&parser);
    return result;
}

int cfg_load_file(ConfigLang* cfg, const char* path) {
    if (!cfg || !path) return ERR_CFG_NULL_POINTER;
    
    FILE* f = fopen(path, "r");
    if (!f) {
        set_error(cfg, ERR_CFG_FILE_ERROR, "Cannot open file", 0);
        return ERR_CFG_FILE_ERROR;
    }
    
    /* Read entire file */
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char* buffer = (char*)malloc(size + 1);
    if (!buffer) {
        fclose(f);
        set_error(cfg, ERR_CFG_OUT_OF_MEMORY, "Out of memory", 0);
        return ERR_CFG_OUT_OF_MEMORY;
    }
    
    size_t read = fread(buffer, 1, size, f);
    buffer[read] = '\0';
    fclose(f);
    
    int result = cfg_load_string(cfg, buffer);
    free(buffer);
    
    return result;
}

int cfg_get_int(ConfigLang* cfg, const char* name, int* out) {
    if (!cfg || !name || !out) return ERR_CFG_NULL_POINTER;
    
    Variable* var = find_variable(cfg, name);
    if (!var) {
        set_error(cfg, ERR_CFG_VARIABLE_NOT_FOUND, "Variable not found", 0);
        return ERR_CFG_VARIABLE_NOT_FOUND;
    }
    
    if (var->type != VAR_TYPE_INT) {
        set_error(cfg, ERR_CFG_TYPE_MISMATCH, "Variable is not an integer", 0);
        return ERR_CFG_TYPE_MISMATCH;
    }
    
    *out = var->value.int_val;
    return ERR_CFG_OK;
}

int cfg_get_string(ConfigLang* cfg, const char* name, const char** out) {
    if (!cfg || !name || !out) return ERR_CFG_NULL_POINTER;
    
    Variable* var = find_variable(cfg, name);
    if (!var) {
        set_error(cfg, ERR_CFG_VARIABLE_NOT_FOUND, "Variable not found", 0);
        return ERR_CFG_VARIABLE_NOT_FOUND;
    }
    
    if (var->type != VAR_TYPE_STRING) {
        set_error(cfg, ERR_CFG_TYPE_MISMATCH, "Variable is not a string", 0);
        return ERR_CFG_TYPE_MISMATCH;
    }
    
    *out = var->value.str_val;
    return ERR_CFG_OK;
}

int cfg_set_int(ConfigLang* cfg, const char* name, int value) {
    if (!cfg || !name) return ERR_CFG_NULL_POINTER;
    
    Variable* var = find_variable(cfg, name);
    if (!var) {
        set_error(cfg, ERR_CFG_VARIABLE_NOT_FOUND, "Variable not found", 0);
        return ERR_CFG_VARIABLE_NOT_FOUND;
    }
    
    if (var->is_const) {
        set_error(cfg, ERR_CFG_CONST_VIOLATION, "Cannot modify const variable", 0);
        return ERR_CFG_CONST_VIOLATION;
    }
    
    if (var->type != VAR_TYPE_INT) {
        set_error(cfg, ERR_CFG_TYPE_MISMATCH, "Variable is not an integer", 0);
        return ERR_CFG_TYPE_MISMATCH;
    }
    
    var->value.int_val = value;
    return ERR_CFG_OK;
}

int cfg_save_file(ConfigLang* cfg, const char* path) {
    if (!cfg || !path) return ERR_CFG_NULL_POINTER;
    
    FILE* f = fopen(path, "w");
    if (!f) {
        set_error(cfg, ERR_CFG_FILE_ERROR, "Cannot open file for writing", 0);
        return ERR_CFG_FILE_ERROR;
    }
    
    for (int i = 0; i < MAX_VARIABLES; i++) {
        if (cfg->variables[i].in_use) {
            Variable* var = &cfg->variables[i];
            
            if (var->is_const) {
                fprintf(f, "const ");
            }
            
            fprintf(f, "set %s = ", var->name);
            
            if (var->type == VAR_TYPE_INT) {
                fprintf(f, "%d\n", var->value.int_val);
            } else {
                /* Check if string contains newlines */
                if (strchr(var->value.str_val, '\n')) {
                    fprintf(f, "#%%%%%%\n%s\n%%%%%%#\n", var->value.str_val);
                } else {
                    fprintf(f, "\"%s\"\n", var->value.str_val);
                }
            }
        }
    }
    
    fclose(f);
    return ERR_CFG_OK;
}

const char* cfg_get_error(ConfigLang* cfg) {
    if (!cfg) return "NULL pointer";
    return cfg->error_msg;
}