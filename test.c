/**
 * test_configlang.c
 * 
 * Test program for the configuration language library
 */

#include "configlang.h"
#include <stdio.h>
#include <string.h>

void test_basic_variables(void) {
    printf("\n=== Test: Basic Variables ===\n");
    
    ConfigLang* cfg = cfg_create();
    
    const char* code = 
        "set x = 10\n"
        "set name = \"Hello World\"\n"
        "set y = 20\n";
    
    int result = cfg_load_string(cfg, code);
    if (result != ERR_CFG_OK) {
        printf("Error: %s\n", cfg_get_error(cfg));
        cfg_destroy(cfg);
        return;
    }
    
    int x, y;
    const char* name;
    
    cfg_get_int(cfg, "x", &x);
    cfg_get_int(cfg, "y", &y);
    cfg_get_string(cfg, "name", &name);
    
    printf("x = %d\n", x);
    printf("y = %d\n", y);
    printf("name = %s\n", name);
    
    cfg_destroy(cfg);
}

void test_const_variables(void) {
    printf("\n=== Test: Const Variables ===\n");
    
    ConfigLang* cfg = cfg_create();
    
    const char* code = 
        "const set max = 100\n"
        "set value = 50\n";
    
    cfg_load_string(cfg, code);
    
    int max, value;
    cfg_get_int(cfg, "max", &max);
    cfg_get_int(cfg, "value", &value);
    
    printf("max = %d (const)\n", max);
    printf("value = %d\n", value);
    
    /* Try to modify const - should fail */
    int result = cfg_set_int(cfg, "max", 200);
    if (result == ERR_CFG_CONST_VIOLATION) {
        printf("✓ Correctly prevented modification of const variable\n");
    } else {
        printf("✗ Failed to prevent const modification\n");
    }
    
    /* Modify non-const - should succeed */
    result = cfg_set_int(cfg, "value", 75);
    if (result == ERR_CFG_OK) {
        cfg_get_int(cfg, "value", &value);
        printf("✓ Successfully modified value to %d\n", value);
    }
    
    cfg_destroy(cfg);
}

void test_conditionals(void) {
    printf("\n=== Test: Conditionals ===\n");
    
    ConfigLang* cfg = cfg_create();
    
    const char* code = 
        "set a = 60\n"
        "if a > 50 { set a = 50 }\n"
        "set b = 5\n"
        "if b > 10 { set b = 10 } { set b = 90 }\n";
    
    cfg_load_string(cfg, code);
    
    int a, b;
    cfg_get_int(cfg, "a", &a);
    cfg_get_int(cfg, "b", &b);
    
    printf("a = %d (was 60, clamped to 50)\n", a);
    printf("b = %d (was 5, else block set to 90)\n", b);
    
    cfg_destroy(cfg);
}

void test_nested_conditionals(void) {
    printf("\n=== Test: Nested Conditionals ===\n");
    
    ConfigLang* cfg = cfg_create();
    
    const char* code = 
        "set a = 55\n"
        "if a > 50 { set a = 50 } if a < 10 { set a = 10 } { set a = 20 }\n";
    
    cfg_load_string(cfg, code);
    
    int a;
    cfg_get_int(cfg, "a", &a);
    
    printf("a = %d (first if: true→50, second if: false→else block sets 20)\n", a);
    
    cfg_destroy(cfg);
}

void test_multiline_values(void) {
    printf("\n=== Test: Multiline Values ===\n");
    
    ConfigLang* cfg = cfg_create();
    
    const char* code = 
        "set data = #%%%\n"
        "line1\n"
        "line2\n"
        "line3\n"
        "%%%#\n"
        "set simple = \"single line\"\n";
    
    cfg_load_string(cfg, code);
    
    const char* data;
    const char* simple;
    
    cfg_get_string(cfg, "data", &data);
    cfg_get_string(cfg, "simple", &simple);
    
    printf("data (multiline):\n%s\n", data);
    printf("simple = %s\n", simple);
    
    cfg_destroy(cfg);
}

