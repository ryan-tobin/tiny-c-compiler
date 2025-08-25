// src/main.c (updated for parser testing)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "semantic.h"

void print_usage(const char* program_name) {
    printf("Usage: %s [options] <input_file>\n", program_name);
    printf("Options:\n");
    printf("  --debug-tokens  Print token stream\n");
    printf("  --debug-ast     Print AST\n");
    printf("  --debug-symbols Print symbol table\n");
    printf("  -h, --help      Show this help\n");
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    char* input_file = NULL;
    int debug_tokens = 0;
    int debug_ast = 0;
    int debug_symbols = 0;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--debug-tokens") == 0) {
            debug_tokens = 1;
        } else if (strcmp(argv[i], "--debug-ast") == 0) {
            debug_ast = 1;
        } else if (strcmp(argv[i], "--debug-symbols") == 0) {
            debug_symbols = 1;
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
    
    printf("TinyC Compiler - Full Frontend Analysis\n");
    printf("Processing file: %s\n\n", input_file);
    
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
        printf("✓ Semantic analysis completed successfully!\n");
    }
    
    if (debug_symbols) {
        printf("\n=== SYMBOL TABLE DEBUG ===\n");
        printf("(Symbol table debugging not yet implemented)\n");
        printf("==========================\n");
    }
    
    // Cleanup
    semantic_destroy(analyzer);
    ast_destroy(ast);
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    if (semantic_success) {
        printf("\n✓ Frontend analysis completed successfully!\n");
        printf("  Ready for code generation phase.\n");
        return 0;
    } else {
        printf("\n✗ Frontend analysis failed.\n");
        return 1;
    }
}