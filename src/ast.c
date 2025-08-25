// scr/ast.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

// Create a new AST node
ast_node_t* ast_create_node(ast_node_type_t type) {
    ast_node_t* node =  malloc(sizeof(ast_node_t));
    if (!node) {
        fprintf(stderr, "Error: Failed to allocate memory for AST node\n");
        return NULL;
    }

    node->type= type;
    node->data_type = TYPE_VOID; // Defualt

    memset(&node->data, 0, sizeof(node->data));

    return node;
}

// Create specific node types with helper functions
ast_node_t* ast_create_program() {
    ast_node_t* node = ast_create_node(AST_PROGRAM);
    if (!node) return NULL;

    node->data.program.declarations = NULL;
    node->data.program.declaration_count = 0;

    return node;
}

ast_node_t* ast_create_function_decl(data_type_t return_type, const char* name) {
    ast_node_t* node = ast_create_node(AST_FUNCTION_DECL);
    if (!node) return NULL;

    node->data.function_decl.return_type = return_type;
    node->data.function_decl.name = malloc(strlen(name) + 1);
    if (!node->data.function_decl.name) {
        free(node);
        return NULL;
    }
    strcpy(node->data.function_decl.name, name);

    node->data.function_decl.parameters = NULL;
    node->data.function_decl.parameter_count = 0;
    node->data.function_decl.body = NULL;

    return node;
}

ast_node_t* ast_create_variable_decl(data_type_t var_type, const char* name, ast_node_t* intializer) {
    ast_node_t* node = ast_create_node(AST_VARIABLE_DECL);
    if (!node) return NULL;

    node->data.variable_decl.var_type = var_type;
    node->data.variable_decl.name = malloc(strlen(name) + 1);
    if (!node->data.variable_decl.name) {
        free(node);
        return NULL;
    }
    strcpy(node->data.variable_decl.name, name);
    node->data.variable_decl.initializer = intializer;

    return node;
}

ast_node_t* ast_create_parameter(data_type_t param_type, const char* name) {
    ast_node_t* node = ast_create_node(AST_PARAMETER);
    if (!node) return NULL;
    
    node->data.parameter.param_type = param_type;
    node->data.parameter.name = malloc(strlen(name) + 1);
    if (!node->data.parameter.name) {
        free(node);
        return NULL;
    }
    strcpy(node->data.parameter.name, name);
    
    return node;
}

ast_node_t* ast_create_compound_stmt() {
    ast_node_t* node = ast_create_node(AST_COMPOUND_STMT);
    if (!node) return NULL;
    
    node->data.compound_stmt.statements = NULL;
    node->data.compound_stmt.statement_count = 0;
    
    return node;
}

ast_node_t* ast_create_if_stmt(ast_node_t* condition, ast_node_t* then_stmt, ast_node_t* else_stmt) {
    ast_node_t* node = ast_create_node(AST_IF_STMT);
    if (!node) return NULL;
    
    node->data.if_stmt.condition = condition;
    node->data.if_stmt.then_stmt = then_stmt;
    node->data.if_stmt.else_stmt = else_stmt;
    
    return node;
}

ast_node_t* ast_create_while_stmt(ast_node_t* condition, ast_node_t* body) {
    ast_node_t* node = ast_create_node(AST_WHILE_STMT);
    if (!node) return NULL;
    
    node->data.while_stmt.condition = condition;
    node->data.while_stmt.body = body;
    
    return node;
}

ast_node_t* ast_create_for_stmt(ast_node_t* init, ast_node_t* condition, ast_node_t* update, ast_node_t* body) {
    ast_node_t* node = ast_create_node(AST_FOR_STMT);
    if (!node) return NULL;
    
    node->data.for_stmt.init = init;
    node->data.for_stmt.condition = condition;
    node->data.for_stmt.update = update;
    node->data.for_stmt.body = body;
    
    return node;
}

ast_node_t* ast_create_return_stmt(ast_node_t* value) {
    ast_node_t* node = ast_create_node(AST_RETURN_STMT);
    if (!node) return NULL;
    
    node->data.return_stmt.value = value;
    
    return node;
}

ast_node_t* ast_create_expression_stmt(ast_node_t* expression) {
    ast_node_t* node = ast_create_node(AST_EXPRESSION_STMT);
    if (!node) return NULL;
    
    node->data.expression_stmt.expression = expression;
    
    return node;
}

ast_node_t* ast_create_binary_op(const char* oper, ast_node_t* left, ast_node_t* right) {
    ast_node_t* node = ast_create_node(AST_BINARY_OP);
    if (!node) return NULL;
    
    node->data.binary_op.oper = malloc(strlen(oper) + 1);
    if (!node->data.binary_op.oper) {
        free(node);
        return NULL;
    }
    strcpy(node->data.binary_op.oper, oper);
    
    node->data.binary_op.left = left;
    node->data.binary_op.right = right;
    
    return node;
}