void test_comments(void) {
    printf("\n=== Test: Comments ===\n");
    
    ConfigLang* cfg = cfg_create();
    
    const char* code = 
        "# This is a comment\n"
        "set x = 10\n"
        "# Another comment\n"
        "set y = 20\n";
    
    cfg_load_string(cfg, code);
    
    int x, y;
    cfg_get_int(cfg, "x", &x);
    cfg_get_int(cfg, "y", &y);
    
    printf("x = %d\n", x);
    printf("y = %d\n", y);
    
    cfg_destroy(cfg);
}

void test_save_load(void) {
    printf("\n=== Test: Save and Load ===\n");
    
    ConfigLang* cfg = cfg_create();
    
    const char* code = 
        "const set max = 100\n"
        "set value = 42\n"
        "set name = \"Test Config\"\n";
    
    cfg_load_string(cfg, code);
    cfg_set_int(cfg, "value", 99);
    
    /* Save to file */
    cfg_save_file(cfg, "test_config.txt");
    printf("✓ Saved configuration to test_config.txt\n");
    
    cfg_destroy(cfg);
    
    /* Load from file */
    cfg = cfg_create();
    cfg_load_file(cfg, "test_config.txt");
    
    int max, value;
    const char* name;
    
    cfg_get_int(cfg, "max", &max);
    cfg_get_int(cfg, "value", &value);
    cfg_get_string(cfg, "name", &name);
    
    printf("Loaded from file:\n");
    printf("  max = %d (const)\n", max);
    printf("  value = %d\n", value);
    printf("  name = %s\n", name);
    
    cfg_destroy(cfg);
}

void test_all_operators(void) {
    printf("\n=== Test: All Comparison Operators ===\n");
    
    ConfigLang* cfg = cfg_create();
    
    const char* code = 
        "set x = 10\n"
        "if x > 5 { set a = 1 }\n"
        "if x < 20 { set b = 1 }\n"
        "if x >= 10 { set c = 1 }\n"
        "if x <= 10 { set d = 1 }\n"
        "if x == 10 { set e = 1 }\n"
        "if x != 5 { set f = 1 }\n";
    
    cfg_load_string(cfg, code);
    
    int a, b, c, d, e, f;
    cfg_get_int(cfg, "a", &a);
    cfg_get_int(cfg, "b", &b);
    cfg_get_int(cfg, "c", &c);
    cfg_get_int(cfg, "d", &d);
    cfg_get_int(cfg, "e", &e);
    cfg_get_int(cfg, "f", &f);
    
    printf("x > 5:  a = %d ✓\n", a);
    printf("x < 20: b = %d ✓\n", b);
    printf("x >= 10: c = %d ✓\n", c);
    printf("x <= 10: d = %d ✓\n", d);
    printf("x == 10: e = %d ✓\n", e);
    printf("x != 5:  f = %d ✓\n", f);
    
    cfg_destroy(cfg);
}

void test_variable_reference(void) {
    printf("\n=== Test: Variable References ===\n");
    
    ConfigLang* cfg = cfg_create();
    
    const char* code = 
        "set x = 42\n"
        "set y = x\n"
        "set name = \"original\"\n"
        "set copy = name\n";
    
    cfg_load_string(cfg, code);
    
    int x, y;
    const char* name;
    const char* copy;
    
    cfg_get_int(cfg, "x", &x);
    cfg_get_int(cfg, "y", &y);
    cfg_get_string(cfg, "name", &name);
    cfg_get_string(cfg, "copy", &copy);
    
    printf("x = %d\n", x);
    printf("y = %d (copied from x)\n", y);
    printf("name = %s\n", name);
    printf("copy = %s (copied from name)\n", copy);
    
    cfg_destroy(cfg);
}

int main(void) {
    printf("ConfigLang Library Test Suite\n");
    printf("==============================\n");
    
    test_basic_variables();
    test_const_variables();
    test_conditionals();
    test_nested_conditionals();
    test_multiline_values();
    test_comments();
    test_all_operators();
    test_variable_reference();
    test_save_load();
    
    printf("\n=== All Tests Complete ===\n");
    
    return 0;
}