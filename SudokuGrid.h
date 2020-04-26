#pragma once

#include "Matrix.h" // IWYU pragma: export

class SudokuGrid final : public Matrix<char, 9, 9>
{
public:
    static constexpr unsigned sideLength() noexcept
    {
        static_assert (rows() == columns(), "Grid is not square" );
        return rows();
    }
};

constexpr bool is_empty(typename SudokuGrid::value_type cell) noexcept
{
    return 0 == cell;
}

bool fill_from_input_file(const char* filePath, SudokuGrid& grid);
void print_grid(const SudokuGrid& grid);
