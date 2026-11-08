# Compiler
CXX = g++

# Flags
# CXXFLAGS = -fsanitize=address -std=c++20 -MMD -MP -O0 -g -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror
# LDFLAGS  = -lutf8proc -ltinfo -fsanitize=address
CXXFLAGS = -std=c++20 -MMD -MP -O3 -g
LDFLAGS  = -lutf8proc -ltinfo

# Source files
SOURCES = \
	src/core/rendering/Screen.cpp \
	src/core/impl/c_surfaces.cpp \
	src/core/rendering/c_surface.cpp \
	src/core/rendering/c_cell.cpp \
	src/widgets/Label.cpp \
	src/core/utils/InputString.cpp \
	src/core/c_eventBus.cpp\
	src/drivers/d_default_input.cpp \
	src/api/api.cpp \
	src/api/surface.cpp\
	src/test.cpp

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
