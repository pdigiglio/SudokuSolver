#pragma once

#include "Matrix.h"

class BacktrackingSolver
{
public:
    explicit BacktrackingSolver(SudokuGrid& grid);
    bool exec();

    bool insertedDigits() const;
private:
    SudokuGrid* Grid_ = nullptr;
    unsigned InsertedDigits_ = 0;
};
