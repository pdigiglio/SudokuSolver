cmake_minimum_required(VERSION 3.8)

project(SudokuSolver LANGUAGES CXX)

# Require out-of-source build
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if (EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

#find_program(CMAKE_CXX_CPPCHECK NAMES cppcheck)
#if (CMAKE_CXX_CPPCHECK)
#    list(APPEND CMAKE_CXX_CPPCHECK
#        "--enable=all"
#        "--suppress=missingIncludeSystem"
#        "--suppress=unusedFunction"
#        "--inconclusive")
#endif()

include(CMakePrintHelpers)

find_package(Git QUIET)
if (GIT_FOUND)

    # Make sure I'm in a Git repository.
    if (EXISTS "${PROJECT_SOURCE_DIR}/.git")

        option(GIT_SUBMODULE "Check git submodules at build time" ON)
        if (GIT_SUBMODULE)

            message(STATUS "Updating git submodules")
            execute_process(
                COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                RESULT_VARIABLE GIT_SUBMODULE_RESULT)

            #cmake_print_variables(CMAKE_CURRENT_SOURCE_DIR GIT_SUBMODULE_RESULT)
            if (NOT GIT_SUBMODULE_RESULT EQUAL "0")
                message(FATAL_ERROR "Updating git submodules failed with error ${GIT_SUBMODULE_RESULT}. Please update submodules.")
            endif()

        endif()
    endif()
endif()

add_subdirectory(test)

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

target_compile_features(SudokuSolver
    PUBLIC cxx_std_14)

set_target_properties(SudokuSolver
    PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON)

#cmake_print_properties(
#    TARGETS SudokuSolver
#    PROPERTIES CXX_EXTENSIONS CXX_STANDARD_REQUIRED CXX_STANDARD)

