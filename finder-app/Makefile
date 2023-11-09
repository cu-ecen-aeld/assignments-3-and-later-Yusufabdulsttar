# *********************************
# Author : Yusuf Abdulsttar
# *********************************

# the compiler: gcc for C program,or if we use specifec compiler
CC = $(CROSS_COMPILE)gcc

# compiler flags:
# -g adds debugging information to the executable file
# -Wall turns on most, but not all, compiler warnings
CFLAGS = -g -Wall

# the build target executable:
TARGET = writer

# Default target
all: $(TARGET)

$(TARGET):$(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

# removes the executable file, as well as object files
clean:
	rm -f *.o $(TARGET)
