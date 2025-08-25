CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -g -O0 -DDEBUG
LDFLAGS = 
TARGET = tcc
SRC_DIR = src
TEST_DIR = tests
EXAMPLE_DIR = examples
BUILD_DIR = build

# Source files (complete compiler)
COMPILER_SOURCES = $(SRC_DIR)/lexer.c $(SRC_DIR)/ast.c $(SRC_DIR)/parser.c $(SRC_DIR)/semantic.c $(SRC_DIR)/codegen.c $(SRC_DIR)/main.c
COMPILER_OBJECTS = $(COMPILER_SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Test files
TEST_LEXER_SOURCES = $(TEST_DIR)/unit/test_lexer.c $(SRC_DIR)/lexer.c
TEST_LEXER_OBJECTS = $(BUILD_DIR)/tests/unit/test_lexer.o $(BUILD_DIR)/lexer.o

TEST_PARSER_SOURCES = $(TEST_DIR)/unit/test_parser.c $(SRC_DIR)/lexer.c $(SRC_DIR)/ast.c $(SRC_DIR)/parser.c
TEST_PARSER_OBJECTS = $(BUILD_DIR)/tests/unit/test_parser.o $(BUILD_DIR)/lexer.o $(BUILD_DIR)/ast.o $(BUILD_DIR)/parser.o

TEST_SEMANTIC_SOURCES = $(TEST_DIR)/unit/test_semantic.c $(SRC_DIR)/lexer.c $(SRC_DIR)/ast.c $(SRC_DIR)/parser.c $(SRC_DIR)/semantic.c
TEST_SEMANTIC_OBJECTS = $(BUILD_DIR)/tests/unit/test_semantic.o $(BUILD_DIR)/lexer.o $(BUILD_DIR)/ast.o $(BUILD_DIR)/parser.o $(BUILD_DIR)/semantic.o

TEST_CODEGEN_SOURCES = $(TEST_DIR)/unit/test_codegen.c $(SRC_DIR)/lexer.c $(SRC_DIR)/ast.c $(SRC_DIR)/parser.c $(SRC_DIR)/semantic.c $(SRC_DIR)/codegen.c
TEST_CODEGEN_OBJECTS = $(BUILD_DIR)/tests/unit/test_codegen.o $(BUILD_DIR)/lexer.o $(BUILD_DIR)/ast.o $(BUILD_DIR)/parser.o $(BUILD_DIR)/semantic.o $(BUILD_DIR)/codegen.o

.PHONY: all clean test test-lexer test-parser test-semantic test-codegen examples debug help

all: $(BUILD_DIR)/$(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/tests/unit

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile test files
$(BUILD_DIR)/tests/unit/%.o: $(TEST_DIR)/unit/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -c $< -o $@

# Link main executable (complete compiler)
$(BUILD_DIR)/$(TARGET): $(COMPILER_OBJECTS) | $(BUILD_DIR)
	$(CC) $(COMPILER_OBJECTS) -o $@ $(LDFLAGS)

# Test targets
test: test-lexer test-parser test-semantic test-codegen

test-lexer: $(BUILD_DIR)/test_lexer
	@echo "Running lexer unit tests..."
	./$(BUILD_DIR)/test_lexer

test-parser: $(BUILD_DIR)/test_parser
	@echo "Running parser unit tests..."
	./$(BUILD_DIR)/test_parser

test-semantic: $(BUILD_DIR)/test_semantic
	@echo "Running semantic analysis unit tests..."
	./$(BUILD_DIR)/test_semantic

test-codegen: $(BUILD_DIR)/test_codegen
	@echo "Running code generation unit tests..."
	./$(BUILD_DIR)/test_codegen

$(BUILD_DIR)/test_lexer: $(TEST_LEXER_OBJECTS) | $(BUILD_DIR)
	$(CC) $(TEST_LEXER_OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/test_parser: $(TEST_PARSER_OBJECTS) | $(BUILD_DIR)
	$(CC) $(TEST_PARSER_OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/test_semantic: $(TEST_SEMANTIC_OBJECTS) | $(BUILD_DIR)
	$(CC) $(TEST_SEMANTIC_OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/test_codegen: $(TEST_CODEGEN_OBJECTS) | $(BUILD_DIR)
	$(CC) $(TEST_CODEGEN_OBJECTS) -o $@ $(LDFLAGS) $(LDFLAGS)

# Test with example programs
examples: $(BUILD_DIR)/$(TARGET)
	@echo "Testing lexer with example programs..."
	@echo "\n=== Hello World ==="
	./$(BUILD_DIR)/$(TARGET) --debug-tokens $(EXAMPLE_DIR)/hello_world.tc
	@echo "\n=== Fibonacci ==="
	./$(BUILD_DIR)/$(TARGET) --debug-tokens $(EXAMPLE_DIR)/fibonacci.tc
	@echo "\n=== Factorial ==="
	./$(BUILD_DIR)/$(TARGET) --debug-tokens $(EXAMPLE_DIR)/factorial.tc

# Debug build
debug: CFLAGS += -g3 -DDEBUG -fsanitize=address
debug: LDFLAGS += -fsanitize=address
debug: clean all

# Interactive testing
interactive: $(BUILD_DIR)/$(TARGET)
	@echo "Interactive mode - enter TinyC code, press Ctrl+D when done:"
	@echo "int main() { return 42; }" | ./$(BUILD_DIR)/$(TARGET) --debug-tokens /dev/stdin

# Help target
help:
	@echo "TinyC Compiler - Available targets:"
	@echo "  all              - Build the complete compiler"
	@echo "  test             - Run all unit tests"
	@echo "  test-lexer       - Run lexer unit tests"
	@echo "  test-parser      - Run parser unit tests"
	@echo "  test-semantic    - Run semantic analysis unit tests"
	@echo "  test-codegen     - Run code generation unit tests"
	@echo "  examples         - Test compiler with example programs"
	@echo "  compile-examples - Compile examples to executables"
	@echo "  debug            - Build with debug symbols and sanitizers"
	@echo "  clean            - Remove build artifacts"
	@echo "  help             - Show this help"
	@echo ""
	@echo "Usage examples:"
	@echo "  make && ./build/tcc examples/hello_world.tc"
	@echo "  make && ./build/tcc -o hello.s examples/hello_world.tc"
	@echo "  make test"
	@echo "  make examples"
	@echo "  make compile-examples"

clean:
	rm -rf $(BUILD_DIR)

# Install (for later)
install: $(BUILD_DIR)/$(TARGET)
	cp $(BUILD_DIR)/$(TARGET) /usr/local/bin/

# Quick syntax check without building
check:
	$(CC) $(CFLAGS) -fsyntax-only $(SRC_DIR)/*.c