cmake_minimum_required(VERSION 3.5)

project(SudokuSolver LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#find_program(CMAKE_CXX_CPPCHECK NAMES cppcheck)
#if (CMAKE_CXX_CPPCHECK)
#    list(APPEND CMAKE_CXX_CPPCHECK
#        "--enable=all"
#        "--suppress=missingIncludeSystem"
#        "--suppress=unusedFunction"
#        "--inconclusive")
#endif()

add_executable(
    SudokuSolver
    Matrix.cpp
    Solver.cpp
    Validator.cpp
    main.cpp)

target_compile_options(SudokuSolver
    PRIVATE
    -Wall
    -Wextra
    -Wzero-as-null-pointer-constant
    -pedantic)
