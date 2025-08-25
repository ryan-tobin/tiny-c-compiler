// tests/unit/test_parser.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../../src/lexer.h"
#include "../../src/parser.h"
#include "../../src/ast.h"

// Test helper functions
ast_node_t* parse_string(const char* source) {
    lexer_t* lexer = lexer_create(source);
    if (!lexer) return NULL;
    
    parser_t* parser = parser_create(lexer);
    if (!parser) {
        lexer_destroy(lexer);
        return NULL;
    }
    
    ast_node_t* ast = parser_parse_program(parser);
    
    if (parser_has_errors(parser)) {
        printf("Parse errors:\n");
        parser_print_errors(parser);
        if (ast) ast_destroy(ast);
        ast = NULL;
    }
    
    parser_destroy(parser);
    lexer_destroy(lexer);
    return ast;
}

void test_simple_function() {
    printf("Testing simple function...\n");
    
    const char* source = 
        "int main() {\n"
        "    return 42;\n"
        "}";
    
    ast_node_t* ast = parse_string(source);
    assert(ast != NULL);
    assert(ast->type == AST_PROGRAM);
    assert(ast->data.program.declaration_count == 1);
    
    ast_node_t* func = ast->data.program.declarations[0];
    assert(func->type == AST_FUNCTION_DECL);
    assert(strcmp(func->data.function_decl.name, "main") == 0);
    assert(func->data.function_decl.return_type == TYPE_INT);
    assert(func->data.function_decl.parameter_count == 0);
    assert(func->data.function_decl.body != NULL);
    
    ast_node_t* body = func->data.function_decl.body;
    assert(body->type == AST_COMPOUND_STMT);
    assert(body->data.compound_stmt.statement_count == 1);
    
    ast_node_t* ret_stmt = body->data.compound_stmt.statements[0];
    assert(ret_stmt->type == AST_RETURN_STMT);
    assert(ret_stmt->data.return_stmt.value != NULL);
    
    ast_node_t* value = ret_stmt->data.return_stmt.value;
    assert(value->type == AST_NUMBER);
    assert(value->data.number.value == 42);
    
    ast_destroy(ast);
    printf("✓ Simple function test passed!\n\n");
}

void test_function_with_parameters() {
    printf("Testing function with parameters...\n");
    
    const char* source = 
        "int add(int a, int b) {\n"
        "    return a + b;\n"
        "}";
    
    ast_node_t* ast = parse_string(source);
    assert(ast != NULL);
    assert(ast->type == AST_PROGRAM);
    assert(ast->data.program.declaration_count == 1);
    
    ast_node_t* func = ast->data.program.declarations[0];
    assert(func->type == AST_FUNCTION_DECL);
    assert(strcmp(func->data.function_decl.name, "add") == 0);
    assert(func->data.function_decl.return_type == TYPE_INT);
    assert(func->data.function_decl.parameter_count == 2);
    
    // Check parameters
    ast_node_t* param1 = func->data.function_decl.parameters[0];
    assert(param1->type == AST_PARAMETER);
    assert(strcmp(param1->data.parameter.name, "a") == 0);
    assert(param1->data.parameter.param_type == TYPE_INT);
    
    ast_node_t* param2 = func->data.function_decl.parameters[1];
    assert(param2->type == AST_PARAMETER);
    assert(strcmp(param2->data.parameter.name, "b") == 0);
    assert(param2->data.parameter.param_type == TYPE_INT);
    
    // Check body has return statement with binary op
    ast_node_t* body = func->data.function_decl.body;
    ast_node_t* ret_stmt = body->data.compound_stmt.statements[0];
    ast_node_t* expr = ret_stmt->data.return_stmt.value;
    assert(expr->type == AST_BINARY_OP);
    assert(strcmp(expr->data.binary_op.oper, "+") == 0);
    
    ast_destroy(ast);
    printf("✓ Function with parameters test passed!\n\n");
}

void test_variable_declaration() {
    printf("Testing variable declarations...\n");
    
    const char* source = 
        "int main() {\n"
        "    int x = 10;\n"
        "    int y;\n"
        "    return x;\n"
        "}";
    
    ast_node_t* ast = parse_string(source);
    assert(ast != NULL);
    
    ast_node_t* func = ast->data.program.declarations[0];
    ast_node_t* body = func->data.function_decl.body;
    assert(body->data.compound_stmt.statement_count == 3);
    
    // Check first variable declaration with initializer
    ast_node_t* var1 = body->data.compound_stmt.statements[0];
    assert(var1->type == AST_VARIABLE_DECL);
    assert(strcmp(var1->data.variable_decl.name, "x") == 0);
    assert(var1->data.variable_decl.var_type == TYPE_INT);
    assert(var1->data.variable_decl.initializer != NULL);
    assert(var1->data.variable_decl.initializer->type == AST_NUMBER);
    assert(var1->data.variable_decl.initializer->data.number.value == 10);
    
    // Check second variable declaration without initializer
    ast_node_t* var2 = body->data.compound_stmt.statements[1];
    assert(var2->type == AST_VARIABLE_DECL);
    assert(strcmp(var2->data.variable_decl.name, "y") == 0);
    assert(var2->data.variable_decl.var_type == TYPE_INT);
    assert(var2->data.variable_decl.initializer == NULL);
    
    ast_destroy(ast);
    printf("✓ Variable declaration test passed!\n\n");
}

