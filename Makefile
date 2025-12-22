# Compiler and flags
CC = gcc
CFLAGS = -Wall -g -Iinclude 

# Target executable
TARGET = client

# Source files
SRCS = main.c network.c file_manager.c trigger.c util.c

# Object files
OBJS = $(SRCS:.c=.o)

# Default rule to build the program
all: $(TARGET)

# Linking object files to create the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lcurl -lpthread -lcjson

# Compiling .c files to .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ 

# Clean up generated files
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets to prevent conflicts with file names
.PHONY: all clean
