cmake_minimum_required(VERSION 3.16)
project(AoG_Solver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Release build by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_executable(main
    src/main.cpp
    src/puzzle.cpp
    src/shapes.cpp
    src/checks.cpp
    src/dfs.cpp
)

target_include_directories(main PRIVATE src)

target_compile_options(main PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>
)
