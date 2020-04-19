#pragma once

#include "fwd/SudokuGrid.h"

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
    const char* type() const;
    int index() const;

private:
    const SudokuGrid* Grid_;

    // either "row" or "col"
    const char* DuplicateType_ = "";
    int DuplicateIndex_ = -1;
};
