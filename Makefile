# Compiler and flags
CC = g++
CFLAGS =   # Example: warning all, C++17 standard, optimization
# Source files
SOURCES = src/Screen.cpp src/Surface.cpp src/Character.cpp src/test.cpp
OBJECTS = $(SOURCES:.cpp=.o)  # Automatic object file names
# Executable name
EXECUTABLE = build/test
# Default target
all: $(EXECUTABLE)
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS)-o $(EXECUTABLE)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS)
