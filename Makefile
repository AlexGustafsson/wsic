# Disable echoing of commands
MAKEFLAGS += --silent

# Build variables (such as version etc.)
BUILD_VARIABLES=-DWSIC_VERSION='"0.0.1"' -DCOMPILER_VERSION='"$(shell $(CC) --version | head -1)"' -DCOMPILE_TIME='"$(shell LC_ALL=en_US date)"'
# Optimize the code and show all warnings (except unused parameters)
BUILD_FLAGS=-O2 -Wall -Wextra -pedantic -Wno-unused-parameter $(BUILD_VARIABLES)
# Don't optimize, provide all warnings and build with clang's memory checks and support for GDB debugging
DEBUG_FLAGS=-Wall -Wextra -pedantic -Wno-unused-parameter -fsanitize=address -fno-omit-frame-pointer -g $(BUILD_VARIABLES)
# Include generated code
INCLUDES=-Isrc -Ibuild

source := $(shell find src -type f -name "*.c" -not -name "/resources/*/*")
objects := $(subst src,build,$(source:.c=.o))

resources := $(shell find src/resources -type f -not -name "*.c" -not -name "*.h")
resourceHeaders := $(subst src,build,$(resources:=.h))

.PHONY: build clean debug directories

# Build wsic, default action
build: build/wsic

# Build wsic with extra debugging enabled
debug: build/wsic.debug

# Executable linking
build/wsic: $(resourceHeaders) $(objects)
	$(CC) $(INCLUDES) $(BUILD_FLAGS) -o build/wsic $(objects)

# Source compilation
$(objects): build/%.o: src/%.c src/%.h
	mkdir -p $(dir $@)
	$(CC) $(INCLUDES) $(BUILD_FLAGS) -c $< -o $@

# Source compilation with debugging enabled
build/wsic.debug: $(source) $(resourceHeaders)
	ASAN_OPTIONS=detect_leaks=1 clang $(INCLUDES) $(DEBUG_FLAGS) -o build/wsic.debug $(source)

# Turn resources into objects
$(resourceHeaders): build/%.h: src/%
	mkdir -p $(dir $@)


	# Include guards
	$(eval name := $(shell echo "$@" | sed 's/[\/\.]//g'))
	echo "#ifndef $(name)\n#define $(name)" > $@
	xxd -i $< >> $@
	echo "#endif" >> $@
	# Null terminate
	sed -i 's/\(0x[0-9abcdef]\{2\}\)$$/\1, 0x00/g' $@
	# Replace generated names
	sed -i 's/src_resources_\([a-z0-9_]\+\)_html\[\]/RESOURCES_\U\1_HTML[]/g' $@
	sed -i 's/src_resources_\([a-z0-9_]\+\)_html_len/RESOURCES_\U\1_HTML_LENGTH/g' $@


# Create the compilation database for llvm tools
compile_commands.json: Makefile
	# compiledb is installed using: pip install compiledb
	compiledb -n make

# Format code according to .clang-format
format: compile_commands.json
	clang-format -i -style=file $(source)

# Analyze code and produce a report using the llvm tool scan-build
analyze: compile_commands.json
	scan-build --keep-going -o build/reports/static-analysis make

# Lint the code according to .clang-format
lint: compile_commands.json
	./ci/lint.sh $(source)

clean:
	rm -rf build/*
