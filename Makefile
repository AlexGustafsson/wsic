# Disable echoing of commands
MAKEFLAGS += --silent

# Optimize the code and show all warnings (except unused parameters)
BUILD_FLAGS=-O2 -Wall -Wextra -pedantic -Wno-unused-parameter
# Don't optimize, provide all warnings and build with clang's memory checks and support for GDB debugging
DEBUG_FLAGS=-Wall -Wextra -pedantic -Wno-unused-parameter -fsanitize=address -fno-omit-frame-pointer -g

source := $(shell find src -name "*.c" -not -name "*main.c")
objects := $(subst src,build,$(source:.c=.o))

.PHONY: build clean debug directories

# Build wsic, default action
build: build/wsic

# Build wsic with extra debugging enabled
debug: build/wsic.debug

# Executable linking
build/wsic: $(objects) build/main.o
	$(CC) $(BUILD_FLAGS) -o build/wsic $(objects) build/main.o

# Source compilation
$(objects): build/%.o: src/%.c src/%.h
	mkdir -p $(dir $@)
	$(CC) $(BUILD_FLAGS) -c $< -o $@
build/main.o: src/main.c
	$(CC) $(BUILD_FLAGS) -c $< -o $@

# Source compilation with debugging enabled
build/wsic.debug: $(source) src/main.c
	ASAN_OPTIONS=detect_leaks=1 clang $(DEBUG_FLAGS) -o build/wsic.debug $(source) src/main.c

# Create the compilation database for llvm tools
compile_commands.json: Makefile
	# compiledb is installed using: pip install compiledb
	compiledb -n make

# Format code according to .clang-format
format: compile_commands.json
	clang-format -i -style=file $(source) src/main.c

# Analyze code and produce a report using the llvm tool scan-build
analyze: compile_commands.json
	scan-build --keep-going -o build/reports/static-analysis make

# Lint the code according to .clang-format
lint: compile_commands.json
	./ci/lint.sh $(source) src/main.c

clean:
	rm -rf build/*