ast_node_t* ast_create_unary_op(const char* oper, ast_node_t* operand) {
    ast_node_t* node = ast_create_node(AST_UNARY_OP);
    if (!node) return NULL;
    
    node->data.unary_op.oper = malloc(strlen(oper) + 1);
    if (!node->data.unary_op.oper) {
        free(node);
        return NULL;
    }
    strcpy(node->data.unary_op.oper, oper);
    
    node->data.unary_op.operand = operand;
    
    return node;
}

ast_node_t* ast_create_function_call(const char* name) {
    ast_node_t* node = ast_create_node(AST_FUNCTION_CALL);
    if (!node) return NULL;
    
    node->data.function_call.name = malloc(strlen(name) + 1);
    if (!node->data.function_call.name) {
        free(node);
        return NULL;
    }
    strcpy(node->data.function_call.name, name);
    
    node->data.function_call.arguments = NULL;
    node->data.function_call.argument_count = 0;
    
    return node;
}

ast_node_t* ast_create_identifier(const char* name) {
    ast_node_t* node = ast_create_node(AST_IDENTIFIER);
    if (!node) return NULL;
    
    node->data.identifier.name = malloc(strlen(name) + 1);
    if (!node->data.identifier.name) {
        free(node);
        return NULL;
    }
    strcpy(node->data.identifier.name, name);
    
    return node;
}

ast_node_t* ast_create_number(int value) {
    ast_node_t* node = ast_create_node(AST_NUMBER);
    if (!node) return NULL;
    
    node->data.number.value = value;
    node->data_type = TYPE_INT;
    
    return node;
}

ast_node_t* ast_create_string(const char* value) {
    ast_node_t* node = ast_create_node(AST_STRING);
    if (!node) return NULL;
    
    node->data.string.value = malloc(strlen(value) + 1);
    if (!node->data.string.value) {
        free(node);
        return NULL;
    }
    strcpy(node->data.string.value, value);
    node->data_type = TYPE_CHAR_PTR;
    
    return node;
}

// Helper functions for dynamic arrays
void ast_add_declaration(ast_node_t* program, ast_node_t* declaration) {
    if (!program || program->type != AST_PROGRAM || !declaration) return;
    
    program->data.program.declarations = realloc(
        program->data.program.declarations,
        (program->data.program.declaration_count + 1) * sizeof(ast_node_t*)
    );
    
    if (!program->data.program.declarations) {
        fprintf(stderr, "Error: Failed to reallocate memory for declarations\n");
        return;
    }
    
    program->data.program.declarations[program->data.program.declaration_count] = declaration;
    program->data.program.declaration_count++;
}

void ast_add_parameter(ast_node_t* function, ast_node_t* parameter) {
    if (!function || function->type != AST_FUNCTION_DECL || !parameter) return;
    
    function->data.function_decl.parameters = realloc(
        function->data.function_decl.parameters,
        (function->data.function_decl.parameter_count + 1) * sizeof(ast_node_t*)
    );
    
    if (!function->data.function_decl.parameters) {
        fprintf(stderr, "Error: Failed to reallocate memory for parameters\n");
        return;
    }
    
    function->data.function_decl.parameters[function->data.function_decl.parameter_count] = parameter;
    function->data.function_decl.parameter_count++;
}

void ast_add_statement(ast_node_t* compound, ast_node_t* statement) {
    if (!compound || compound->type != AST_COMPOUND_STMT || !statement) return;
    
    compound->data.compound_stmt.statements = realloc(
        compound->data.compound_stmt.statements,
        (compound->data.compound_stmt.statement_count + 1) * sizeof(ast_node_t*)
    );
    
    if (!compound->data.compound_stmt.statements) {
        fprintf(stderr, "Error: Failed to reallocate memory for statements\n");
        return;
    }
    
    compound->data.compound_stmt.statements[compound->data.compound_stmt.statement_count] = statement;
    compound->data.compound_stmt.statement_count++;
}

void ast_add_argument(ast_node_t* function_call, ast_node_t* argument) {
    if (!function_call || function_call->type != AST_FUNCTION_CALL || !argument) return;
    
    function_call->data.function_call.arguments = realloc(
        function_call->data.function_call.arguments,
        (function_call->data.function_call.argument_count + 1) * sizeof(ast_node_t*)
    );
    
    if (!function_call->data.function_call.arguments) {
        fprintf(stderr, "Error: Failed to reallocate memory for arguments\n");
        return;
    }
    
    function_call->data.function_call.arguments[function_call->data.function_call.argument_count] = argument;
    function_call->data.function_call.argument_count++;
}

