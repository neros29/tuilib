cmake_minimum_required(VERSION 3.15...3.28)
project(tui_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use modern Python lookup in pybind11
# 1. Define a variable for the target Python version.
# This can be passed via command line: cmake -DTUI_PYTHON_VERSION=3.10 ..
set(TUI_PYTHON_VERSION "3" CACHE STRING "Target Python version (e.g. 3.10 or 3.12)")

message(STATUS "Targeting Python version: ${TUI_PYTHON_VERSION}")

# 2. Find the specific Python version first.
# This tells CMake which Python interpreter and headers we want to use.
find_package(Python3 ${TUI_PYTHON_VERSION} REQUIRED COMPONENTS Interpreter Development)

# 3. Force pybind11 to use the Python we just found.
# We tell pybind11 where the Python include directories and libraries are.
set(pybind11_PYTHON_VERSION ${TUI_PYTHON_VERSION})
find_package(pybind11 CONFIG REQUIRED)

# Core engine sources
set(TUI_CORE_SOURCES
	src/core/rendering/Screen.cpp
	src/core/rendering/c_surface.cpp
	src/core/rendering/c_cell.cpp
	src/core/impl/c_surfaces.cpp
	src/core/utils/InputString.cpp
	src/core/c_eventBus.cpp
	src/drivers/d_default_input.cpp
	src/api/api.cpp
	src/api/surface.cpp
	src/widgets/Label.cpp
)

# Header location
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/tui)

# -------------------------
# Python extension module
# -------------------------
pybind11_add_module(tui
    src/tui.cpp
    ${TUI_CORE_SOURCES}
)

# Link libraries to the Python module
target_link_libraries(tui PRIVATE utf8proc tinfo)
target_compile_definitions(tui PRIVATE VERSION_INFO="0.0.1")

# -------------------------
# Native C++ test executable
# -------------------------
add_executable(test
    src/test.cpp
    ${TUI_CORE_SOURCES}
)

# Link the same libraries to the test executable
target_link_libraries(test PRIVATE utf8proc tinfo)
