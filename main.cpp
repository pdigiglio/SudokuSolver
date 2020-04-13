#include "Matrix.h"
#include "Solver.h"
#include "BacktrackingSolver.h"
#include "Validator.h"

#include <cstdio>
#include <chrono>

bool print_validation_status(const SudokuGrid& grid)
{
    Validator validator(grid);
    const auto status = validator.validate();
    const auto output = status ? stdout : stderr;
    fprintf(output, "Validation [1: success, 0: failure]: %d.\n", status);
    if (!status)
    {
        fprintf(output, "Duplicate at %s index %d\n", validator.type(), validator.index());
    }

    return status;
}

template <typename T>
class TD;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: SudokuSolver \"input file\"\n");
        return 1;
    }

    SudokuGrid grid;
    const auto readStatus = fill_from_input_file(argv[1], grid);
    if (!readStatus)
        return 1;

    print_grid(grid);

    const auto inputValid = print_validation_status(grid);
    if (!inputValid)
        return 1;

    const auto start = std::chrono::steady_clock::now();
    //Solver solver(grid);
    BacktrackingSolver solver(grid);
    solver.exec();
    const auto end = std::chrono::steady_clock::now();

    puts("");
    puts("Solved:");
    print_grid(grid);

    printf("Inserted %u (of %u) elements in %u iteration(s).\n",
           solver.insertedDigits(), 0, 0);

    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    printf("Solution took %ld ms.\n", elapsed.count());

    //printf("Inserted %u (of %u) elements in %u iteration(s).\n",
    //       solver.insertedDigits(),
    //       solver.originalNumberOfMissingDigits(),
    //       solver.iterations());

    print_validation_status(grid);
    return 0;
}
