# TinyC Language Grammar

## Overview
This document defines the formal grammar for the TinyC programming language using Extended Backus-Naur Form (EBNF).

## Notation
- `::=` means "is defined as"
- `|` means "or" (alternative)
- `*` means "zero or more"
- `+` means "one or more" 
- `?` means "optional" (zero or one)
- `()` groups elements
- `''` contains literal strings
- `UPPERCASE` represents terminal tokens

## Grammar Rules

### Program Structure
```ebnf
program        ::= declaration*
declaration    ::= function_decl | var_decl
```

### Function Declaration
```ebnf
function_decl  ::= type IDENTIFIER '(' parameter_list? ')' compound_stmt
parameter_list ::= parameter (',' parameter)*
parameter      ::= type IDENTIFIER
```

### Variable Declaration
```ebnf
var_decl       ::= type IDENTIFIER ('=' expression)? ';'
```

### Statements
```ebnf
compound_stmt  ::= '{' statement* '}'
statement      ::= compound_stmt 
                | if_stmt 
                | while_stmt 
                | for_stmt 
                | return_stmt 
                | expression_stmt 
                | var_decl

if_stmt        ::= 'if' '(' expression ')' statement ('else' statement)?
while_stmt     ::= 'while' '(' expression ')' statement  
for_stmt       ::= 'for' '(' statement expression ';' expression ')' statement
return_stmt    ::= 'return' expression? ';'
expression_stmt ::= expression? ';'
```

### Expressions (Precedence Order - Lowest to Highest)
```ebnf
expression     ::= assignment
assignment     ::= logical_or ('=' assignment)?
logical_or     ::= logical_and ('||' logical_and)*
logical_and    ::= equality ('&&' equality)*
equality       ::= relational (('==' | '!=') relational)*
relational     ::= additive (('<' | '>' | '<=' | '>=') additive)*
additive       ::= multiplicative (('+' | '-') multiplicative)*
multiplicative ::= unary (('*' | '/' | '%') unary)*
unary          ::= ('!' | '-' | '+')? postfix
postfix        ::= primary ('(' argument_list? ')')*
primary        ::= IDENTIFIER | NUMBER | STRING | '(' expression ')'
```

### Types and Miscellaneous
```ebnf
type           ::= 'int' | 'char' | 'void'
argument_list  ::= expression (',' expression)*
```

## Operator Precedence (Highest to Lowest)
1. `()` - Function calls, grouping
2. `!`, `-`, `+` - Unary operators
3. `*`, `/`, `%` - Multiplicative
4. `+`, `-` - Additive  
5. `<`, `>`, `<=`, `>=` - Relational
6. `==`, `!=` - Equality
7. `&&` - Logical AND
8. `||` - Logical OR
9. `=` - Assignment

## Associativity
- Most operators are left-associative
- Assignment (`=`) is right-associative
- Unary operators are right-associative

## Keywords
```
int, char, void, if, else, while, for, return
```

## Example Valid Programs

### Hello World
```c
int print(char* str);

int main() {
    print("Hello, World!");
    return 0;
}
```

### Fibonacci
```c
int fib(int n) {
    if (n <= 1) {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}
```

### Variable Usage
```c
int main() {
    int x = 42;
    int y = x + 10;
    return y;
}
```