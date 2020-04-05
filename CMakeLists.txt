cmake_minimum_required(VERSION 3.5)

project(SudokuSolver LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(
    SudokuSolver
    Matrix.cpp
    Solver.cpp
    main.cpp)

target_compile_options(SudokuSolver PRIVATE -Wall -Wextra -pedantic)
