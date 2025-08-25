// src/lexer.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

// Keywords lookup table
static const struct {
    const char* word;
    token_type_t token;
} keywords[] = {
    {"int", TOKEN_INT},
    {"char", TOKEN_CHAR},
    {"void", TOKEN_VOID},
    {"if", TOKEN_IF},
    {"else", TOKEN_ELSE},
    {"while", TOKEN_WHILE},
    {"for", TOKEN_FOR},
    {"return", TOKEN_RETURN},
    {NULL, TOKEN_EOF}
};

// Token type to string mapping for debugging
const char* token_type_to_string(token_type_t type) {
    switch (type) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_STRING: return "STRING";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_INT: return "int";
        case TOKEN_CHAR: return "char";
        case TOKEN_VOID: return "void";
        case TOKEN_IF: return "if";
        case TOKEN_ELSE: return "else";
        case TOKEN_WHILE: return "while";
        case TOKEN_FOR: return "for";
        case TOKEN_RETURN: return "return";
        case TOKEN_PLUS: return "+";
        case TOKEN_MINUS: return "-";
        case TOKEN_MULTIPLY: return "*";
        case TOKEN_DIVIDE: return "/";
        case TOKEN_MODULO: return "%";
        case TOKEN_ASSIGN: return "=";
        case TOKEN_EQUAL: return "==";
        case TOKEN_NOT_EQUAL: return "!=";
        case TOKEN_LESS: return "<";
        case TOKEN_LESS_EQUAL: return "<=";
        case TOKEN_GREATER: return ">";
        case TOKEN_GREATER_EQUAL: return ">=";
        case TOKEN_LOGICAL_AND: return "&&";
        case TOKEN_LOGICAL_OR: return "||";
        case TOKEN_LOGICAL_NOT: return "!";
        case TOKEN_SEMICOLON: return ";";
        case TOKEN_COMMA: return ",";
        case TOKEN_LEFT_PAREN: return "(";
        case TOKEN_RIGHT_PAREN: return ")";
        case TOKEN_LEFT_BRACE: return "{";
        case TOKEN_RIGHT_BRACE: return "}";
        case TOKEN_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

// Create lexer from source string
lexer_t* lexer_create(const char* source) {
    if (!source) return NULL;
    
    lexer_t* lexer = malloc(sizeof(lexer_t));
    if (!lexer) return NULL;
    
    lexer->length = strlen(source);
    lexer->source = malloc(lexer->length + 1);
    if (!lexer->source) {
        free(lexer);
        return NULL;
    }
    
    strcpy(lexer->source, source);
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->current_token.type = TOKEN_EOF;
    lexer->current_token.value = NULL;
    lexer->current_token.line = 0;
    lexer->current_token.column = 0;
    
    return lexer;
}

// Create lexer from file
lexer_t* lexer_create_from_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return NULL;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Read entire file
    char* source = malloc(file_size + 1);
    if (!source) {
        fclose(file);
        return NULL;
    }
    
    size_t bytes_read = fread(source, 1, file_size, file);
    source[bytes_read] = '\0';
    fclose(file);
    
    // Create lexer directly instead of calling lexer_create
    lexer_t* lexer = malloc(sizeof(lexer_t));
    if (!lexer) {
        free(source);
        return NULL;
    }
    
    lexer->length = bytes_read;
    lexer->source = source; // Take ownership of the source string
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->current_token.type = TOKEN_EOF;
    lexer->current_token.value = NULL;
    lexer->current_token.line = 0;
    lexer->current_token.column = 0;
    
    return lexer;
}

// Destroy lexer and free memory
void lexer_destroy(lexer_t* lexer) {
    if (!lexer) return;
    
    if (lexer->source) free(lexer->source);
    if (lexer->current_token.value) free(lexer->current_token.value);
    free(lexer);
}

// Helper: Check if we're at end of source
static int lexer_at_end(lexer_t* lexer) {
    return lexer->position >= lexer->length;
}

// Helper: Peek at current character without advancing
static char lexer_peek(lexer_t* lexer) {
    if (lexer_at_end(lexer)) return '\0';
    return lexer->source[lexer->position];
}

// Helper: Peek at next character
static char lexer_peek_next(lexer_t* lexer) {
    if (lexer->position + 1 >= lexer->length) return '\0';
    return lexer->source[lexer->position + 1];
}

// Helper: Advance position and return current character
static char lexer_advance(lexer_t* lexer) {
    if (lexer_at_end(lexer)) return '\0';
    
    char c = lexer->source[lexer->position++];
    
    if (c == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }
    
    return c;
}

// Helper: Skip whitespace and comments
static void lexer_skip_whitespace(lexer_t* lexer) {
    while (!lexer_at_end(lexer)) {
        char c = lexer_peek(lexer);
        
        if (isspace(c)) {
            lexer_advance(lexer);
        } else if (c == '/' && lexer_peek_next(lexer) == '/') {
            // Skip line comment
            while (!lexer_at_end(lexer) && lexer_peek(lexer) != '\n') {
                lexer_advance(lexer);
            }
        } else if (c == '/' && lexer_peek_next(lexer) == '*') {
            // Skip block comment
            lexer_advance(lexer); // consume '/'
            lexer_advance(lexer); // consume '*'
            
            while (!lexer_at_end(lexer)) {
                if (lexer_peek(lexer) == '*' && lexer_peek_next(lexer) == '/') {
                    lexer_advance(lexer); // consume '*'
                    lexer_advance(lexer); // consume '/'
                    break;
                }
                lexer_advance(lexer);
            }
        } else {
            break;
        }
    }
}

