#pragma once

#include "fwd/SudokuGrid.h" // IWYU pragma: keep
// IWYU pragma: no_include "SudokuGrid.h"

#include "Matrix.h" // IWYU pragma: keep
#include "Solver.h"
#include "StaticVector.h"

class ConstrainSolver final : public Solver
{
public:
    explicit ConstrainSolver(SudokuGrid& grid);

    bool exec() override;
    unsigned iterations() const;

    using candidate_collection = StaticVector<SudokuGrid::value_type, SudokuGrid::rows()>;
    using candidate_grid = Matrix<candidate_collection, SudokuGrid::rows(), SudokuGrid::columns()>;

private:
    candidate_grid CandidateGrid_;
    unsigned Iterations_ = 0;
};
