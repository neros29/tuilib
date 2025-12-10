# Compiler and flags
CC = g++
CFLAGS =   # Example: warning all, C++17 standard, optimization
# Source files
SOURCES = src/rendering/Screen.cpp src/rendering/Surface.cpp src/rendering/Character.cpp src/test.cpp src/input/input.cpp
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
