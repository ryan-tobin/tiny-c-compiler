// src/codegen.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "codegen.h"

// Register names for different sizes
static const char* register_names[][3] = {
    {"rax", "eax", "al"},  // REG_RAX
    {"rbx", "ebx", "bl"},  // REG_RBX  
    {"rcx", "ecx", "cl"},  // REG_RCX
    {"rdx", "edx", "dl"},  // REG_RDX
    {"rsi", "esi", "sil"}, // REG_RSI
    {"rdi", "edi", "dil"}, // REG_RDI
    {"r8",  "r8d", "r8b"}, // REG_R8
    {"r9",  "r9d", "r9b"}  // REG_R9
};

// Code generator lifecycle
codegen_t* codegen_create(const char* output_filename) {
    codegen_t* codegen = malloc(sizeof(codegen_t));
    if (!codegen) return NULL;
    
    codegen->output = fopen(output_filename, "w");
    if (!codegen->output) {
        free(codegen);
        return NULL;
    }
    
    codegen->current_function = NULL;
    codegen->string_counter = 0;
    codegen->label_counter = 0;
    codegen->next_temp_register = REG_RAX;
    
    // Initialize register allocation
    for (int i = 0; i < MAX_REGISTERS; i++) {
        codegen->register_used[i] = 0;
    }
    
    // Initialize string literals
    codegen->string_literal_count = 0;
    codegen->string_literal_capacity = 10;
    codegen->string_literals = malloc(codegen->string_literal_capacity * 
                                    sizeof(string_literal_t));
    
    return codegen;
}

void codegen_destroy(codegen_t* codegen) {
    if (!codegen) return;
    
    if (codegen->output) {
        fclose(codegen->output);
    }
    
    if (codegen->current_function) {
        function_context_destroy(codegen->current_function);
    }
    
    // Free string literals
    for (size_t i = 0; i < codegen->string_literal_count; i++) {
        free(codegen->string_literals[i].value);
        free(codegen->string_literals[i].label);
    }
    free(codegen->string_literals);
    
    free(codegen);
}

// Assembly output helpers
void codegen_emit(codegen_t* codegen, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    fprintf(codegen->output, "    ");
    vfprintf(codegen->output, format, args);
    fprintf(codegen->output, "\n");
    
    va_end(args);
}

void codegen_emit_label(codegen_t* codegen, const char* label) {
    fprintf(codegen->output, "%s:\n", label);
}

void codegen_emit_comment(codegen_t* codegen, const char* comment) {
    fprintf(codegen->output, "    # %s\n", comment);
}

// Register management
register_t codegen_allocate_register(codegen_t* codegen) {
    for (int i = 0; i < MAX_REGISTERS; i++) {
        if (!codegen->register_used[i]) {
            codegen->register_used[i] = 1;
            return (register_t)i;
        }
    }
    
    // If we run out of registers, just reuse RAX (simple strategy)
    return REG_RAX;
}

void codegen_free_register(codegen_t* codegen, register_t reg) {
    if (reg >= 0 && reg < MAX_REGISTERS) {
        codegen->register_used[reg] = 0;
    }
}

void codegen_free_all_registers(codegen_t* codegen) {
    for (int i = 0; i < MAX_REGISTERS; i++) {
        codegen->register_used[i] = 0;
    }
}

const char* codegen_register_name(register_t reg, int size) {
    if (reg < 0 || reg >= MAX_REGISTERS) return "INVALID";
    
    switch (size) {
        case 8: return register_names[reg][0];  // 64-bit
        case 4: return register_names[reg][1];  // 32-bit  
        case 1: return register_names[reg][2];  // 8-bit
        default: return register_names[reg][0];
    }
}

// Function context management
function_context_t* function_context_create(const char* name) {
    function_context_t* context = malloc(sizeof(function_context_t));
    if (!context) return NULL;
    
    context->name = malloc(strlen(name) + 1);
    strcpy(context->name, name);
    context->stack_size = 0;
    context->variable_count = 0;
    context->variable_capacity = 10;
    context->label_counter = 0;
    
    context->variables = malloc(context->variable_capacity * sizeof(stack_var_t));
    if (!context->variables) {
        free(context->name);
        free(context);
        return NULL;
    }
    
    return context;
}

