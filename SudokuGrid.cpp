#include "SudokuGrid.h"

#include "constexpr_functions.h"

#include <cassert>
#include <cstdio>
#include <fstream>

bool fill_from_input_file(const char* filePath, SudokuGrid& grid)
{
    std::ifstream inFile (filePath);
    if (!inFile.is_open())
    {
        fprintf(stderr, "Invalid input file '%s'\n", filePath);
        return false;
    }

    for (size_t r = 0; r < SudokuGrid::sideLength(); ++r)
    {
        for (size_t c = 0; c < SudokuGrid::sideLength(); ++c)
        {
            char buffer = 0;
            inFile >> buffer;

            if (buffer < '0' || buffer > '9')
            {
                fprintf(stderr, "Invalid input: '%c' at (%zu, %zu)\n", buffer, r, c);
                return false;
            }

            //grid[r][c] = static_cast<char>(buffer - '0');
            //grid[r][c] = buffer - 48;
            grid[r][c] = buffer - '0';
        }
    }

    return true;
}

namespace
{

void print_row(const SudokuGrid& grid, size_t row)
{
    assert(row < SudokuGrid::rows());
    printf("%c  %c  %c | %c  %c  %c | %c  %c  %c\n",
            grid[row][0] + '0', // NOLINT(cppcoreguidelines-avoid-magic-numbers)
            grid[row][1] + '0', // NOLINT(cppcoreguidelines-avoid-magic-numbers)
            grid[row][2] + '0', // NOLINT(cppcoreguidelines-avoid-magic-numbers)
            grid[row][3] + '0', // NOLINT(cppcoreguidelines-avoid-magic-numbers)
            grid[row][4] + '0', // NOLINT(cppcoreguidelines-avoid-magic-numbers)
            grid[row][5] + '0', // NOLINT(cppcoreguidelines-avoid-magic-numbers)
            grid[row][6] + '0', // NOLINT(cppcoreguidelines-avoid-magic-numbers)
            grid[row][7] + '0', // NOLINT(cppcoreguidelines-avoid-magic-numbers)
            grid[row][8] + '0'); // NOLINT(cppcoreguidelines-avoid-magic-numbers)

}

void print_rows(const SudokuGrid& grid, size_t rowStart, size_t rowEnd)
{
    for (size_t r = rowStart; r < rowEnd; ++r)
    {
        print_row(grid, r);
    }
}

void print_row_separator()
{
    puts("---------------------------");
}

}

void print_grid(const SudokuGrid& grid)
{
    constexpr auto subgridSideLength = Sqrt<SudokuGrid::sideLength()>::value;

    auto rStart = 0;
    auto rEnd =  subgridSideLength;
    for (size_t sgr = 0; sgr < subgridSideLength - 1; ++sgr)
    {
        print_rows(grid, rStart, rEnd);
        print_row_separator();

        rStart += subgridSideLength;
        rEnd += subgridSideLength;
    }

    print_rows(grid, rStart, rEnd);
}
