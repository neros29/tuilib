# Compiler
CXX = g++

# Flags
CXXFLAGS = -std=c++20 -MMD -MP -O0
LDFLAGS  = -lutf8proc -ltinfo

# Source files
SOURCES = \
	src/core/rendering/Screen.cpp \
	src/core/rendering/Surface.cpp \
	src/core/rendering/Character.cpp \
	src/core/input/input.cpp \
	src/core/widgets/Label.cpp \
	src/core/utils/InputString.cpp \
	src/api/api.cpp \
	src/debug.cpp

# Object / dependency files
OBJECTS = $(SOURCES:.cpp=.o)
DEPS    = $(OBJECTS:.o=.d)

# Executable
EXECUTABLE = build/test

# Default target
all: $(EXECUTABLE)

# Link
$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $(EXECUTABLE)

# Compile
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Pull in header dependencies
-include $(DEPS)

# Clean
clean:
	rm -f $(OBJECTS) $(DEPS)
	rm -rf build/*
