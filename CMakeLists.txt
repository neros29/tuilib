cmake_minimum_required(VERSION 3.15...3.28)
project(tui_project LANGUAGES CXX)
# --- Global Settings ---
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Python setup
set(TUI_PYTHON_VERSION "3" CACHE STRING "Target Python version")
message(STATUS "Targeting Python version: ${TUI_PYTHON_VERSION}")

find_package(Python3 ${TUI_PYTHON_VERSION} REQUIRED COMPONENTS Interpreter Development)
set(pybind11_PYTHON_VERSION ${TUI_PYTHON_VERSION})
find_package(pybind11 CONFIG REQUIRED)

# --- 1. Define the Core Library ---

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

# IMPORTANT: You must add the library BEFORE you can set properties on it
add_library(tui_core STATIC ${TUI_CORE_SOURCES})

# Set include directories (PUBLIC means anyone linking to tui_core gets these)
target_include_directories(tui_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link external dependencies to the CORE (PUBLIC)
# By using PUBLIC, tuilib, integration_demo, and unit_tests will
# "inherit" utf8proc and tinfo automatically.
target_link_libraries(tui_core PUBLIC utf8proc tinfo)


# --- 2. Python Extension Module ---

pybind11_add_module(tuilib src/binding.cpp)

# We ONLY need to link to tui_core.
# CMake handles the utf8proc/tinfo part for us!
target_link_libraries(tuilib PRIVATE tui_core)
target_compile_definitions(tuilib PRIVATE VERSION_INFO="0.0.1")


# --- 3. Native C++ Integration Demo ---

add_executable(integration_demo demos/integration_demo.cpp)

# We ONLY need to link to tui_core.
target_link_libraries(integration_demo PRIVATE tui_core)


# --- 4. Google Test Setup ---

find_package(GTest REQUIRED)
include(GoogleTest)

# Define the Unit Test executable
add_executable(unit_tests_input
    tests/cpp/d_default_input.cpp
    tests/cpp/main.cpp
)

# We ONLY need to link GTest and tui_core.
# Everything else is inherited from tui_core.
target_link_libraries(unit_tests_input
    PRIVATE
        GTest::gtest_main
        tui_core
)

gtest_discover_tests(unit_tests_input)