void function_context_destroy(function_context_t* context) {
    if (!context) return;
    
    free(context->name);
    
    for (size_t i = 0; i < context->variable_count; i++) {
        free(context->variables[i].name);
    }
    free(context->variables);
    
    free(context);
}

int function_context_add_variable(function_context_t* context, const char* name, data_type_t type) {
    if (!context) return -1;
    
    if (context->variable_count >= context->variable_capacity) {
        context->variable_capacity *= 2;
        context->variables = realloc(context->variables,
                                   context->variable_capacity * sizeof(stack_var_t));
        if (!context->variables) return -1;
    }
    
    stack_var_t* var = &context->variables[context->variable_count++];
    var->name = malloc(strlen(name) + 1);
    strcpy(var->name, name);
    var->type = type;
    
    // Allocate stack space (align to 8 bytes)
    int size = codegen_type_size(type);
    context->stack_size += (size + 7) & ~7;
    var->offset = -context->stack_size;
    
    return var->offset;
}

stack_var_t* function_context_find_variable(function_context_t* context, const char* name) {
    if (!context) return NULL;
    
    for (size_t i = 0; i < context->variable_count; i++) {
        if (strcmp(context->variables[i].name, name) == 0) {
            return &context->variables[i];
        }
    }
    
    return NULL;
}

// Label generation
char* codegen_generate_label(codegen_t* codegen, const char* prefix) {
    char* label = malloc(64);
    snprintf(label, 64, ".L%s%d", prefix, codegen->label_counter++);
    return label;
}

char* codegen_generate_string_label(codegen_t* codegen) {
    char* label = malloc(32);
    snprintf(label, 32, ".LC%d", codegen->string_counter++);
    return label;
}

// String literal management
const char* codegen_add_string_literal(codegen_t* codegen, const char* value) {
    // Check if string already exists
    for (size_t i = 0; i < codegen->string_literal_count; i++) {
        if (strcmp(codegen->string_literals[i].value, value) == 0) {
            return codegen->string_literals[i].label;
        }
    }
    
    // Add new string literal
    if (codegen->string_literal_count >= codegen->string_literal_capacity) {
        codegen->string_literal_capacity *= 2;
        codegen->string_literals = realloc(codegen->string_literals,
                                          codegen->string_literal_capacity * 
                                          sizeof(string_literal_t));
    }
    
    string_literal_t* literal = &codegen->string_literals[codegen->string_literal_count++];
    literal->value = malloc(strlen(value) + 1);
    strcpy(literal->value, value);
    literal->label = codegen_generate_string_label(codegen);
    
    return literal->label;
}

// Utility functions
int codegen_type_size(data_type_t type) {
    switch (type) {
        case TYPE_INT: return 4;
        case TYPE_CHAR: return 1;
        case TYPE_CHAR_PTR: return 8;
        case TYPE_VOID: return 0;
        default: return 4;
    }
}

const char* codegen_type_suffix(data_type_t type) {
    switch (type) {
        case TYPE_INT: return "l";      // 32-bit
        case TYPE_CHAR: return "b";     // 8-bit
        case TYPE_CHAR_PTR: return "q"; // 64-bit
        default: return "l";
    }
}

// Main code generation
int codegen_generate(codegen_t* codegen, ast_node_t* ast) {
    if (!codegen || !ast) return 0;
    
    codegen_program(codegen, ast);
    return 1;
}

