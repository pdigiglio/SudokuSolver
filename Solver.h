#pragma once

#include "SudokuGrid.h"

class Solver
{
public:
    explicit Solver(SudokuGrid& grid);

    Solver(const Solver&) = delete;
    Solver(Solver&&) = delete;

    Solver& operator=(const Solver&) = delete;
    Solver& operator=(Solver&&) = delete;

    virtual ~Solver() = default;

    virtual bool exec() = 0;

    unsigned insertedDigits() const;
    unsigned originalNumberOfMissingDigits() const;

protected:
    SudokuGrid* const Grid_ = nullptr;

    unsigned InsertedDigits_ = 0;
    const unsigned NumberOfMissingDigits_ = 0;
};
