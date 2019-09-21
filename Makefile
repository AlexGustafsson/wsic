# Disable echoing of commands
MAKEFLAGS += --silent

WSIC_VERSION := 0.0.1

# Build variables (such as version etc.)
BUILD_VARIABLES=-D WSIC_VERSION='"$(WSIC_VERSION)"' -D COMPILER_VERSION='"$(shell $(CC) --version | head -1)"' -D COMPILE_TIME='"$(shell LC_ALL=en_US date)"'

# Optimize the code and show all warnings (except unused parameters)
BUILD_FLAGS=-O2 -Wall -Wextra -pedantic -Wno-unused-parameter $(BUILD_VARIABLES)

# Don't optimize, provide all warnings and build with clang's memory checks and support for GDB debugging
DEBUG_FLAGS=-Wall -Wextra -pedantic -Wno-unused-parameter -fsanitize=address -fno-omit-frame-pointer -g $(BUILD_VARIABLES)

# Link towards the math library (not done by GCC)
LINKER_FLAGS=-lm

# Include generated and third-party code
INCLUDES := -Ibuild -Iincludes

# The name of the target binary
TARGET_NAME := "wsic"
DEBUG_TARGET_NAME := "wsic.debug"

# Source code
source := $(shell find src -type f -name "*.c" -not -path "src/resources/*") src/resources/resources.c
headers := $(shell find src -type f -name "*.h" -not -path "src/resources/*") src/resources/resources.h
objects := $(subst src,build,$(source:.c=.o))
# Third party includes
buildIncludes := build/includes/tomlc99/toml.o

# Test code
testSource := $(shell find test -type f -name "*.c")
testHeaders := $(shell find test -type f -name "*.h")
testIncludes := build/includes/unity/unity.o

filesToFormat := $(source) $(sourceHeaders) $(testSource) $(testHeaders)

# Resources as defined in their source form (be it html, toml etc.)
resources := $(shell find src/resources -type f -not -name "*.c" -not -name "*.h")
# Generated files for resources
resourceSources := $(subst src,build,$(resources:=.c))
resourceHeaders := $(subst src,build,$(resources:=.h))
resourceObjects := $(subst src,build,$(resources:=.o))

.PHONY: build clean debug test

# Build wsic, default action
build: build/$(TARGET_NAME)

# Build wsic with extra debugging enabled
debug: BUILD_FLAGS = $(DEBUG_FLAGS)
debug: TARGET_NAME = $(DEBUG_TARGET_NAME)
debug: CC = clang
debug: build/$(TARGET_NAME)

# Build wsic with all tests
test: source := $(filter-out src/main.c, $(source))
test: objects := $(filter-out build/main.o, $(objects))
test: BUILD_FLAGS := $(BUILD_FLAGS) --coverage
test: build/wsic.test

# Executable linking
build/$(TARGET_NAME): $(buildIncludes) $(resourceObjects) $(objects)
	$(CC) $(INCLUDES) $(BUILD_FLAGS) -o build/$(TARGET_NAME) $(buildIncludes) $(resourceObjects) $(objects) $(LINKER_FLAGS)

# Source compilation
$(objects): build/%.o: src/%.c src/%.h
	mkdir -p $(dir $@)
	$(CC) $(INCLUDES) $(BUILD_FLAGS) -c $< -o $@

# Test linking
build/wsic.test: $(buildIncludes) $(testIncludes) $(resourceObjects) $(objects) build/test/main.o $(testSource)
	$(CC) $(INCLUDES) $(BUILD_FLAGS) -o $@ $(buildIncludes) $(testIncludes) $(resourceObjects) $(objects) build/test/main.o $(LINKER_FLAGS)

# Test compilation
build/test/main.o: $(testSource)
	mkdir -p $(dir $@)
	$(CC) $(INCLUDES) $(BUILD_FLAGS) -c test/main.c -o $@

# Turn resources into c files
$(resourceSources): build/%.c: src/%
	mkdir -p $(dir $@)

	echo '#include "$(addsuffix .h, $(basename $(notdir $@)))"' > $@
	xxd -i $< >> $@
	# Null terminate
	sed -i 's/\(0x[0-9abcdef]\{2\}\)$$/\1, 0x00/g' $@
	# Replace generated names
	sed -i 's/src_resources_\([a-z0-9_]\+\)_\([a-z]\+\)\[\]/RESOURCES_\U\1_\U\2[]/g' $@
	sed -i 's/src_resources_\([a-z0-9_]\+\)_\([a-z]\+\)_len/RESOURCES_\U\1_\U\2_LENGTH/g' $@

# Turn resources into h files
$(resourceHeaders): build/%.h: build/%.c
	mkdir -p $(dir $@)

	$(eval name := $(shell echo "$@" | sed 's/[^0-9a-zA-Z]//g'))
	echo "#ifndef $(name)\n#define $(name)" > $@
	cat $< | tail -n +2 | tr -d '\n' >> $@
	echo "#endif" >> $@
	# Replace initializations with definitions
	sed -i 's/\s\+=[^;]\+;/;\n/g' $@
	# Make variables extern as they are initialized in the c source file
	sed -i 's/^\([^#]\)/extern \1/g' $@

# Turn resources into objects
$(resourceObjects): build/%.o: build/%.c build/%.h
	$(CC) $(INCLUDES) $(BUILD_FLAGS) -c $< -o $@

# Compile tomlc99
build/includes/tomlc99/toml.o: includes/tomlc99/toml.c
	mkdir -p $(dir $@)
	$(CC) -O2 -c $< -o $@

# Compile unity test framework
build/includes/unity/unity.o: includes/unity/unity.c
	mkdir -p $(dir $@)
	$(CC) -O2 -c $< -o $@

# Create the compilation database for llvm tools
compile_commands.json: Makefile
	# compiledb is installed using: pip install compiledb
	compiledb -n make

# Format code according to .clang-format
format: compile_commands.json
	clang-format -i -style=file $(filesToFormat)

# Analyze code and produce a report using the llvm tool scan-build
analyze: compile_commands.json
	scan-build --force-analyze-debug-code --keep-going -o build/reports/static-analysis make

# Lint the code according to .clang-format
lint: compile_commands.json
	./ci/lint.sh $(source)

# Build and tag the docker image
docker: $(includeSource) $(resources) $(source)
	docker build -t wsic/wsic -t wsic/wsic:$(WSIC_VERSION) -t registry.axgn.se/wsic/wsic .

clean:
	rm -rf build/*
