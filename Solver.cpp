#include "Solver.h"

#include <algorithm>
#include <iterator>
#include <utility>

namespace
{
    unsigned count_empty_cells(const SudokuGrid& grid) noexcept
    {
        const auto first = std::cbegin(grid);
        const auto last = std::cend(grid);
        return std::count_if(first, last, [](const SudokuGrid::value_type c) { return is_empty(c); });
    }
}

Solver::Solver(SudokuGrid& grid)
    :
      Grid_(std::addressof(grid)),
      NumberOfMissingDigits_(count_empty_cells(grid))
{ }

unsigned Solver::insertedDigits() const
{
    return this->InsertedDigits_;
}

unsigned Solver::originalNumberOfMissingDigits() const
{
    return this->NumberOfMissingDigits_;
}
