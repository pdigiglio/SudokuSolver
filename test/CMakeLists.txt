enable_testing()

add_executable(test_main
    EXCLUDE_FROM_ALL
    test_main.cpp)

target_include_directories(test_main
    PRIVATE "${PROJECT_SOURCE_DIR}/external/doctest")

target_link_libraries(test_main PRIVATE SudokuSolverLib)

add_test(
    NAME test_all
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/test"
    COMMAND test_main)

cmake_print_properties(
    TARGETS test_main
    PROPERTIES INCLUDE_DIRECTORIES)