void codegen_program(codegen_t* codegen, ast_node_t* node) {
    if (!node || node->type != AST_PROGRAM) return;
    
    // Emit assembly header
    codegen_emit_comment(codegen, "Generated by TinyC Compiler");
    fprintf(codegen->output, ".section .data\n");
    
    // Generate code for all functions first to collect string literals
    for (size_t i = 0; i < node->data.program.declaration_count; i++) {
        ast_node_t* decl = node->data.program.declarations[i];
        if (decl->type == AST_FUNCTION_DECL && decl->data.function_decl.body) {
            // We'll generate functions in the text section
        }
    }
    
    // Emit string literals if any were collected
    if (codegen->string_literal_count > 0) {
        for (size_t i = 0; i < codegen->string_literal_count; i++) {
            fprintf(codegen->output, "%s:\n", codegen->string_literals[i].label);
            fprintf(codegen->output, "    .string \"%s\"\n", codegen->string_literals[i].value);
        }
        fprintf(codegen->output, "\n");
    }
    
    fprintf(codegen->output, ".section .text\n");
    
    // Generate code for all functions
    for (size_t i = 0; i < node->data.program.declaration_count; i++) {
        ast_node_t* decl = node->data.program.declarations[i];
        if (decl->type == AST_FUNCTION_DECL) {
            codegen_function_decl(codegen, decl);
        }
    }
}

void codegen_function_decl(codegen_t* codegen, ast_node_t* node) {
    if (!node || node->type != AST_FUNCTION_DECL) return;
    
    // Skip function declarations without bodies
    if (!node->data.function_decl.body) return;
    
    // Create function context
    codegen->current_function = function_context_create(node->data.function_decl.name);
    
    // Add parameters to function context
    for (size_t i = 0; i < node->data.function_decl.parameter_count; i++) {
        ast_node_t* param = node->data.function_decl.parameters[i];
        function_context_add_variable(codegen->current_function,
                                    param->data.parameter.name,
                                    param->data.parameter.param_type);
    }
    
    // Make main function global
    if (strcmp(node->data.function_decl.name, "main") == 0) {
        fprintf(codegen->output, ".global main\n");
    }
    
    // Function label
    fprintf(codegen->output, "%s:\n", node->data.function_decl.name);
    
    // Function prologue
    codegen_emit(codegen, "pushq %%rbp");
    codegen_emit(codegen, "movq %%rsp, %%rbp");
    
    // Allocate stack space if needed
    if (codegen->current_function->stack_size > 0) {
        codegen_emit(codegen, "subq $%d, %%rsp", 
                    (codegen->current_function->stack_size + 15) & ~15); // Align to 16 bytes
    }
    
    // Generate function body
    codegen_compound_stmt(codegen, node->data.function_decl.body);
    
    // Function epilogue
    codegen_emit_label(codegen, ".Lreturn");
    if (node->data.function_decl.return_type == TYPE_VOID) {
        codegen_emit(codegen, "movq $0, %%rax");  // Default return value for void
    }
    codegen_emit(codegen, "movq %%rbp, %%rsp");
    codegen_emit(codegen, "popq %%rbp");
    codegen_emit(codegen, "ret");
    
    fprintf(codegen->output, "\n");
    
    // Clean up function context
    function_context_destroy(codegen->current_function);
    codegen->current_function = NULL;
}

void codegen_statement(codegen_t* codegen, ast_node_t* node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_COMPOUND_STMT:
            codegen_compound_stmt(codegen, node);
            break;
        case AST_IF_STMT:
            codegen_if_stmt(codegen, node);
            break;
        case AST_WHILE_STMT:
            codegen_while_stmt(codegen, node);
            break;
        case AST_FOR_STMT:
            codegen_for_stmt(codegen, node);
            break;
        case AST_RETURN_STMT:
            codegen_return_stmt(codegen, node);
            break;
        case AST_EXPRESSION_STMT:
            codegen_expression_stmt(codegen, node);
            break;
        case AST_VARIABLE_DECL:
            codegen_variable_decl(codegen, node);
            break;
        default:
            break;
    }
}

void codegen_compound_stmt(codegen_t* codegen, ast_node_t* node) {
    if (!node || node->type != AST_COMPOUND_STMT) return;
    
    for (size_t i = 0; i < node->data.compound_stmt.statement_count; i++) {
        codegen_statement(codegen, node->data.compound_stmt.statements[i]);
    }
}

