
add_executable(test
    EXCLUDE_FROM_ALL
    test_SudokuSolver.cpp)

target_include_directories(test
    PRIVATE "${PROJECT_SOURCE_DIR}/external/doctest")

#include(CTest)
#
#add_test(
#    NAME test
#    COMMAND test)

#cmake_print_properties(
#    TARGETS test
#    PROPERTIES INCLUDE_DIRECTORIES)