// Destroy AST and free all memory
void ast_destroy(ast_node_t* node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_PROGRAM:
            for (size_t i = 0; i < node->data.program.declaration_count; i++) {
                ast_destroy(node->data.program.declarations[i]);
            }
            free(node->data.program.declarations);
            break;
            
        case AST_FUNCTION_DECL:
            free(node->data.function_decl.name);
            for (size_t i = 0; i < node->data.function_decl.parameter_count; i++) {
                ast_destroy(node->data.function_decl.parameters[i]);
            }
            free(node->data.function_decl.parameters);
            ast_destroy(node->data.function_decl.body);
            break;
            
        case AST_VARIABLE_DECL:
            free(node->data.variable_decl.name);
            ast_destroy(node->data.variable_decl.initializer);
            break;
            
        case AST_PARAMETER:
            free(node->data.parameter.name);
            break;
            
        case AST_COMPOUND_STMT:
            for (size_t i = 0; i < node->data.compound_stmt.statement_count; i++) {
                ast_destroy(node->data.compound_stmt.statements[i]);
            }
            free(node->data.compound_stmt.statements);
            break;
            
        case AST_IF_STMT:
            ast_destroy(node->data.if_stmt.condition);
            ast_destroy(node->data.if_stmt.then_stmt);
            ast_destroy(node->data.if_stmt.else_stmt);
            break;
            
        case AST_WHILE_STMT:
            ast_destroy(node->data.while_stmt.condition);
            ast_destroy(node->data.while_stmt.body);
            break;
            
        case AST_FOR_STMT:
            ast_destroy(node->data.for_stmt.init);
            ast_destroy(node->data.for_stmt.condition);
            ast_destroy(node->data.for_stmt.update);
            ast_destroy(node->data.for_stmt.body);
            break;
            
        case AST_RETURN_STMT:
            ast_destroy(node->data.return_stmt.value);
            break;
            
        case AST_EXPRESSION_STMT:
            ast_destroy(node->data.expression_stmt.expression);
            break;
            
        case AST_BINARY_OP:
            free(node->data.binary_op.oper);
            ast_destroy(node->data.binary_op.left);
            ast_destroy(node->data.binary_op.right);
            break;
            
        case AST_UNARY_OP:
            free(node->data.unary_op.oper);
            ast_destroy(node->data.unary_op.operand);
            break;
            
        case AST_FUNCTION_CALL:
            free(node->data.function_call.name);
            for (size_t i = 0; i < node->data.function_call.argument_count; i++) {
                ast_destroy(node->data.function_call.arguments[i]);
            }
            free(node->data.function_call.arguments);
            break;
            
        case AST_IDENTIFIER:
            free(node->data.identifier.name);
            break;
            
        case AST_NUMBER:
            // No dynamic memory to free
            break;
            
        case AST_STRING:
            free(node->data.string.value);
            break;
    }
    
    free(node);
}

// Convert AST node type to string (for debugging)
const char* ast_node_type_to_string(ast_node_type_t type) {
    switch (type) {
        case AST_PROGRAM: return "PROGRAM";
        case AST_FUNCTION_DECL: return "FUNCTION_DECL";
        case AST_VARIABLE_DECL: return "VARIABLE_DECL";
        case AST_PARAMETER: return "PARAMETER";
        case AST_COMPOUND_STMT: return "COMPOUND_STMT";
        case AST_IF_STMT: return "IF_STMT";
        case AST_WHILE_STMT: return "WHILE_STMT";
        case AST_FOR_STMT: return "FOR_STMT";
        case AST_RETURN_STMT: return "RETURN_STMT";
        case AST_EXPRESSION_STMT: return "EXPRESSION_STMT";
        case AST_BINARY_OP: return "BINARY_OP";
        case AST_UNARY_OP: return "UNARY_OP";
        case AST_FUNCTION_CALL: return "FUNCTION_CALL";
        case AST_IDENTIFIER: return "IDENTIFIER";
        case AST_NUMBER: return "NUMBER";
        case AST_STRING: return "STRING";
        default: return "UNKNOWN";
    }
}

const char* data_type_to_string(data_type_t type) {
    switch (type) {
        case TYPE_INT: return "int";
        case TYPE_CHAR: return "char";
        case TYPE_VOID: return "void";
        case TYPE_CHAR_PTR: return "char*";
        default: return "unknown";
    }
}