// Helper: Create a token
static token_t lexer_make_token(lexer_t* lexer, token_type_t type, const char* value) {
    token_t token;
    token.type = type;
    token.line = lexer->line;
    token.column = lexer->column;
    
    if (value) {
        token.value = malloc(strlen(value) + 1);
        strcpy(token.value, value);
    } else {
        token.value = NULL;
    }
    
    return token;
}

// Helper: Check if string is a keyword
static token_type_t lexer_check_keyword(const char* text) {
    for (int i = 0; keywords[i].word != NULL; i++) {
        if (strcmp(text, keywords[i].word) == 0) {
            return keywords[i].token;
        }
    }
    return TOKEN_IDENTIFIER;
}

// Helper: Scan identifier or keyword
static token_t lexer_scan_identifier(lexer_t* lexer) {
    int start_line = lexer->line;
    int start_column = lexer->column;
    
    size_t start = lexer->position;
    
    // First character must be letter or underscore
    if (isalpha(lexer_peek(lexer)) || lexer_peek(lexer) == '_') {
        lexer_advance(lexer);
        
        // Subsequent characters can be letters, digits, or underscores
        while (!lexer_at_end(lexer) && 
               (isalnum(lexer_peek(lexer)) || lexer_peek(lexer) == '_')) {
            lexer_advance(lexer);
        }
    }
    
    size_t length = lexer->position - start;
    char* text = malloc(length + 1);
    strncpy(text, &lexer->source[start], length);
    text[length] = '\0';
    
    token_type_t type = lexer_check_keyword(text);
    
    token_t token;
    token.type = type;
    token.value = text;
    token.line = start_line;
    token.column = start_column;
    
    return token;
}

// Helper: Scan number literal
static token_t lexer_scan_number(lexer_t* lexer) {
    int start_line = lexer->line;
    int start_column = lexer->column;
    
    size_t start = lexer->position;
    
    while (!lexer_at_end(lexer) && isdigit(lexer_peek(lexer))) {
        lexer_advance(lexer);
    }
    
    size_t length = lexer->position - start;
    char* text = malloc(length + 1);
    strncpy(text, &lexer->source[start], length);
    text[length] = '\0';
    
    token_t token;
    token.type = TOKEN_NUMBER;
    token.value = text;
    token.line = start_line;
    token.column = start_column;
    
    return token;
}

// Helper: Scan string literal
static token_t lexer_scan_string(lexer_t* lexer) {
    int start_line = lexer->line;
    int start_column = lexer->column;
    
    lexer_advance(lexer); // consume opening quote
    
    size_t start = lexer->position;
    
    while (!lexer_at_end(lexer) && lexer_peek(lexer) != '"') {
        if (lexer_peek(lexer) == '\\') {
            lexer_advance(lexer); // consume backslash
            if (!lexer_at_end(lexer)) {
                lexer_advance(lexer); // consume escaped character
            }
        } else {
            lexer_advance(lexer);
        }
    }
    
    if (lexer_at_end(lexer)) {
        // Unterminated string
        token_t token;
        token.type = TOKEN_ERROR;
        token.value = malloc(20);
        strcpy(token.value, "Unterminated string");
        token.line = start_line;
        token.column = start_column;
        return token;
    }
    
    size_t length = lexer->position - start;
    char* text = malloc(length + 1);
    strncpy(text, &lexer->source[start], length);
    text[length] = '\0';
    
    lexer_advance(lexer); // consume closing quote
    
    token_t token;
    token.type = TOKEN_STRING;
    token.value = text;
    token.line = start_line;
    token.column = start_column;
    
    return token;
}

