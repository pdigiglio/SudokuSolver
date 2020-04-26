#pragma once

#include "fwd/SudokuGrid.h"

#include "MatrixPoint.h" // IWYU pragma: export

class Validator final
{
public:
    explicit Validator(const SudokuGrid& grid);

    Validator(const Validator&) = delete;
    Validator(Validator&&) = delete;

    Validator& operator=(const Validator&) = delete;
    Validator& operator=(Validator&&) = delete;

    ~Validator() = default;

    bool validate();

    const MatrixPoint<unsigned>& firstDuplicate() const noexcept;
    const MatrixPoint<unsigned>& secondDuplicate() const noexcept;
private:
    const SudokuGrid* Grid_ { nullptr };

    MatrixPoint<unsigned> FirstDuplicate_;
    MatrixPoint<unsigned> SecondDuplicate_;
};
