cmake_minimum_required(VERSION 3.15...3.28)
project(tui_project LANGUAGES CXX)

# binding
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use modern Python lookup in pybind11
set(TUI_PYTHON_VERSION "3" CACHE STRING "Target Python version (e.g. 3.10 or 3.12)")
message(STATUS "Targeting Python version: ${TUI_PYTHON_VERSION}")

find_package(Python3 ${TUI_PYTHON_VERSION} REQUIRED COMPONENTS Interpreter Development)
set(pybind11_PYTHON_VERSION ${TUI_PYTHON_VERSION})
find_package(pybind11 CONFIG REQUIRED)

# Core engine sources (REUSED FOR TESTS)
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
pybind11_add_module(tuilib
    src/binding.cpp
    ${TUI_CORE_SOURCES}
)
target_link_libraries(tuilib PRIVATE utf8proc tinfo)
target_compile_definitions(tuilib PRIVATE VERSION_INFO="0.0.1")

# -------------------------
# Native C++ integration demo
# -------------------------
add_executable(integration_demo
    demos/integration_demo.cpp
    ${TUI_CORE_SOURCES}
)
target_link_libraries(integration_demo PRIVATE utf8proc tinfo)

# ---------------------------------------------------------
# NEW: Google Test Setup
# ---------------------------------------------------------

# 1. Find GoogleTest package
find_package(GTest REQUIRED)
include(GoogleTest)

# 2. Define the Unit Test executable
# We use the path you provided: tests/cpp/d_default_input.cpp
# We MUST include TUI_CORE_SOURCES so the test knows how to compile the logic.
add_executable(unit_tests_input
    tests/cpp/d_default_input.cpp
    tests/cpp/main.cpp
    ${TUI_CORE_SOURCES}
)

# 3. Link the test executable to GTest and your project dependencies
target_link_libraries(unit_tests_input
    PRIVATE
        GTest::gtest
        utf8proc
        tinfo
)

# 4. Register the test with CTest
gtest_discover_tests(unit_tests_input)
