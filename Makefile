# Optimize the code and show all warnings (except unused paramters)
BUILD_FLAGS=-O2 -Wall -Wextra -pedantic -Wno-unused-parameter

source := $(shell find src -name "*.c" -not -name "*main.c")
objects := $(subst src,build,$(source:.c=.o))

.PHONY: all clean
.SILENT: all clean build build/wsic $(objects) build/main.o compile_commands.json format lint

all: build/wsic

# Executable linking
build/wsic: $(objects) build/main.o build
	# Compile with debug information (-g)
	$(CC) $(BUILD_FLAGS) -g -o build/wsic $(objects) build/main.o

# Source compilation
$(objects): build/%.o: src/%.c src/%.h build
	mkdir -p $(dir $@)
	$(CC) $(BUILD_FLAGS) -c $< -o $@
build/main.o: src/main.c build
	$(CC) $(BUILD_FLAGS) -c $< -o $@

build:
	mkdir -p build

compile_commands.json: Makefile
	# Installed using pip install compiledb
	compiledb -n make

format: compile_commands.json
	# Format code adhering to .clang-format
	clang-format -i -style=file $(source) src/main.c

analyze: compile_commands.json
	# Analyze code and produce a report using the llvm tool scan-build
	scan-build --keep-going -o build/reports/static-analysis make

lint: compile_commands.json
	./ci/lint.sh $(source) src/main.c

clean:
	rm -rf build/*
