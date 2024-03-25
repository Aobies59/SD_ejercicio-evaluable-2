# Define the compiler
CC = gcc

# Define the flags
CFLAGS = -Wall -g

# Define the shared library location
LIB_LOCATION = ./

# Define all the source files
CLIENT_SRC = client.c
SERVER_SRC = server.c

# Define all the header files
HEADER = utils.h
NO_SHARED_LIB_HEADER = utils.h claves.h claves.c

PHONY: clean all nolib

.DEFAULT_TARGET: all

all:
	clear
	@echo "If shared library is not detected when executing ./client, compile with make nolib"
	@make --no-print-directory clean
	@make --no-print-directory server
	@make --no-print-directory client

nolib:
	clear
	@make --no-print-directory clean
	@make --no-print-directory server
	@make --no-print-directory no_shared_lib_client

# Target to build the server executable
server: $(SERVER_SRC) $(HEADER)
	@$(CC) $(CFLAGS) -o server $(SERVER_SRC) $(HEADER)

# Target to build the client executable, creating and using the shared library libclaves.so
client: $(CLIENT_SRC) $(HEADER)
	@$(CC) -c -fPIC claves.c -o claves.o 
	@$(CC) -shared claves.o -o libclaves.so
	@$(CC) $(CFLAGS) -L$(LIB_LOCATION) -Wl,-rpath=$(LIB_LOCATION) -lclaves -o client $(CLIENT_SRC) $(HEADER)

# Target to build the client executable, without creating or using the shared library
no_shared_lib_client: $(NO_SHARED_LIB_CLIENT_SRC) $(NO_SHARED_LIB_HEADER)
	@$(CC) $(CFLAGS) -o client $(CLIENT_SRC) $(NO_SHARED_LIB_HEADER)

# Clean rule to remove all object files and executables
clean:
	@rm -f server client claves.o
	@rm -f libclaves.so
	@rm -f claves.o
