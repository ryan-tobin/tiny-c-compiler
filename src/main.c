// src/main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"

void print_usage(const char* program_name) {
    printf("Usage: %s [options] <input_file>\n", program_name);
    printf("Options:\n");
    printf("  --debug-tokens  Print token stream\n");
    printf("  -h, --help      Show this help\n");
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    char* input_file = NULL;
    int debug_tokens = 0;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--debug-tokens") == 0) {
            debug_tokens = 1;
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
    
    printf("TinyC Compiler - Lexer Phase\n");
    printf("Processing file: %s\n\n", input_file);
    
    // Create lexer from file
    lexer_t* lexer = lexer_create_from_file(input_file);
    if (!lexer) {
        fprintf(stderr, "Error: Could not read input file '%s'\n", input_file);
        return 1;
    }
    
    if (debug_tokens) {
        lexer_print_tokens(lexer);
    } else {
        // Just verify lexing works without printing
        token_t token;
        int token_count = 0;
        int error_count = 0;
        
        do {
            token = lexer_next_token(lexer);
            token_count++;
            
            if (token.type == TOKEN_ERROR) {
                printf("ERROR at line %d, column %d: %s\n", 
                       token.line, token.column, 
                       token.value ? token.value : "Unknown error");
                error_count++;
            }
            
        } while (token.type != TOKEN_EOF && token.type != TOKEN_ERROR);
        
        if (error_count == 0) {
            printf("✓ Lexical analysis completed successfully!\n");
            printf("  Total tokens: %d\n", token_count - 1); // -1 for EOF
        } else {
            printf("✗ Lexical analysis failed with %d error(s)\n", error_count);
        }
    }
    
    lexer_destroy(lexer);
    return 0;
}