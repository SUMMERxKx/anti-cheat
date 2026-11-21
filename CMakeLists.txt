# Build the offline game fork, then our libraries and tools.

cmake_minimum_required(VERSION 3.20)

project(anti_cheat
    VERSION 0.1.0
    DESCRIPTION "An aimbot built only for an offline lab fork of Red Eclipse, and an anti-cheat that detects it"
    LANGUAGES C CXX)

# C99 is for the boundary layer (the lab_api.h interface and the labtrace
# telemetry library). Everything else is C++20.
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(LAB_BUILD "Build the offline lab configuration of the game fork. This repository supports nothing else." ON)

if(NOT LAB_BUILD)
    message(FATAL_ERROR
        "LAB_BUILD must stay ON. This repository only supports the offline lab build. See SAFETY.md.")
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Warning flags shared by every target we own.
add_library(ac_warnings INTERFACE)
if(MSVC)
    target_compile_options(ac_warnings INTERFACE /W4)
else()
    target_compile_options(ac_warnings INTERFACE -Wall -Wextra -Wpedantic)
endif()

# Every target we own sees LAB_BUILD as a compile definition.
add_compile_definitions(LAB_BUILD=1)

enable_testing()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/game/src/CMakeLists.txt")
    message(FATAL_ERROR "Initialize the game submodule with git submodule update --init -- game")
endif()
set(LAB_BUILD ON CACHE BOOL "Build the offline lab" FORCE)
set(LAB_MODULES_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE PATH "Lab modules" FORCE)
add_subdirectory(game/src game-build)

add_subdirectory(libs)
add_subdirectory(tools)
add_subdirectory(tests)