// Main tokenization function
token_t lexer_next_token(lexer_t* lexer) {
    lexer_skip_whitespace(lexer);
    
    if (lexer_at_end(lexer)) {
        lexer->current_token = lexer_make_token(lexer, TOKEN_EOF, NULL);
        return lexer->current_token;
    }
    
    char c = lexer_advance(lexer);
    
    // Single character tokens
    switch (c) {
        case '+': 
            lexer->current_token = lexer_make_token(lexer, TOKEN_PLUS, NULL);
            break;
        case '-': 
            lexer->current_token = lexer_make_token(lexer, TOKEN_MINUS, NULL);
            break;
        case '*': 
            lexer->current_token = lexer_make_token(lexer, TOKEN_MULTIPLY, NULL);
            break;
        case '/': 
            lexer->current_token = lexer_make_token(lexer, TOKEN_DIVIDE, NULL);
            break;
        case '%': 
            lexer->current_token = lexer_make_token(lexer, TOKEN_MODULO, NULL);
            break;
        case ';': 
            lexer->current_token = lexer_make_token(lexer, TOKEN_SEMICOLON, NULL);
            break;
        case ',': 
            lexer->current_token = lexer_make_token(lexer, TOKEN_COMMA, NULL);
            break;
        case '(': 
            lexer->current_token = lexer_make_token(lexer, TOKEN_LEFT_PAREN, NULL);
            break;
        case ')': 
            lexer->current_token = lexer_make_token(lexer, TOKEN_RIGHT_PAREN, NULL);
            break;
        case '{': 
            lexer->current_token = lexer_make_token(lexer, TOKEN_LEFT_BRACE, NULL);
            break;
        case '}': 
            lexer->current_token = lexer_make_token(lexer, TOKEN_RIGHT_BRACE, NULL);
            break;
            
        // Multi-character tokens
        case '=':
            if (lexer_peek(lexer) == '=') {
                lexer_advance(lexer);
                lexer->current_token = lexer_make_token(lexer, TOKEN_EQUAL, NULL);
            } else {
                lexer->current_token = lexer_make_token(lexer, TOKEN_ASSIGN, NULL);
            }
            break;
            
        case '!':
            if (lexer_peek(lexer) == '=') {
                lexer_advance(lexer);
                lexer->current_token = lexer_make_token(lexer, TOKEN_NOT_EQUAL, NULL);
            } else {
                lexer->current_token = lexer_make_token(lexer, TOKEN_LOGICAL_NOT, NULL);
            }
            break;
            
        case '<':
            if (lexer_peek(lexer) == '=') {
                lexer_advance(lexer);
                lexer->current_token = lexer_make_token(lexer, TOKEN_LESS_EQUAL, NULL);
            } else {
                lexer->current_token = lexer_make_token(lexer, TOKEN_LESS, NULL);
            }
            break;
            
        case '>':
            if (lexer_peek(lexer) == '=') {
                lexer_advance(lexer);
                lexer->current_token = lexer_make_token(lexer, TOKEN_GREATER_EQUAL, NULL);
            } else {
                lexer->current_token = lexer_make_token(lexer, TOKEN_GREATER, NULL);
            }
            break;
            
        case '&':
            if (lexer_peek(lexer) == '&') {
                lexer_advance(lexer);
                lexer->current_token = lexer_make_token(lexer, TOKEN_LOGICAL_AND, NULL);
            } else {
                lexer->current_token = lexer_make_token(lexer, TOKEN_ERROR, "Unexpected character");
            }
            break;
            
        case '|':
            if (lexer_peek(lexer) == '|') {
                lexer_advance(lexer);
                lexer->current_token = lexer_make_token(lexer, TOKEN_LOGICAL_OR, NULL);
            } else {
                lexer->current_token = lexer_make_token(lexer, TOKEN_ERROR, "Unexpected character");
            }
            break;
            
        case '"':
            // Put back the quote for string scanning
            lexer->position--;
            lexer->column--;
            lexer->current_token = lexer_scan_string(lexer);
            break;
            
        default:
            if (isalpha(c) || c == '_') {
                // Put back the character for identifier scanning
                lexer->position--;
                lexer->column--;
                lexer->current_token = lexer_scan_identifier(lexer);
            } else if (isdigit(c)) {
                // Put back the character for number scanning
                lexer->position--;
                lexer->column--;
                lexer->current_token = lexer_scan_number(lexer);
            } else {
                char error_msg[50];
                snprintf(error_msg, sizeof(error_msg), "Unexpected character: '%c'", c);
                lexer->current_token = lexer_make_token(lexer, TOKEN_ERROR, error_msg);
            }
            break;
    }
    
    return lexer->current_token;
}

// Peek at next token without consuming it
token_t lexer_peek_token(lexer_t* lexer) {
    // Save current state
    size_t saved_position = lexer->position;
    int saved_line = lexer->line;
    int saved_column = lexer->column;
    token_t saved_token = lexer->current_token;
    
    // Get next token
    token_t next_token = lexer_next_token(lexer);
    
    // Restore state
    lexer->position = saved_position;
    lexer->line = saved_line;
    lexer->column = saved_column;
    lexer->current_token = saved_token;
    
    return next_token;
}

// Reset lexer to beginning
void lexer_reset(lexer_t* lexer) {
    if (!lexer) return;
    
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    
    if (lexer->current_token.value) {
        free(lexer->current_token.value);
        lexer->current_token.value = NULL;
    }
    lexer->current_token.type = TOKEN_EOF;
}

// Print all tokens (for debugging)
void lexer_print_tokens(lexer_t* lexer) {
    if (!lexer) return;
    
    printf("=== TOKEN STREAM ===\n");
    token_t token;
    
    do {
        token = lexer_next_token(lexer);
        printf("%-15s", token_type_to_string(token.type));
        
        if (token.value) {
            printf(" '%s'", token.value);
        }
        
        printf(" [%d:%d]\n", token.line, token.column);
        
    } while (token.type != TOKEN_EOF && token.type != TOKEN_ERROR);
    
    printf("===================\n\n");
}