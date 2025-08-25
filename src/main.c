#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "semantic.h"
#include "codegen.h"

void print_usage(const char* program_name) {
    printf("Usage: %s [options] <input_file>\n", program_name);
    printf("Options:\n");
    printf("  -o <file>         Output file (default: out.s)\n");
    printf("  --debug-tokens    Print token stream\n");
    printf("  --debug-ast       Print AST\n");
    printf("  --debug-symbols   Print symbol table\n");
    printf("  --compile-only    Generate assembly only (don't assemble)\n");
    printf("  -h, --help        Show this help\n");
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    char* input_file = NULL;
    char* output_file = "out.s";
    int debug_tokens = 0;
    int debug_ast = 0;
    int debug_symbols = 0;
    int compile_only = 0;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_file = argv[++i];
        } else if (strcmp(argv[i], "--debug-tokens") == 0) {
            debug_tokens = 1;
        } else if (strcmp(argv[i], "--debug-ast") == 0) {
            debug_ast = 1;
        } else if (strcmp(argv[i], "--debug-symbols") == 0) {
            debug_symbols = 1;
        } else if (strcmp(argv[i], "--compile-only") == 0) {
            compile_only = 1;
        } else if (argv[i][0] != '-') {
            input_file = argv[i];
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            return 1;
        }
    }
    
    if (!input_file) {
        fprintf(stderr, "Error: No input file specified\n");
        print_usage(argv[0]);
        return 1;
    }
    
    printf("TinyC Compiler - Complete Pipeline\n");
    printf("Processing file: %s\n", input_file);
    if (compile_only) {
        printf("Output: %s\n\n", output_file);
    } else {
        printf("Assembly: %s\n\n", output_file);
    }
    
    // Phase 1: Lexical Analysis
    lexer_t* lexer = lexer_create_from_file(input_file);
    if (!lexer) {
        fprintf(stderr, "Error: Could not read input file '%s'\n", input_file);
        return 1;
    }
    
    if (debug_tokens) {
        printf("=== LEXICAL ANALYSIS ===\n");
        lexer_print_tokens(lexer);
        lexer_reset(lexer);
    }
    
    // Phase 2: Parsing
    printf("=== PARSING ===\n");
    parser_t* parser = parser_create(lexer);
    if (!parser) {
        fprintf(stderr, "Error: Could not create parser\n");
        lexer_destroy(lexer);
        return 1;
    }
    
    ast_node_t* ast = parser_parse_program(parser);
    
    if (parser_has_errors(parser)) {
        printf("✗ Parsing failed with errors:\n");
        parser_print_errors(parser);
        
        if (ast) ast_destroy(ast);
        parser_destroy(parser);
        lexer_destroy(lexer);
        return 1;
    }
    
    printf("✓ Parsing completed successfully!\n\n");
    
    if (debug_ast && ast) {
        printf("=== ABSTRACT SYNTAX TREE ===\n");
        ast_print(ast, 0);
        printf("============================\n\n");
    }
    
    // Phase 3: Semantic Analysis
    printf("=== SEMANTIC ANALYSIS ===\n");
    semantic_analyzer_t* analyzer = semantic_create();
    if (!analyzer) {
        fprintf(stderr, "Error: Could not create semantic analyzer\n");
        ast_destroy(ast);
        parser_destroy(parser);
        lexer_destroy(lexer);
        return 1;
    }
    
    int semantic_success = semantic_analyze(analyzer, ast);
    
    if (semantic_has_errors(analyzer)) {
        printf("✗ Semantic analysis failed with errors:\n");
        semantic_print_errors(analyzer);
        semantic_success = 0;
    } else {
        printf("✓ Semantic analysis completed successfully!\n\n");
    }
    
    if (!semantic_success) {
        semantic_destroy(analyzer);
        ast_destroy(ast);
        parser_destroy(parser);
        lexer_destroy(lexer);
        return 1;
    }
    
    if (debug_symbols) {
        printf("=== SYMBOL TABLE DEBUG ===\n");
        printf("(Symbol table debugging not yet implemented)\n");
        printf("==========================\n\n");
    }
    
    // Phase 4: Code Generation
    printf("=== CODE GENERATION ===\n");
    codegen_t* codegen = codegen_create(output_file);
    if (!codegen) {
        fprintf(stderr, "Error: Could not create code generator\n");
        semantic_destroy(analyzer);
        ast_destroy(ast);
        parser_destroy(parser);
        lexer_destroy(lexer);
        return 1;
    }
    
    int codegen_success = codegen_generate(codegen, ast);
    
    if (codegen_success) {
        printf("✓ Code generation completed successfully!\n");
        printf("  Assembly written to: %s\n", output_file);
    } else {
        printf("✗ Code generation failed!\n");
    }
    
    // Cleanup
    codegen_destroy(codegen);
    semantic_destroy(analyzer);
    ast_destroy(ast);
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    if (!codegen_success) {
        return 1;
    }
    
    // Phase 5: Assembly and Linking (optional)
    if (!compile_only) {
        printf("\n=== ASSEMBLY & LINKING ===\n");
        
        // Determine executable name
        char* exe_name = malloc(strlen(input_file) + 10);
        strcpy(exe_name, input_file);
        char* dot = strrchr(exe_name, '.');
        if (dot) *dot = '\0';
        
        // Assembly and link command
        char link_cmd[512];
        snprintf(link_cmd, sizeof(link_cmd), 
                "gcc -m64 -no-pie %s runtime/runtime.c -o %s",
                output_file, exe_name);
        
        printf("Running: %s\n", link_cmd);
        int link_result = system(link_cmd);
        
        if (link_result == 0) {
            printf("✓ Assembly and linking completed successfully!\n");
            printf("  Executable created: %s\n", exe_name);
            printf("\nRun your program with: ./%s\n", exe_name);
        } else {
            printf("✗ Assembly and linking failed!\n");
            printf("  You can still use the assembly file: %s\n", output_file);
        }
        
        free(exe_name);
    }
    
    printf("\n✓ Compilation completed successfully!\n");
    return 0;
}