void codegen_if_stmt(codegen_t* codegen, ast_node_t* node) {
    if (!node || node->type != AST_IF_STMT) return;
    
    char* else_label = codegen_generate_label(codegen, "else");
    char* end_label = codegen_generate_label(codegen, "endif");
    
    // Evaluate condition
    register_t cond_reg = codegen_expression(codegen, node->data.if_stmt.condition);
    codegen_emit(codegen, "testq %%%s, %%%s", 
                codegen_register_name(cond_reg, 8),
                codegen_register_name(cond_reg, 8));
    codegen_free_register(codegen, cond_reg);
    
    // Jump to else if condition is false
    if (node->data.if_stmt.else_stmt) {
        codegen_emit(codegen, "jz %s", else_label);
    } else {
        codegen_emit(codegen, "jz %s", end_label);
    }
    
    // Then branch
    codegen_statement(codegen, node->data.if_stmt.then_stmt);
    
    if (node->data.if_stmt.else_stmt) {
        codegen_emit(codegen, "jmp %s", end_label);
        codegen_emit_label(codegen, else_label);
        codegen_statement(codegen, node->data.if_stmt.else_stmt);
    }
    
    codegen_emit_label(codegen, end_label);
    
    free(else_label);
    free(end_label);
}

void codegen_while_stmt(codegen_t* codegen, ast_node_t* node) {
    if (!node || node->type != AST_WHILE_STMT) return;
    
    char* loop_label = codegen_generate_label(codegen, "while");
    char* end_label = codegen_generate_label(codegen, "endwhile");
    
    // Loop label
    codegen_emit_label(codegen, loop_label);
    
    // Evaluate condition
    register_t cond_reg = codegen_expression(codegen, node->data.while_stmt.condition);
    codegen_emit(codegen, "testq %%%s, %%%s",
                codegen_register_name(cond_reg, 8),
                codegen_register_name(cond_reg, 8));
    codegen_free_register(codegen, cond_reg);
    
    // Jump to end if condition is false
    codegen_emit(codegen, "jz %s", end_label);
    
    // Loop body
    codegen_statement(codegen, node->data.while_stmt.body);
    
    // Jump back to condition
    codegen_emit(codegen, "jmp %s", loop_label);
    
    codegen_emit_label(codegen, end_label);
    
    free(loop_label);
    free(end_label);
}

void codegen_for_stmt(codegen_t* codegen, ast_node_t* node) {
    if (!node || node->type != AST_FOR_STMT) return;
    
    char* loop_label = codegen_generate_label(codegen, "for");
    char* update_label = codegen_generate_label(codegen, "forupdate");
    char* end_label = codegen_generate_label(codegen, "endfor");
    
    // Init statement
    if (node->data.for_stmt.init) {
        codegen_statement(codegen, node->data.for_stmt.init);
    }
    
    // Loop label
    codegen_emit_label(codegen, loop_label);
    
    // Condition
    if (node->data.for_stmt.condition) {
        register_t cond_reg = codegen_expression(codegen, node->data.for_stmt.condition);
        codegen_emit(codegen, "testq %%%s, %%%s",
                    codegen_register_name(cond_reg, 8),
                    codegen_register_name(cond_reg, 8));
        codegen_free_register(codegen, cond_reg);
        codegen_emit(codegen, "jz %s", end_label);
    }
    
    // Body
    codegen_statement(codegen, node->data.for_stmt.body);
    
    // Update
    codegen_emit_label(codegen, update_label);
    if (node->data.for_stmt.update) {
        register_t update_reg = codegen_expression(codegen, node->data.for_stmt.update);
        codegen_free_register(codegen, update_reg);
    }
    
    // Jump back to condition
    codegen_emit(codegen, "jmp %s", loop_label);
    
    codegen_emit_label(codegen, end_label);
    
    free(loop_label);
    free(update_label);
    free(end_label);
}

