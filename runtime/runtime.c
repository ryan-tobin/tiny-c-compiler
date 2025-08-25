// runtime/runtime.c
// Simple runtime library for TinyC programs

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Print a string (for our TinyC programs)
void print(char* str) {
    if (str) {
        write(1, str, strlen(str));  // stdout
        write(1, "\n", 1);          // newline
    }
}

// Print an integer
void print_int(int n) {
    char buffer[32];
    sprintf(buffer, "%d", n);
    write(1, buffer, strlen(buffer));
    write(1, "\n", 1);
}

// Print a character
void print_char(char c) {
    write(1, &c, 1);
}

// Simple input function (reads a line)
int read_int() {
    char buffer[32];
    int bytes_read = read(0, buffer, sizeof(buffer) - 1);  // stdin
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        return atoi(buffer);
    }
    return 0;
}