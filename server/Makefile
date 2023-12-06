# *********************************
# Author : Yusuf Abdulsttar
# *********************************

# the compiler: gcc for C program,or if we use specifec compiler
CC ?= $(CROSS_COMPILE)gcc

# compiler flags:
# -g adds debugging information to the executable file
# -Wall turns on most, but not all, compiler warnings
CFLAGS ?= -g -Wall -Werror

# the build target executable:
TARGET ?= aesdsocket

LDFLAGS ?= -lpthread -lrt

# Default target
all: $(TARGET)

$(TARGET):$(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c $(LDFLAGS)

# removes the executable file, as well as object files
clean:
	rm -f *.o $(TARGET)
