# Compiler settings
CC = gcc
CFLAGS = -Wall -g

# Default target
all: build_dirs Binaries/lex Binaries/parser

# Create the Binaries directory before compiling
build_dirs:
	@mkdir -p Binaries

# Compile lexer into the Binaries folder
Binaries/lex: lexer.c
	$(CC) $(CFLAGS) -o Binaries/lex lexer.c

# Compile parser into the Binaries folder
Binaries/parser: parser.c
	$(CC) $(CFLAGS) -o Binaries/parser parser.c

# Run target: Ensures output directories exist, then runs the executables
run: all
	@mkdir -p Lexer Parser
	./Binaries/lex t1.mc
	./Binaries/parser

# Clean up generated files, binaries, and directories
clean:
	rm -rf Binaries/ Lexer/ Parser/

.PHONY: all build_dirs run clean