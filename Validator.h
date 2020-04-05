#pragma once

#include "fwd/Matrix.h"

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

private:
    const SudokuGrid* Grid_;
};
