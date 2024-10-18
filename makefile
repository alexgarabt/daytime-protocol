# Compiler and flags
CC=gcc
FLAGS=-O3 -Wall

# Directories
SRC=src/
OUT=bin/

# Target to build
OBJS=$(OUT)server $(OUT)client

# Ensure bin directory exists
$(OUT):
	mkdir -p $(OUT)

# Help command
help:
	@echo
	@echo "make server 	Build only the server."
	@echo "make client 	Build only the client."
	@echo "make all     Build server and client."
	@echo

# Build all targets
all: $(OUT) $(OBJS)

# Build the server
$(OUT)server: $(SRC)server.c | $(OUT)
	$(CC) $(FLAGS) $(SRC)server.c -o $(OUT)server

# Build the client
$(OUT)client: $(SRC)client.c | $(OUT)
	$(CC) $(FLAGS) $(SRC)client.c -o $(OUT)client

# Clean binaries
clean:
	rm -rf $(OUT)*


