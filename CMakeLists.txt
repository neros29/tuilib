cmake_minimum_required(VERSION 3.15...3.28)
project(tui_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use modern Python lookup in pybind11
set(PYBIND11_FINDPYTHON NEW)

# Find pybind11 (installed via pacman/apt/pip/etc)
find_package(pybind11 CONFIG REQUIRED)

# Core engine sources
set(TUI_CORE_SOURCES
    src/core/rendering/Screen.cpp
    src/core/rendering/InternalSurface.cpp
    src/core/rendering/Character.cpp
    src/core/input/input.cpp
    src/widgets/Label.cpp
    src/core/utils/InputString.cpp
    src/api/api.cpp
    src/api/surface.cpp
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
    src/debug.cpp
    ${TUI_CORE_SOURCES}
)

# Link the same libraries to the test executable
target_link_libraries(test PRIVATE utf8proc tinfo)
