# Compiler and flags
CC = gcc
FLAGS = -O3 -Wall
DEBUG_FLAGS = -g  # Add debug flag for debugging
SRC = src/
OUT = bin/

# Target binaries
OBJS = $(OUT)server $(OUT)client

# Help target to display usage information
help:
	@echo
	@echo "make server       Build only the server."
	@echo "make client       Build only the client."
	@echo "make all          Build both server and client."
	@echo "make debug        Build both server and client with debug info."
	@echo

# Ensure the bin directory exists
$(OUT):
	mkdir -p $(OUT)

# Build all targets
all: $(OUT) $(OBJS)

# Build server
$(OUT)server: $(SRC)server.c
	$(CC) $(FLAGS) $< -o $@

# Build client
$(OUT)client: $(SRC)client.c
	$(CC) $(FLAGS) $< -o $@

# Clean binaries and remove the bin directory
clean:
	rm -rf $(OUT)

# Build with debug information
debug: FLAGS := $(DEBUG_FLAGS)  # Overwrite FLAGS with debug flags
debug: clean all  # Rebuild everything in debug mode
