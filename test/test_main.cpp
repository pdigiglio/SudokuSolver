// This file is the test entry point.

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include "Matrix.h"
#include "Solver.h"
#include "Validator.h"

void solve_grid(const char* inputFileName)
{
    printf("Solving: '%s'\n", inputFileName);

    SudokuGrid grid;
    const auto readStatus = fill_from_input_file(inputFileName, grid);
    REQUIRE(readStatus);

    const auto inputValid = Validator(grid).validate();
    REQUIRE(inputValid);

    Solver solver(grid);
    const auto solved = solver.exec();

    if (!solved)
    {
        print_grid(grid);
    }

    printf("Inserted %u (of %u) elements in %u iteration(s).\n",
           solver.insertedDigits(),
           solver.originalNumberOfMissingDigits(),
           solver.iterations());

    CHECK(solved);
}

TEST_CASE("non-regression")
{
    SUBCASE("solve easy grid")
    {
        solve_grid("../../data/easy_input.txt");
    }

    SUBCASE("solve medium grid")
    {
        solve_grid("../../data/medium_input.txt");
    }

    SUBCASE("solve hard grid")
    {
        solve_grid("../../data/hard_input.txt");
    }

    SUBCASE("solve evil grid")
    {
        solve_grid("../../data/evil_input.txt");
    }
}

