cmake_minimum_required(VERSION 3.16)
project(game2048 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_compile_options(-Wall -Wextra)

# Core game logic, kept free of I/O so it can be linked into the tests.
add_library(game_core
    src/board.cpp
    src/renderer.cpp
    src/input.cpp
    src/game.cpp)
target_include_directories(game_core PUBLIC src)

# The playable executable.
add_executable(2048 src/main.cpp)
target_link_libraries(2048 PRIVATE game_core)

enable_testing()
add_subdirectory(tests)
