// tests/unit/test_lexer.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../../src/lexer.h"

// Test helper function
void assert_token(token_t token, token_type_t expected_type, const char* expected_value) {
    printf("  Token: %s", token_type_to_string(token.type));
    if (token.value) {
        printf(" '%s'", token.value);
    }
    printf(" [%d:%d]", token.line, token.column);
    
    if (token.type == expected_type) {
        printf(" ✓");
    } else {
        printf(" ✗ (expected %s)", token_type_to_string(expected_type));
        exit(1);
    }
    
    if (expected_value) {
        if (token.value && strcmp(token.value, expected_value) == 0) {
            printf(" ✓");
        } else {
            printf(" ✗ (expected value '%s')", expected_value);
            exit(1);
        }
    }
    
    printf("\n");
}

void test_keywords() {
    printf("Testing keywords...\n");
    lexer_t* lexer = lexer_create("int char void if else while for return");
    
    assert_token(lexer_next_token(lexer), TOKEN_INT, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_CHAR, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_VOID, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_IF, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_ELSE, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_WHILE, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_FOR, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_RETURN, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_EOF, NULL);
    
    lexer_destroy(lexer);
    printf("Keywords test passed!\n\n");
}

void test_operators() {
    printf("Testing operators...\n");
    lexer_t* lexer = lexer_create("+ - * / % = == != < <= > >= && || !");
    
    assert_token(lexer_next_token(lexer), TOKEN_PLUS, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_MINUS, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_MULTIPLY, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_DIVIDE, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_MODULO, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_ASSIGN, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_EQUAL, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_NOT_EQUAL, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_LESS, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_LESS_EQUAL, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_GREATER, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_GREATER_EQUAL, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_LOGICAL_AND, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_LOGICAL_OR, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_LOGICAL_NOT, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_EOF, NULL);
    
    lexer_destroy(lexer);
    printf("Operators test passed!\n\n");
}

void test_punctuation() {
    printf("Testing punctuation...\n");
    lexer_t* lexer = lexer_create("; , ( ) { }");
    
    assert_token(lexer_next_token(lexer), TOKEN_SEMICOLON, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_COMMA, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_LEFT_PAREN, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_RIGHT_PAREN, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_LEFT_BRACE, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_RIGHT_BRACE, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_EOF, NULL);
    
    lexer_destroy(lexer);
    printf("Punctuation test passed!\n\n");
}

void test_identifiers() {
    printf("Testing identifiers...\n");
    lexer_t* lexer = lexer_create("main foo bar123 _test variable_name");
    
    assert_token(lexer_next_token(lexer), TOKEN_IDENTIFIER, "main");
    assert_token(lexer_next_token(lexer), TOKEN_IDENTIFIER, "foo");
    assert_token(lexer_next_token(lexer), TOKEN_IDENTIFIER, "bar123");
    assert_token(lexer_next_token(lexer), TOKEN_IDENTIFIER, "_test");
    assert_token(lexer_next_token(lexer), TOKEN_IDENTIFIER, "variable_name");
    assert_token(lexer_next_token(lexer), TOKEN_EOF, NULL);
    
    lexer_destroy(lexer);
    printf("Identifiers test passed!\n\n");
}

void test_numbers() {
    printf("Testing numbers...\n");
    lexer_t* lexer = lexer_create("42 0 123 999");
    
    assert_token(lexer_next_token(lexer), TOKEN_NUMBER, "42");
    assert_token(lexer_next_token(lexer), TOKEN_NUMBER, "0");
    assert_token(lexer_next_token(lexer), TOKEN_NUMBER, "123");
    assert_token(lexer_next_token(lexer), TOKEN_NUMBER, "999");
    assert_token(lexer_next_token(lexer), TOKEN_EOF, NULL);
    
    lexer_destroy(lexer);
    printf("Numbers test passed!\n\n");
}

void test_strings() {
    printf("Testing strings...\n");
    lexer_t* lexer = lexer_create("\"Hello, World!\" \"test\" \"\"");
    
    assert_token(lexer_next_token(lexer), TOKEN_STRING, "Hello, World!");
    assert_token(lexer_next_token(lexer), TOKEN_STRING, "test");
    assert_token(lexer_next_token(lexer), TOKEN_STRING, "");
    assert_token(lexer_next_token(lexer), TOKEN_EOF, NULL);
    
    lexer_destroy(lexer);
    printf("Strings test passed!\n\n");
}

void test_comments() {
    printf("Testing comments...\n");
    lexer_t* lexer = lexer_create("int x; // line comment\nint y; /* block comment */ int z;");
    
    assert_token(lexer_next_token(lexer), TOKEN_INT, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_IDENTIFIER, "x");
    assert_token(lexer_next_token(lexer), TOKEN_SEMICOLON, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_INT, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_IDENTIFIER, "y");
    assert_token(lexer_next_token(lexer), TOKEN_SEMICOLON, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_INT, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_IDENTIFIER, "z");
    assert_token(lexer_next_token(lexer), TOKEN_SEMICOLON, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_EOF, NULL);
    
    lexer_destroy(lexer);
    printf("Comments test passed!\n\n");
}

void test_complete_program() {
    printf("Testing complete program...\n");
    const char* program = 
        "int main() {\n"
        "    int x = 42;\n"
        "    return x;\n"
        "}";
    
    lexer_t* lexer = lexer_create(program);
    
    assert_token(lexer_next_token(lexer), TOKEN_INT, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_IDENTIFIER, "main");
    assert_token(lexer_next_token(lexer), TOKEN_LEFT_PAREN, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_RIGHT_PAREN, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_LEFT_BRACE, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_INT, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_IDENTIFIER, "x");
    assert_token(lexer_next_token(lexer), TOKEN_ASSIGN, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_NUMBER, "42");
    assert_token(lexer_next_token(lexer), TOKEN_SEMICOLON, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_RETURN, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_IDENTIFIER, "x");
    assert_token(lexer_next_token(lexer), TOKEN_SEMICOLON, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_RIGHT_BRACE, NULL);
    assert_token(lexer_next_token(lexer), TOKEN_EOF, NULL);
    
    lexer_destroy(lexer);
    printf("Complete program test passed!\n\n");
}

int main() {
    printf("=== RUNNING LEXER TESTS ===\n\n");
    
    test_keywords();
    test_operators();
    test_punctuation();
    test_identifiers();
    test_numbers();
    test_strings();
    test_comments();
    test_complete_program();
    
    printf("🎉 All lexer tests passed!\n");
    return 0;
}