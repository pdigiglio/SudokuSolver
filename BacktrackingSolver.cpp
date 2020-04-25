#include "BacktrackingSolver.h"

#include <cassert>
#include <cstddef>

BacktrackingSolver::BacktrackingSolver(SudokuGrid& grid)
    : Grid_(&grid)
{ }

namespace
{

bool row_contains_value(
        const SudokuGrid& grid,
        char value,
        size_t row)
{
    for (unsigned c = 0; c < grid.columns(); ++c)
    {
        if (value == grid[row][c])
            return true;
    }

    return false;
}

bool column_contains_value(
        const SudokuGrid& grid,
        char value,
        size_t column)
{
    for (unsigned r = 0; r < grid.rows(); ++r)
    {
        if (value == grid[r][column])
            return true;
    }

    return false;
}

bool subgrid_contains_value(
        const SudokuGrid& grid,
        char value,
        size_t row,
        size_t column)
{
    constexpr auto subgridSideLength = 3;
    const auto rowStart = subgridSideLength * (row / subgridSideLength);
    const auto rowEnd = rowStart + subgridSideLength;

    const auto columnStart = subgridSideLength * (column / subgridSideLength);
    const auto columnEnd = columnStart + subgridSideLength;

    for (unsigned r = rowStart; r < rowEnd; ++r)
    {
        for (unsigned c = columnStart; c < columnEnd; ++c)
        {
            if (value == grid[r][c])
                return true;
        }
    }

    return false;
}

bool is_possible(
        const SudokuGrid& grid,
        char value,
        size_t row,
        size_t column)
{
    return  !row_contains_value(grid, value, row) &&
            !column_contains_value(grid, value, column) &&
            !subgrid_contains_value(grid, value, row, column);
}

void solve_impl(
        SudokuGrid& grid,
        const unsigned missing,
        unsigned& inserted,
        const unsigned rowStart = 0)
{
    assert(rowStart < grid.rows());
    for (unsigned r = rowStart; r < grid.rows(); ++r)
    {
        for (unsigned c = 0; c < grid.columns(); ++c)
        {
            if (0 == grid[r][c])
            {
                for (unsigned i = 1; i < 10; ++i)
                {
                    if (is_possible(grid, static_cast<char>(i), r, c))
                    {
                        grid[r][c] = static_cast<char>(i);
                        ++inserted;

                        solve_impl(grid, missing, inserted, r);
                        if (inserted < missing)
                        {
                            grid[r][c] = 0;
                            --inserted;
                        }
                        else
                        {
                            return;
                        }
                    }
                }

                return;
            }
        }
    }
    return;
}

unsigned count_missing(const SudokuGrid& grid)
{
    unsigned missing = 0;
    for (unsigned r = 0; r < grid.rows(); ++r)
    {
        for (unsigned c = 0; c < grid.columns(); ++c)
        {
            missing += static_cast<char>(0) == grid[r][c];
        }
    }

    return missing;
}

}

bool BacktrackingSolver::exec()
{
    const auto missing = count_missing(*this->Grid_);
    unsigned inserted = 0;
    solve_impl(*this->Grid_, missing, inserted);
    this->InsertedDigits_ = missing;
    return true;
}

bool BacktrackingSolver::insertedDigits() const
{
    return this->InsertedDigits_;
}
