#include "Matrix.h"
#include "Solver.h"
#include "Validator.h"

#include <cstdio>

void print_validation_status(bool status)
{
    auto output = status ? stdout : stderr;
    fprintf(output, "Validation [1: success, 0: failure]: %d.\n", status);
}

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

    const auto inputValid = Validator(grid).validate();
    print_validation_status(inputValid);
    if (!inputValid)
        return 1;

    Solver solver(grid);
    solver.exec();

    puts("");
    puts("Solved:");
    print_grid(grid);

    printf("Inserted %u (of %u) elements in %u iteration(s).\n",
           solver.insertedDigits(),
           solver.originalNumberOfMissingDigits(),
           solver.iterations());

    print_validation_status(Validator(grid).validate());
    return 0;
}
