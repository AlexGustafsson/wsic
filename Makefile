# Optimize the code and show all warnings (except unused paramters)
BUILD_FLAGS=-O2 -Wall -Wextra -pedantic -Wno-unused-parameter

source := $(shell find src -name "*.c" -not -name "*main.c")
objects := $(subst src,build,$(source:.c=.o))

.PHONY: all directories clean
.SILENT: all directories clean build/wsic $(objects) build/main.o compile_commands.json format

all: build/wsic

# Executable linking
build/wsic: $(objects) build/main.o
	# Compile with debug information (-g)
	$(CC) $(BUILD_FLAGS) -g -o build/wsic $(objects) build/main.o

# Source compilation
$(objects): build/%.o: src/%.c src/%.h
	mkdir -p $(dir $@)
	$(CC) $(BUILD_FLAGS) -c $< -o $@
build/main.o: src/main.c
	$(CC) $(BUILD_FLAGS) -c $< -o $@

directories:
	mkdir -p build

compile_commands.json: Makefile
	# Installed using pip install compiledb
	compiledb -n make

format: compile_commands.json
	# Format code adhering to .clang-format
	clang-format -i -style=file $(source) src/main.c

clean:
	rm -rf build/*
