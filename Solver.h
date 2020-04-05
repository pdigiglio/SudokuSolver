#pragma once

#include "Matrix.h"
#include "StaticVector.h"

#include <array>

class Solver final
{
public:
    explicit Solver(SudokuGrid& grid);

    bool exec();

    unsigned insertedDigits() const;
    unsigned originalNumberOfMissingDigits() const;
    unsigned iterations() const;

    using cell_digit_collection = StaticVector<SudokuGrid::value_type, SudokuGrid::rows()>;
    using candidate_digit_collection = Matrix<cell_digit_collection, SudokuGrid::rows(), SudokuGrid::columns()>;

private:

    candidate_digit_collection CandidateDigits_;
    SudokuGrid* Grid_ = nullptr;

    unsigned InsertedDigits_ = 0;
    unsigned Iterations_ = 0;
    unsigned NumberOfMissingDigits_ = 0;
};
