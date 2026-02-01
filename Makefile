# Compiler and flags
CC = g++
CFLAGS = -lutf8proc -ltinfo  -std=c++20
# Source files
SOURCES = src/core/rendering/Screen.cpp src/core/rendering/Surface.cpp src/core/rendering/Character.cpp src/test.cpp src/core/input/input.cpp src/core/widgets/Label.cpp src/core/utils/InputString.cpp src/api/api.cpp
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
	rm -rf build/*

