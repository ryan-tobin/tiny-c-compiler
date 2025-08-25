CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -g -O0 -DDEBUG
LDFLAGS = 
TARGET = tcc
SRC_DIR = src
TEST_DIR = tests
EXAMPLE_DIR = examples
BUILD_DIR = build

# Source files (only lexer for now)
LEXER_SOURCES = $(SRC_DIR)/lexer.c $(SRC_DIR)/main.c
LEXER_OBJECTS = $(LEXER_SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Test files
TEST_LEXER_SOURCES = $(TEST_DIR)/unit/test_lexer.c $(SRC_DIR)/lexer.c
TEST_LEXER_OBJECTS = $(BUILD_DIR)/tests/unit/test_lexer.o $(BUILD_DIR)/lexer.o

.PHONY: all clean test test-lexer examples debug help

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

# Link main executable (lexer only for now)
$(BUILD_DIR)/$(TARGET): $(LEXER_OBJECTS) | $(BUILD_DIR)
	$(CC) $(LEXER_OBJECTS) -o $@ $(LDFLAGS)

# Test targets
test: test-lexer

test-lexer: $(BUILD_DIR)/test_lexer
	@echo "Running lexer unit tests..."
	./$(BUILD_DIR)/test_lexer

$(BUILD_DIR)/test_lexer: $(TEST_LEXER_OBJECTS) | $(BUILD_DIR)
	$(CC) $(TEST_LEXER_OBJECTS) -o $@ $(LDFLAGS)

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
	@echo "  all          - Build the compiler (lexer phase)"
	@echo "  test         - Run all unit tests"
	@echo "  test-lexer   - Run lexer unit tests"
	@echo "  examples     - Test lexer with example programs"
	@echo "  debug        - Build with debug symbols and sanitizers"
	@echo "  clean        - Remove build artifacts"
	@echo "  help         - Show this help"
	@echo ""
	@echo "Usage examples:"
	@echo "  make && ./build/tcc examples/hello_world.tc"
	@echo "  make test"
	@echo "  make examples"

clean:
	rm -rf $(BUILD_DIR)

# Install (for later)
install: $(BUILD_DIR)/$(TARGET)
	cp $(BUILD_DIR)/$(TARGET) /usr/local/bin/

# Quick syntax check without building
check:
	$(CC) $(CFLAGS) -fsyntax-only $(SRC_DIR)/*.c