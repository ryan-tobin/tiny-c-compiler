// src/lexer.h
#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stddef.h>

// Token types enumeration
typedef enum {
    TOKEN_EOF = 0,
    
    // Literals
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_IDENTIFIER,
    
    // Keywords
    TOKEN_INT,
    TOKEN_CHAR,
    TOKEN_VOID,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_RETURN,
    
    // Operators
    TOKEN_PLUS,            // +
    TOKEN_MINUS,           // -
    TOKEN_MULTIPLY,        // *
    TOKEN_DIVIDE,          // /
    TOKEN_MODULO,          // %
    TOKEN_ASSIGN,          // =
    TOKEN_EQUAL,           // ==
    TOKEN_NOT_EQUAL,       // !=
    TOKEN_LESS,            // <
    TOKEN_LESS_EQUAL,      // <=
    TOKEN_GREATER,         // >
    TOKEN_GREATER_EQUAL,   // >=
    TOKEN_LOGICAL_AND,     // &&
    TOKEN_LOGICAL_OR,      // ||
    TOKEN_LOGICAL_NOT,     // !
    
    // Punctuation
    TOKEN_SEMICOLON,       // ;
    TOKEN_COMMA,           // ,
    TOKEN_LEFT_PAREN,      // (
    TOKEN_RIGHT_PAREN,     // )
    TOKEN_LEFT_BRACE,      // {
    TOKEN_RIGHT_BRACE,     // }
    
    // Error token
    TOKEN_ERROR
} token_type_t;

// Token structure
typedef struct {
    token_type_t type;     // Type of the token
    char* value;           // String value (for identifiers, numbers, strings)
    int line;              // Line number where token appears
    int column;            // Column number where token starts
} token_t;

// Lexer state structure
typedef struct {
    char* source;          // Source code string
    size_t length;         // Length of source code
    size_t position;       // Current position in source
    int line;              // Current line number
    int column;            // Current column number
    token_t current_token; // Currently processed token
} lexer_t;

// Lexer lifecycle functions
/**
 * @brief Creates a new lexer instance from source code string
 * 
 * @param source The source code to tokenize (null-terminated string)
 * @return lexer_t* Pointer to new lexer instance, or NULL on failure
 * 
 * @note The caller is responsible for freeing the lexer with lexer_destroy()
 * @see lexer_destroy(), lexer_create_from_file()
 */
lexer_t* lexer_create(const char* source);

/**
 * @brief Creates a new lexer instance from a source file
 * 
 * @param filename Path to the source file to read and tokenize
 * @return lexer_t* Pointer to new lexer instance, or NULL on failure
 * 
 * @note The caller is responsible for freeing the lexer with lexer_destroy()
 * @see lexer_destroy(), lexer_create()
 */
lexer_t* lexer_create_from_file(const char* filename);

/**
 * @brief Destroys a lexer instance and frees all associated memory
 * 
 * @param lexer The lexer instance to destroy
 * 
 * @note Safe to call with NULL pointer
 */
void lexer_destroy(lexer_t* lexer);

// Tokenization functions
/**
 * @brief Advances the lexer and returns the next token
 * 
 * @param lexer The lexer instance
 * @return token_t The next token in the stream
 * 
 * @warning The returned token's value pointer may be invalidated by
 *          subsequent calls to lexer_next_token()
 * @note Returns TOKEN_EOF when end of input is reached
 * @note Returns TOKEN_ERROR for invalid input
 */
token_t lexer_next_token(lexer_t* lexer);

/**
 * @brief Peeks at the next token without consuming it
 * 
 * @param lexer The lexer instance
 * @return token_t The next token that would be returned by lexer_next_token()
 * 
 * @note This function does not advance the lexer position
 * @note Multiple calls will return the same token
 */
token_t lexer_peek_token(lexer_t* lexer);

/**
 * @brief Resets the lexer to the beginning of the source
 * 
 * @param lexer The lexer instance to reset
 * 
 * @note After reset, the next call to lexer_next_token() will return the first token
 */
void lexer_reset(lexer_t* lexer);

// Utility functions
/**
 * @brief Prints all tokens in the source (for debugging)
 * 
 * @param lexer The lexer instance
 * 
 * @note This function will consume all tokens and reset the lexer afterward
 * @note Useful for debugging and development
 */
void lexer_print_tokens(lexer_t* lexer);

/**
 * @brief Converts a token type to its string representation
 * 
 * @param type The token type to convert
 * @return const char* String representation of the token type
 * 
 * @note Returns "UNKNOWN" for invalid token types
 * @note Used primarily for debugging and error messages
 */
const char* token_type_to_string(token_type_t type);

// Token helper macros
#define TOKEN_IS_KEYWORD(token) \
    ((token).type >= TOKEN_INT && (token).type <= TOKEN_RETURN)

#define TOKEN_IS_OPERATOR(token) \
    ((token).type >= TOKEN_PLUS && (token).type <= TOKEN_LOGICAL_NOT)

#define TOKEN_IS_LITERAL(token) \
    ((token).type >= TOKEN_NUMBER && (token).type <= TOKEN_IDENTIFIER)

#define TOKEN_IS_PUNCTUATION(token) \
    ((token).type >= TOKEN_SEMICOLON && (token).type <= TOKEN_RIGHT_BRACE)

#define TOKEN_HAS_VALUE(token) \
    ((token).value != NULL)

#endif // LEXER_H