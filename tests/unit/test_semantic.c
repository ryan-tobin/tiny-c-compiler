// tests/unit/test_semantic.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../../src/lexer.h"
#include "../../src/parser.h"
#include "../../src/ast.h"
#include "../../src/semantic.h"

// Test helper functions
int analyze_string(const char* source) {
    lexer_t* lexer = lexer_create(source);
    if (!lexer) return 0;
    
    parser_t* parser = parser_create(lexer);
    if (!parser) {
        lexer_destroy(lexer);
        return 0;
    }
    
    ast_node_t* ast = parser_parse_program(parser);
    if (!ast || parser_has_errors(parser)) {
        if (ast) ast_destroy(ast);
        parser_destroy(parser);
        lexer_destroy(lexer);
        return 0;
    }
    
    semantic_analyzer_t* analyzer = semantic_create();
    if (!analyzer) {
        ast_destroy(ast);
        parser_destroy(parser);
        lexer_destroy(lexer);
        return 0;
    }
    
    int result = semantic_analyze(analyzer, ast);
    
    if (semantic_has_errors(analyzer)) {
        printf("Semantic errors:\n");
        semantic_print_errors(analyzer);
        result = 0;
    }
    
    semantic_destroy(analyzer);
    ast_destroy(ast);
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    return result;
}

void test_simple_function() {
    printf("Testing simple function semantic analysis...\n");
    
    const char* source = 
        "int main() {\n"
        "    return 42;\n"
        "}";
    
    assert(analyze_string(source) == 1);
    printf("✓ Simple function semantic test passed!\n\n");
}

void test_function_with_parameters() {
    printf("Testing function with parameters...\n");
    
    const char* source = 
        "int add(int a, int b) {\n"
        "    return a + b;\n"
        "}";
    
    assert(analyze_string(source) == 1);
    printf("✓ Function with parameters semantic test passed!\n\n");
}

void test_variable_declaration() {
    printf("Testing variable declarations...\n");
    
    const char* source = 
        "int main() {\n"
        "    int x = 10;\n"
        "    int y;\n"
        "    y = x + 5;\n"
        "    return y;\n"
        "}";
    
    assert(analyze_string(source) == 1);
    printf("✓ Variable declaration semantic test passed!\n\n");
}

void test_function_call() {
    printf("Testing function calls...\n");
    
    const char* source = 
        "int add(int a, int b);\n"
        "int main() {\n"
        "    int result = add(1, 2);\n"
        "    return result;\n"
        "}";
    
    assert(analyze_string(source) == 1);
    printf("✓ Function call semantic test passed!\n\n");
}

void test_type_checking() {
    printf("Testing type checking...\n");
    
    const char* source = 
        "int main() {\n"
        "    int x = 10;\n"
        "    int y = 20;\n"
        "    int result = x + y * 2;\n"
        "    return result;\n"
        "}";
    
    assert(analyze_string(source) == 1);
    printf("✓ Type checking semantic test passed!\n\n");
}

void test_scope_management() {
    printf("Testing scope management...\n");
    
    const char* source = 
        "int main() {\n"
        "    int x = 10;\n"
        "    {\n"
        "        int y = 20;\n"
        "        x = x + y;\n"
        "    }\n"
        "    return x;\n"
        "}";
    
    assert(analyze_string(source) == 1);
    printf("✓ Scope management semantic test passed!\n\n");
}

void test_undeclared_variable_error() {
    printf("Testing undeclared variable error...\n");
    
    const char* source = 
        "int main() {\n"
        "    return undeclared_var;\n"
        "}";
    
    // This should fail semantic analysis
    assert(analyze_string(source) == 0);
    printf("✓ Undeclared variable error test passed!\n\n");
}

void test_type_mismatch_error() {
    printf("Testing type mismatch error...\n");
    
    const char* source = 
        "int main() {\n"
        "    int x;\n"
        "    x = \"hello\";\n"  // Type mismatch: int = char*
        "    return x;\n"
        "}";
    
    // This should fail semantic analysis
    assert(analyze_string(source) == 0);
    printf("✓ Type mismatch error test passed!\n\n");
}

void test_function_redeclaration_error() {
    printf("Testing function redeclaration error...\n");
    
    const char* source = 
        "int main() { return 0; }\n"
        "int main() { return 1; }\n";  // Redeclaration
    
    // This should fail semantic analysis
    assert(analyze_string(source) == 0);
    printf("✓ Function redeclaration error test passed!\n\n");
}

void test_wrong_argument_count_error() {
    printf("Testing wrong argument count error...\n");
    
    const char* source = 
        "int add(int a, int b);\n"
        "int main() {\n"
        "    return add(1);\n"  // Wrong argument count
        "}";
    
    // This should fail semantic analysis
    assert(analyze_string(source) == 0);
    printf("✓ Wrong argument count error test passed!\n\n");
}

void test_return_type_mismatch_error() {
    printf("Testing return type mismatch error...\n");
    
    const char* source = 
        "int main() {\n"
        "    return \"hello\";\n"  // Return type mismatch
        "}";
    
    // This should fail semantic analysis
    assert(analyze_string(source) == 0);
    printf("✓ Return type mismatch error test passed!\n\n");
}

void test_void_function_return() {
    printf("Testing void function return...\n");
    
    const char* source = 
        "void print_hello() {\n"
        "    return;\n"
        "}\n"
        "int main() {\n"
        "    print_hello();\n"
        "    return 0;\n"
        "}";
    
    assert(analyze_string(source) == 1);
    printf("✓ Void function return test passed!\n\n");
}

int main() {
    printf("=== RUNNING SEMANTIC ANALYSIS TESTS ===\n\n");
    
    // Positive tests (should pass)
    test_simple_function();
    test_function_with_parameters();
    test_variable_declaration();
    test_function_call();
    test_type_checking();
    test_scope_management();
    test_void_function_return();
    
    // Negative tests (should fail)
    test_undeclared_variable_error();
    test_type_mismatch_error();
    test_function_redeclaration_error();
    test_wrong_argument_count_error();
    test_return_type_mismatch_error();
    
    printf("🎉 All semantic analysis tests passed!\n");
    return 0;
}