void test_expressions() {
    printf("Testing expressions...\n");
    
    const char* source = 
        "int main() {\n"
        "    int x = 1 + 2 * 3;\n"
        "    return x;\n"
        "}";
    
    ast_node_t* ast = parse_string(source);
    assert(ast != NULL);
    
    ast_node_t* func = ast->data.program.declarations[0];
    ast_node_t* body = func->data.function_decl.body;
    ast_node_t* var = body->data.compound_stmt.statements[0];
    ast_node_t* expr = var->data.variable_decl.initializer;
    
    // Should be: 1 + (2 * 3) due to precedence
    assert(expr->type == AST_BINARY_OP);
    assert(strcmp(expr->data.binary_op.oper, "+") == 0);
    
    ast_node_t* left = expr->data.binary_op.left;
    assert(left->type == AST_NUMBER);
    assert(left->data.number.value == 1);
    
    ast_node_t* right = expr->data.binary_op.right;
    assert(right->type == AST_BINARY_OP);
    assert(strcmp(right->data.binary_op.oper, "*") == 0);
    assert(right->data.binary_op.left->data.number.value == 2);
    assert(right->data.binary_op.right->data.number.value == 3);
    
    ast_destroy(ast);
    printf("✓ Expression test passed!\n\n");
}

void test_function_call() {
    printf("Testing function calls...\n");
    
    const char* source = 
        "int main() {\n"
        "    print(\"Hello\");\n"
        "    return add(1, 2);\n"
        "}";
    
    ast_node_t* ast = parse_string(source);
    assert(ast != NULL);
    
    ast_node_t* func = ast->data.program.declarations[0];
    ast_node_t* body = func->data.function_decl.body;
    assert(body->data.compound_stmt.statement_count == 2);
    
    // Check first function call (print)
    ast_node_t* expr_stmt = body->data.compound_stmt.statements[0];
    assert(expr_stmt->type == AST_EXPRESSION_STMT);
    
    ast_node_t* call1 = expr_stmt->data.expression_stmt.expression;
    assert(call1->type == AST_FUNCTION_CALL);
    assert(strcmp(call1->data.function_call.name, "print") == 0);
    assert(call1->data.function_call.argument_count == 1);
    
    ast_node_t* arg1 = call1->data.function_call.arguments[0];
    assert(arg1->type == AST_STRING);
    assert(strcmp(arg1->data.string.value, "Hello") == 0);
    
    // Check second function call (add) in return statement
    ast_node_t* ret_stmt = body->data.compound_stmt.statements[1];
    assert(ret_stmt->type == AST_RETURN_STMT);
    
    ast_node_t* call2 = ret_stmt->data.return_stmt.value;
    assert(call2->type == AST_FUNCTION_CALL);
    assert(strcmp(call2->data.function_call.name, "add") == 0);
    assert(call2->data.function_call.argument_count == 2);
    
    ast_destroy(ast);
    printf("✓ Function call test passed!\n\n");
}

void test_if_statement() {
    printf("Testing if statements...\n");
    
    const char* source = 
        "int main() {\n"
        "    if (x > 0) {\n"
        "        return 1;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}";
    
    ast_node_t* ast = parse_string(source);
    assert(ast != NULL);
    
    ast_node_t* func = ast->data.program.declarations[0];
    ast_node_t* body = func->data.function_decl.body;
    ast_node_t* if_stmt = body->data.compound_stmt.statements[0];
    
    assert(if_stmt->type == AST_IF_STMT);
    assert(if_stmt->data.if_stmt.condition != NULL);
    assert(if_stmt->data.if_stmt.then_stmt != NULL);
    assert(if_stmt->data.if_stmt.else_stmt != NULL);
    
    // Check condition (x > 0)
    ast_node_t* condition = if_stmt->data.if_stmt.condition;
    assert(condition->type == AST_BINARY_OP);
    assert(strcmp(condition->data.binary_op.oper, ">") == 0);
    
    ast_destroy(ast);
    printf("✓ If statement test passed!\n\n");
}

void test_while_statement() {
    printf("Testing while statements...\n");
    
    const char* source = 
        "int main() {\n"
        "    while (x < 10) {\n"
        "        x = x + 1;\n"
        "    }\n"
        "    return x;\n"
        "}";
    
    ast_node_t* ast = parse_string(source);
    assert(ast != NULL);
    
    ast_node_t* func = ast->data.program.declarations[0];
    ast_node_t* body = func->data.function_decl.body;
    ast_node_t* while_stmt = body->data.compound_stmt.statements[0];
    
    assert(while_stmt->type == AST_WHILE_STMT);
    assert(while_stmt->data.while_stmt.condition != NULL);
    assert(while_stmt->data.while_stmt.body != NULL);
    
    // Check condition (x < 10)
    ast_node_t* condition = while_stmt->data.while_stmt.condition;
    assert(condition->type == AST_BINARY_OP);
    assert(strcmp(condition->data.binary_op.oper, "<") == 0);
    
    ast_destroy(ast);
    printf("✓ While statement test passed!\n\n");
}

void test_error_handling() {
    printf("Testing error handling...\n");
    
    const char* source = 
        "int main( {\n"  // Missing closing paren
        "    return 42\n"  // Missing semicolon
        "}";
    
    lexer_t* lexer = lexer_create(source);
    parser_t* parser = parser_create(lexer);
    ast_node_t* ast = parser_parse_program(parser);
    
    // Should have errors
    assert(parser_has_errors(parser));
    
    if (ast) ast_destroy(ast);
    parser_destroy(parser);
    lexer_destroy(lexer);
    
    printf("✓ Error handling test passed!\n\n");
}

int main() {
    printf("=== RUNNING PARSER TESTS ===\n\n");
    
    test_simple_function();
    test_function_with_parameters();
    test_variable_declaration();
    test_expressions();
    test_function_call();
    test_if_statement();
    test_while_statement();
    test_error_handling();
    
    printf("🎉 All parser tests passed!\n");
    return 0;
}