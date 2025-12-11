
cmake_minimum_required(VERSION 3.15...3.28)
project(tui_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use the modern Python lookup in pybind11
set(PYBIND11_FINDPYTHON NEW)

# Find pybind11 (installed via pacman/apt/pip/etc)
find_package(pybind11 CONFIG REQUIRED)

# Core engine sources (everything that implements Screen / Surface / Character / input)
set(TUI_CORE_SOURCES
    src/rendering/Screen.cpp
    src/rendering/Surface.cpp
    src/rendering/Character.cpp
    src/input/input.cpp
)

# Header location (tui.h etc)
# Adjust if your headers live somewhere else
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# -------------------------
# Python extension module
# -------------------------
pybind11_add_module(tui
    src/tui.cpp        # pybind11 bindings
    ${TUI_CORE_SOURCES}# engine implementation
)

target_compile_definitions(tui PRIVATE VERSION_INFO="0.0.1")

# -------------------------
# Native C++ test executable
# -------------------------
add_executable(test
    src/test.cpp       # your C++ test / demo main
    ${TUI_CORE_SOURCES}
)

# If test needs any special libs later, link them here:
# target_link_libraries(test PRIVATE some_lib)