// Print AST (for debugging)
void ast_print(ast_node_t* node, int indent) {
    if (!node) return;
    
    // Print indentation
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
    
    printf("%s", ast_node_type_to_string(node->type));
    
    switch (node->type) {
        case AST_PROGRAM:
            printf(" (%zu declarations)\n", node->data.program.declaration_count);
            for (size_t i = 0; i < node->data.program.declaration_count; i++) {
                ast_print(node->data.program.declarations[i], indent + 1);
            }
            break;
            
        case AST_FUNCTION_DECL:
            printf(" %s -> %s (%zu params)\n", 
                   node->data.function_decl.name,
                   data_type_to_string(node->data.function_decl.return_type),
                   node->data.function_decl.parameter_count);
            for (size_t i = 0; i < node->data.function_decl.parameter_count; i++) {
                ast_print(node->data.function_decl.parameters[i], indent + 1);
            }
            if (node->data.function_decl.body) {
                ast_print(node->data.function_decl.body, indent + 1);
            }
            break;
            
        case AST_VARIABLE_DECL:
            printf(" %s : %s\n", 
                   node->data.variable_decl.name,
                   data_type_to_string(node->data.variable_decl.var_type));
            if (node->data.variable_decl.initializer) {
                ast_print(node->data.variable_decl.initializer, indent + 1);
            }
            break;
            
        case AST_PARAMETER:
            printf(" %s : %s\n",
                   node->data.parameter.name,
                   data_type_to_string(node->data.parameter.param_type));
            break;
            
        case AST_COMPOUND_STMT:
            printf(" (%zu statements)\n", node->data.compound_stmt.statement_count);
            for (size_t i = 0; i < node->data.compound_stmt.statement_count; i++) {
                ast_print(node->data.compound_stmt.statements[i], indent + 1);
            }
            break;
            
        case AST_IF_STMT:
            printf("\n");
            for (int i = 0; i <= indent; i++) printf("  ");
            printf("condition:\n");
            ast_print(node->data.if_stmt.condition, indent + 2);
            for (int i = 0; i <= indent; i++) printf("  ");
            printf("then:\n");
            ast_print(node->data.if_stmt.then_stmt, indent + 2);
            if (node->data.if_stmt.else_stmt) {
                for (int i = 0; i <= indent; i++) printf("  ");
                printf("else:\n");
                ast_print(node->data.if_stmt.else_stmt, indent + 2);
            }
            break;
            
        case AST_WHILE_STMT:
            printf("\n");
            for (int i = 0; i <= indent; i++) printf("  ");
            printf("condition:\n");
            ast_print(node->data.while_stmt.condition, indent + 2);
            for (int i = 0; i <= indent; i++) printf("  ");
            printf("body:\n");
            ast_print(node->data.while_stmt.body, indent + 2);
            break;
            
        case AST_FOR_STMT:
            printf("\n");
            if (node->data.for_stmt.init) {
                for (int i = 0; i <= indent; i++) printf("  ");
                printf("init:\n");
                ast_print(node->data.for_stmt.init, indent + 2);
            }
            if (node->data.for_stmt.condition) {
                for (int i = 0; i <= indent; i++) printf("  ");
                printf("condition:\n");
                ast_print(node->data.for_stmt.condition, indent + 2);
            }
            if (node->data.for_stmt.update) {
                for (int i = 0; i <= indent; i++) printf("  ");
                printf("update:\n");
                ast_print(node->data.for_stmt.update, indent + 2);
            }
            for (int i = 0; i <= indent; i++) printf("  ");
            printf("body:\n");
            ast_print(node->data.for_stmt.body, indent + 2);
            break;
            
        case AST_RETURN_STMT:
            printf("\n");
            if (node->data.return_stmt.value) {
                ast_print(node->data.return_stmt.value, indent + 1);
            }
            break;
            
        case AST_EXPRESSION_STMT:
            printf("\n");
            if (node->data.expression_stmt.expression) {
                ast_print(node->data.expression_stmt.expression, indent + 1);
            }
            break;
            
        case AST_BINARY_OP:
            printf(" '%s'\n", node->data.binary_op.oper);
            ast_print(node->data.binary_op.left, indent + 1);
            ast_print(node->data.binary_op.right, indent + 1);
            break;
            
        case AST_UNARY_OP:
            printf(" '%s'\n", node->data.unary_op.oper);
            ast_print(node->data.unary_op.operand, indent + 1);
            break;
            
        case AST_FUNCTION_CALL:
            printf(" %s (%zu args)\n", 
                   node->data.function_call.name,
                   node->data.function_call.argument_count);
            for (size_t i = 0; i < node->data.function_call.argument_count; i++) {
                ast_print(node->data.function_call.arguments[i], indent + 1);
            }
            break;
            
        case AST_IDENTIFIER:
            printf(" '%s'\n", node->data.identifier.name);
            break;
            
        case AST_NUMBER:
            printf(" %d\n", node->data.number.value);
            break;
            
        case AST_STRING:
            printf(" \"%s\"\n", node->data.string.value);
            break;
    }
}
