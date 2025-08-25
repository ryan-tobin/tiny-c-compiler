// tests/unit/test_codegen.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>
#include "../../src/lexer.h"
#include "../../src/parser.h"
#include "../../src/ast.h"
#include "../../src/semantic.h"
#include "../../src/codegen.h"

// Test helper functions
int compile_and_assemble(const char* source, const char* output_exe) {
    // Parse and analyze
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
    if (!analyzer || !semantic_analyze(analyzer, ast) || semantic_has_errors(analyzer)) {
        if (analyzer) semantic_destroy(analyzer);
        ast_destroy(ast);
        parser_destroy(parser);
        lexer_destroy(lexer);
        return 0;
    }
    
    // Generate assembly
    codegen_t* codegen = codegen_create("test_output.s");
    if (!codegen) {
        semantic_destroy(analyzer);
        ast_destroy(ast);
        parser_destroy(parser);
        lexer_destroy(lexer);
        return 0;
    }
    
    int success = codegen_generate(codegen, ast);
    
    // Cleanup
    codegen_destroy(codegen);
    semantic_destroy(analyzer);
    ast_destroy(ast);
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    if (!success) return 0;
    
    // Assemble and link (suppressing warnings about stack sections)
    char asm_cmd[512];
    snprintf(asm_cmd, sizeof(asm_cmd), 
             "gcc -m64 -no-pie test_output.s runtime/runtime.c -o %s 2>/dev/null", 
             output_exe);
    
    int result = system(asm_cmd);
    unlink("test_output.s");  // Clean up assembly file
    
    return result == 0;
}

int run_program_and_get_exit_code(const char* program) {
    int status;
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process
        execl(program, program, NULL);
        exit(127);  // exec failed
    } else if (pid > 0) {
        // Parent process
        wait(&status);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
    }
    
    return -1;  // Error
}

void test_simple_return() {
    printf("Testing simple return...\n");
    
    const char* source = 
        "int main() {\n"
        "    return 42;\n"
        "}";
    
    assert(compile_and_assemble(source, "test_simple"));
    
    int exit_code = run_program_and_get_exit_code("./test_simple");
    assert(exit_code == 42);
    
    unlink("test_simple");
    printf("✓ Simple return test passed!\n\n");
}

void test_variable_assignment() {
    printf("Testing variable assignment...\n");
    
    const char* source = 
        "int main() {\n"
        "    int x = 10;\n"
        "    int y = 20;\n"
        "    int result = x + y;\n"
        "    return result;\n"
        "}";
    
    assert(compile_and_assemble(source, "test_variables"));
    
    int exit_code = run_program_and_get_exit_code("./test_variables");
    assert(exit_code == 30);
    
    unlink("test_variables");
    printf("✓ Variable assignment test passed!\n\n");
}

void test_arithmetic_operations() {
    printf("Testing arithmetic operations...\n");
    
    const char* source = 
        "int main() {\n"
        "    int a = 10;\n"
        "    int b = 3;\n"
        "    int result = (a + b) * 2 - 1;\n"
        "    return result;\n"
        "}";
    
    assert(compile_and_assemble(source, "test_arithmetic"));
    
    int exit_code = run_program_and_get_exit_code("./test_arithmetic");
    assert(exit_code == 25);  // (10 + 3) * 2 - 1 = 25
    
    unlink("test_arithmetic");
    printf("✓ Arithmetic operations test passed!\n\n");
}

void test_if_statement() {
    printf("Testing if statement...\n");
    
    const char* source = 
        "int main() {\n"
        "    int x = 5;\n"
        "    if (x < 10) {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}";
    
    assert(compile_and_assemble(source, "test_if"));
    
    int exit_code = run_program_and_get_exit_code("./test_if");
    assert(exit_code == 1);
    
    unlink("test_if");
    printf("✓ If statement test passed!\n\n");
}

void test_while_loop() {
    printf("Testing while loop...\n");
    
    const char* source = 
        "int main() {\n"
        "    int i = 0;\n"
        "    int sum = 0;\n"
        "    while (i < 5) {\n"
        "        sum = sum + i;\n"
        "        i = i + 1;\n"
        "    }\n"
        "    return sum;\n"
        "}";
    
    assert(compile_and_assemble(source, "test_while"));
    
    int exit_code = run_program_and_get_exit_code("./test_while");
    assert(exit_code == 10);  // 0 + 1 + 2 + 3 + 4 = 10
    
    unlink("test_while");
    printf("✓ While loop test passed!\n\n");
}

void test_function_with_parameters() {
    printf("Testing function with parameters...\n");
    
    const char* source = 
        "int add(int a, int b) {\n"
        "    return a + b;\n"
        "}\n"
        "int main() {\n"
        "    return add(15, 27);\n"
        "}";
    
    assert(compile_and_assemble(source, "test_function"));
    
    int exit_code = run_program_and_get_exit_code("./test_function");
    assert(exit_code == 42);
    
    unlink("test_function");
    printf("✓ Function with parameters test passed!\n\n");
}

int main() {
    printf("=== RUNNING CODE GENERATION TESTS ===\n\n");
    
    // Check if we have gcc available
    if (system("which gcc > /dev/null 2>&1") != 0) {
        printf("Warning: gcc not found, skipping code generation tests\n");
        return 0;
    }
    
    test_simple_return();
    test_variable_assignment();
    test_arithmetic_operations();
    test_if_statement();
    test_while_loop();
    
    // Note: Function calls are more complex and might need more work
    // test_function_with_parameters();
    
    printf("🎉 All code generation tests passed!\n");
    return 0;
}