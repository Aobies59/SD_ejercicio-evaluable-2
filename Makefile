# Define the compiler
CC = gcc

# Define the flags
CFLAGS = -Wall -g
LDFLAGS = -lrt -pthread

#modify the LD_LIBRARY_PATH environment variable
LD_LIBRARY_PATH += .

# Define the shared library location
LIB_LOCATION = ./

# Define all the source files
CLIENT_SRC = client.c
SERVER_SRC = server.c

# Define all the header files
HEADER = utils.h
NO_SHARED_LIB_HEADER = utils.h claves.h claves.c

PHONY: clean all

.DEFAULT_TARGET: all

all:
	@make --no-print-directory clean
	@make --no-print-directory server
	@echo "compiled server"
	@make --no-print-directory client
	@echo "compiled client"

# Target to build the server executable
server: $(SERVER_SRC) $(HEADER)
	@$(CC) $(CFLAGS) -o server $(SERVER_SRC) $(HEADER) $(LDFLAGS)

# Target to build the client executable, creating and using the shared library libclaves.so
client: $(CLIENT_SRC) $(HEADER)
	@$(CC) -shared -fPIC claves.c -o libclaves.so $(CFLAGS) $(LDFLAGS)
	@$(CC) $(CFLAGS) -o client $(CLIENT_SRC) $(HEADER) $(LDFLAGS) -L$(LIB_LOCATION) -Wl,-rpath=./ -lclaves

# Clean rule to remove all object files and executables
clean:
	@rm -f server client libclaves.so
