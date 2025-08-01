# Makefile for PSCAD Co-Simulation Interface
# Compatible with GCC and other C compilers

# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
DEBUG_CFLAGS = -Wall -Wextra -std=c99 -g -DDEBUG
LDFLAGS = -lm

# Directories
SRC_DIR = .
BUILD_DIR = build
INSTALL_DIR = /usr/local

# Source files
SOURCES = pscad_cosim.c
HEADERS = pscad_cosim.h
OBJECTS = $(SOURCES:%.c=$(BUILD_DIR)/%.o)

# Target executables
TARGET = pscad_cosim_demo
DEBUG_TARGET = pscad_cosim_demo_debug

# Library targets
STATIC_LIB = libpscad_cosim.a
SHARED_LIB = libpscad_cosim.so

# Default target
all: $(TARGET)

# Debug build
debug: CFLAGS = $(DEBUG_CFLAGS)
debug: $(DEBUG_TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

# Debug executable
$(DEBUG_TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

# Static library
$(STATIC_LIB): $(BUILD_DIR)/pscad_cosim_lib.o
	ar rcs $@ $^

# Shared library
$(SHARED_LIB): $(BUILD_DIR)/pscad_cosim_lib.o
	$(CC) -shared -fPIC $^ $(LDFLAGS) -o $@

# Compile library version (without main function)
$(BUILD_DIR)/pscad_cosim_lib.o: pscad_cosim.c $(HEADERS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -fPIC -DPSCAD_COSIM_LIBRARY -c $< -o $@

# Library targets
library: $(STATIC_LIB) $(SHARED_LIB)

# Test target
test: $(TARGET)
	./$(TARGET)

# Install target
install: $(TARGET) $(STATIC_LIB) $(SHARED_LIB)
	install -d $(INSTALL_DIR)/bin
	install -d $(INSTALL_DIR)/lib
	install -d $(INSTALL_DIR)/include
	install -m 755 $(TARGET) $(INSTALL_DIR)/bin/
	install -m 644 $(STATIC_LIB) $(INSTALL_DIR)/lib/
	install -m 755 $(SHARED_LIB) $(INSTALL_DIR)/lib/
	install -m 644 $(HEADERS) $(INSTALL_DIR)/include/

# Uninstall target
uninstall:
	rm -f $(INSTALL_DIR)/bin/$(TARGET)
	rm -f $(INSTALL_DIR)/lib/$(STATIC_LIB)
	rm -f $(INSTALL_DIR)/lib/$(SHARED_LIB)
	rm -f $(INSTALL_DIR)/include/pscad_cosim.h

# Clean up build files
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET) $(DEBUG_TARGET)
	rm -f $(STATIC_LIB) $(SHARED_LIB)
	rm -f *.o *.so *.a

# Show help
help:
	@echo "Available targets:"
	@echo "  all         - Build the main executable (default)"
	@echo "  debug       - Build debug version with debug symbols"
	@echo "  library     - Build static and shared libraries"
	@echo "  test        - Build and run the demo program"
	@echo "  install     - Install to system directories"
	@echo "  uninstall   - Remove from system directories"
	@echo "  clean       - Remove all build files"
	@echo "  help        - Show this help message"
	@echo ""
	@echo "Compiler: $(CC)"
	@echo "Flags: $(CFLAGS)"
	@echo "Libraries: $(LDFLAGS)"

# Documentation target (requires doxygen)
docs:
	@if command -v doxygen >/dev/null 2>&1; then \
		doxygen Doxyfile; \
		echo "Documentation generated in docs/"; \
	else \
		echo "Doxygen not found. Install doxygen to generate documentation."; \
	fi

# Formatting target (requires clang-format)
format:
	@if command -v clang-format >/dev/null 2>&1; then \
		clang-format -i $(SOURCES) $(HEADERS); \
		echo "Code formatted with clang-format"; \
	else \
		echo "clang-format not found. Install clang-format for code formatting."; \
	fi

# Static analysis target (requires cppcheck)
analyze:
	@if command -v cppcheck >/dev/null 2>&1; then \
		cppcheck --enable=all --std=c99 $(SOURCES); \
	else \
		echo "cppcheck not found. Install cppcheck for static analysis."; \
	fi

# Valgrind memory check target
memcheck: debug
	@if command -v valgrind >/dev/null 2>&1; then \
		valgrind --leak-check=full --show-leak-kinds=all ./$(DEBUG_TARGET); \
	else \
		echo "valgrind not found. Install valgrind for memory checking."; \
	fi

# Cross-compilation targets
windows:
	$(MAKE) CC=x86_64-w64-mingw32-gcc TARGET=pscad_cosim_demo.exe

# Phony targets
.PHONY: all debug library test install uninstall clean help docs format analyze memcheck windows

# Variables for easy customization
# Override these on command line: make CC=clang CFLAGS="-O3 -march=native"
# Example: make CC=icc CFLAGS="-O3 -xHost" for Intel compiler