void codegen_return_stmt(codegen_t* codegen, ast_node_t* node) {
    if (!node || node->type != AST_RETURN_STMT) return;
    
    if (node->data.return_stmt.value) {
        register_t result_reg = codegen_expression(codegen, node->data.return_stmt.value);
        if (result_reg != REG_RAX) {
            codegen_emit(codegen, "movq %%%s, %%rax",
                        codegen_register_name(result_reg, 8));
        }
        codegen_free_register(codegen, result_reg);
    }
    
    codegen_emit(codegen, "jmp .Lreturn");
}

void codegen_expression_stmt(codegen_t* codegen, ast_node_t* node) {
    if (!node || node->type != AST_EXPRESSION_STMT) return;
    
    if (node->data.expression_stmt.expression) {
        register_t reg = codegen_expression(codegen, node->data.expression_stmt.expression);
        codegen_free_register(codegen, reg);
    }
}

void codegen_variable_decl(codegen_t* codegen, ast_node_t* node) {
    if (!node || node->type != AST_VARIABLE_DECL || !codegen->current_function) return;
    
    // Add variable to function context
    function_context_add_variable(codegen->current_function,
                                 node->data.variable_decl.name,
                                 node->data.variable_decl.var_type);
    
    // Handle initializer if present
    if (node->data.variable_decl.initializer) {
        register_t init_reg = codegen_expression(codegen, node->data.variable_decl.initializer);
        stack_var_t* var = function_context_find_variable(codegen->current_function,
                                                         node->data.variable_decl.name);
        if (var) {
            codegen_emit(codegen, "mov%s %%%s, %d(%%rbp)",
                        codegen_type_suffix(var->type),
                        codegen_register_name(init_reg, codegen_type_size(var->type)),
                        var->offset);
        }
        codegen_free_register(codegen, init_reg);
    }
}

// Expression code generation
register_t codegen_expression(codegen_t* codegen, ast_node_t* node) {
    if (!node) return REG_NONE;
    
    switch (node->type) {
        case AST_BINARY_OP:
            return codegen_binary_op(codegen, node);
        case AST_UNARY_OP:
            return codegen_unary_op(codegen, node);
        case AST_FUNCTION_CALL:
            return codegen_function_call(codegen, node);
        case AST_IDENTIFIER:
            return codegen_identifier(codegen, node);
        case AST_NUMBER:
            return codegen_number(codegen, node);
        case AST_STRING:
            return codegen_string(codegen, node);
        default:
            return REG_NONE;
    }
}

register_t codegen_binary_op(codegen_t* codegen, ast_node_t* node) {
    if (!node || node->type != AST_BINARY_OP) return REG_NONE;
    
    // Handle assignment separately
    if (strcmp(node->data.binary_op.oper, "=") == 0) {
        // Right side first
        register_t right_reg = codegen_expression(codegen, node->data.binary_op.right);
        
        // Left side should be an identifier
        if (node->data.binary_op.left->type == AST_IDENTIFIER) {
            stack_var_t* var = function_context_find_variable(codegen->current_function,
                                                             node->data.binary_op.left->data.identifier.name);
            if (var) {
                codegen_emit(codegen, "mov%s %%%s, %d(%%rbp)",
                            codegen_type_suffix(var->type),
                            codegen_register_name(right_reg, codegen_type_size(var->type)),
                            var->offset);
            }
        }
        
        return right_reg;
    }
    
    // Arithmetic and comparison opers
    register_t left_reg = codegen_expression(codegen, node->data.binary_op.left);
    register_t right_reg = codegen_expression(codegen, node->data.binary_op.right);
    register_t result_reg = left_reg;
    
    const char* op = node->data.binary_op.oper;
    
    if (strcmp(op, "+") == 0) {
        codegen_emit(codegen, "addq %%%s, %%%s",
                    codegen_register_name(right_reg, 8),
                    codegen_register_name(left_reg, 8));
    } else if (strcmp(op, "-") == 0) {
        codegen_emit(codegen, "subq %%%s, %%%s",
                    codegen_register_name(right_reg, 8),
                    codegen_register_name(left_reg, 8));
    } else if (strcmp(op, "*") == 0) {
        codegen_emit(codegen, "imulq %%%s, %%%s",
                    codegen_register_name(right_reg, 8),
                    codegen_register_name(left_reg, 8));
    } else if (strcmp(op, "<") == 0) {
        codegen_emit(codegen, "cmpq %%%s, %%%s",
                    codegen_register_name(right_reg, 8),
                    codegen_register_name(left_reg, 8));
        codegen_emit(codegen, "setl %%%s",
                    codegen_register_name(left_reg, 1));
        codegen_emit(codegen, "movzbl %%%s, %%%s",
                    codegen_register_name(left_reg, 1),
                    codegen_register_name(left_reg, 4));
    } else if (strcmp(op, "==") == 0) {
        codegen_emit(codegen, "cmpq %%%s, %%%s",
                    codegen_register_name(right_reg, 8),
                    codegen_register_name(left_reg, 8));
        codegen_emit(codegen, "sete %%%s",
                    codegen_register_name(left_reg, 1));
        codegen_emit(codegen, "movzbl %%%s, %%%s",
                    codegen_register_name(left_reg, 1),
                    codegen_register_name(left_reg, 4));
    }
    // Add more opers as needed
    
    codegen_free_register(codegen, right_reg);
    return result_reg;
}

