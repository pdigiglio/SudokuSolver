#include "Matrix.h"
#include "Solver.h"
#include "Validator.h"

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

    Solver solver(grid);
    solver.exec();

    puts("Solved:");
    print_grid(grid);

    printf("Inserted %u (of %u) elements in %u iteration(s).\n",
           solver.insertedDigits(),
           solver.originalNumberOfMissingDigits(),
           solver.iterations());

    printf("Solution validation [1: success, 0: failure]: %d.\n",
           Validator(grid).validate());

    return 0;
}