register_t codegen_unary_op(codegen_t* codegen, ast_node_t* node) {
    if (!node || node->type != AST_UNARY_OP) return REG_NONE;
    
    register_t operand_reg = codegen_expression(codegen, node->data.unary_op.operand);
    const char* op = node->data.unary_op.oper;
    
    if (strcmp(op, "-") == 0) {
        codegen_emit(codegen, "negq %%%s", codegen_register_name(operand_reg, 8));
    } else if (strcmp(op, "!") == 0) {
        codegen_emit(codegen, "testq %%%s, %%%s",
                    codegen_register_name(operand_reg, 8),
                    codegen_register_name(operand_reg, 8));
        codegen_emit(codegen, "sete %%%s", codegen_register_name(operand_reg, 1));
        codegen_emit(codegen, "movzbl %%%s, %%%s",
                    codegen_register_name(operand_reg, 1),
                    codegen_register_name(operand_reg, 4));
    }
    
    return operand_reg;
}

register_t codegen_function_call(codegen_t* codegen, ast_node_t* node) {
    if (!node || node->type != AST_FUNCTION_CALL) return REG_NONE;
    
    // Simple function call - just call and assume result in RAX
    codegen_emit(codegen, "call %s", node->data.function_call.name);
    
    register_t result_reg = codegen_allocate_register(codegen);
    if (result_reg != REG_RAX) {
        codegen_emit(codegen, "movq %%rax, %%%s", codegen_register_name(result_reg, 8));
    }
    
    return result_reg;
}

register_t codegen_identifier(codegen_t* codegen, ast_node_t* node) {
    if (!node || node->type != AST_IDENTIFIER || !codegen->current_function) return REG_NONE;
    
    stack_var_t* var = function_context_find_variable(codegen->current_function,
                                                     node->data.identifier.name);
    if (!var) return REG_NONE;
    
    register_t reg = codegen_allocate_register(codegen);
    codegen_emit(codegen, "mov%s %d(%%rbp), %%%s",
                codegen_type_suffix(var->type),
                var->offset,
                codegen_register_name(reg, codegen_type_size(var->type)));
    
    return reg;
}

register_t codegen_number(codegen_t* codegen, ast_node_t* node) {
    if (!node || node->type != AST_NUMBER) return REG_NONE;
    
    register_t reg = codegen_allocate_register(codegen);
    codegen_emit(codegen, "movq $%d, %%%s",
                node->data.number.value,
                codegen_register_name(reg, 8));
    
    return reg;
}

register_t codegen_string(codegen_t* codegen, ast_node_t* node) {
    if (!node || node->type != AST_STRING) return REG_NONE;
    
    const char* label = codegen_add_string_literal(codegen, node->data.string.value);
    register_t reg = codegen_allocate_register(codegen);
    codegen_emit(codegen, "movq $%s, %%%s", label, codegen_register_name(reg, 8));
    
    return reg;
}