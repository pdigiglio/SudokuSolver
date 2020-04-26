#include "ConstrainSolver.h"

#include "MatrixPoint.h"
#include "StaticVector.h"
#include "SudokuGrid.h"
#include "constexpr_functions.h"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <vector>

namespace
{

template <typename Container, typename T>
bool contains(Container& container, const T value)
{
    auto last = std::end(container);
    return last != std::find(std::begin(container), last, value);
}

template <typename It, typename Container>
constexpr void append_unique_non_empty_values(It begin, It end, Container& digits)
{
    for (; begin != end; ++begin)
    {
        const auto value = *begin;
        if (!is_empty(value) && !contains(digits, value))
        {
            digits.push_back(value);
        }
    }
}

constexpr auto SudokuSubgridSide = 3;

template <typename It>
struct Range
{
    It Begin;
    It End;
};

template <typename It>
constexpr Range<It> make_range(It begin, It end)
{
    return { begin, end };
}

template <typename Grid>
constexpr auto sudoku_subgrid_range(Grid& grid, unsigned row, unsigned col)
{
    static_assert (Grid::rows() == SudokuGridSide, "Not a SudokuGrid" );
    static_assert (Grid::columns() == SudokuGridSide, "Not a SudokuGrid" );

    const auto rowStart = SudokuSubgridSide * (row / SudokuSubgridSide);
    const auto colStart = SudokuSubgridSide * (col / SudokuSubgridSide);

    return make_range(
        grid.template subgrid_begin<SudokuSubgridSide, SudokuSubgridSide>(rowStart, colStart),
        grid.template subgrid_end<SudokuSubgridSide, SudokuSubgridSide>(rowStart, colStart));
}

constexpr auto sudoku_subgrid_crange(const SudokuGrid& grid, unsigned row, unsigned col)
{
    return sudoku_subgrid_range(grid, row, col);
}

void append_row_digits(
        const SudokuGrid& grid,
        unsigned row,
        ConstrainSolver::candidate_collection& digits)
{
    append_unique_non_empty_values(grid.row_begin(row), grid.row_end(row), digits);
}

void append_column_digits(
        const SudokuGrid& grid,
        unsigned column,
        ConstrainSolver::candidate_collection& digits)
{
    append_unique_non_empty_values(grid.column_begin(column), grid.column_end(column), digits);
}

void append_subgrid_digits(
        const SudokuGrid& grid,
        unsigned row,
        unsigned column,
        ConstrainSolver::candidate_collection& digits)
{
    auto range = sudoku_subgrid_crange(grid, row, column);
    append_unique_non_empty_values(range.Begin, range.End, digits);
}

std::vector<SudokuGrid::value_type> get_candidates(ConstrainSolver::candidate_collection& forbiddenDigits)
{
    constexpr auto capacity = ConstrainSolver::candidate_collection::capacity();

    // Sort the input vector.
    std::sort(std::begin(forbiddenDigits), std::end(forbiddenDigits));

    std::vector<SudokuGrid::value_type> missing;
    missing.reserve(capacity - forbiddenDigits.size());

    constexpr auto iBegin = 1;

    unsigned i = iBegin;

    {
        unsigned offset = iBegin;
        for (; i < forbiddenDigits.size() + offset; ++i)
        {
            const auto digit = forbiddenDigits[i - offset];
            if (static_cast<unsigned>(digit) != i)
            {
                missing.emplace_back(i);
                ++offset;
            }
        }
    }

    for (; i < capacity + iBegin; ++i)
    {
        missing.emplace_back(i);
    }

    return missing;
}

template <typename Container, typename T>
void erase_value(Container& container, const T& value)
{
    const auto first = std::begin(container);
    const auto last = std::end(container);
    const auto it = std::find(first, last, value);
    container.erase(it);
}

template <typename It, typename T>
void remove_from_candidates(It beginCandidate, It endCandidate, const T& value)
{
    std::for_each(beginCandidate, endCandidate,
          [&value](auto& candidates) { erase_value(candidates, value); });
}

void remove_from_candidate_row(
        SudokuGrid::value_type value,
        ConstrainSolver::candidate_grid& candidateDigits,
        unsigned row)
{
    remove_from_candidates(
        candidateDigits.row_begin(row),
        candidateDigits.row_end(row),
        value);
}

void remove_from_candidate_column(
        SudokuGrid::value_type value,
        ConstrainSolver::candidate_grid& candidateDigits,
        unsigned column)
{
    remove_from_candidates(
        candidateDigits.column_begin(column),
        candidateDigits.column_end(column),
        value);
}

void remove_from_candidates(
        SudokuGrid::value_type value,
        ConstrainSolver::candidate_grid& missingDigits,
        unsigned row,
        unsigned column)
{
    remove_from_candidate_row(value, missingDigits, row);
    remove_from_candidate_column(value, missingDigits, column);

    {
        auto range = sudoku_subgrid_range(missingDigits, row, column);
        remove_from_candidates(range.Begin, range.End, value);
    }
}

std::vector<MatrixPoint<unsigned>> get_occurrences_in_grid(
        SudokuGrid::value_type value,
        const ConstrainSolver::candidate_grid& candidateDigits,
        unsigned row,
        unsigned column)
{
    assert(row < candidateDigits.rows());
    assert(column < candidateDigits.columns());

    constexpr auto subgridSideLength = Sqrt<SudokuGrid::sideLength()>::value;
    const auto rowStart = subgridSideLength * (row / subgridSideLength);
    const auto columnStart = subgridSideLength * (column / subgridSideLength);

    std::vector<MatrixPoint<unsigned>> points;
    for (unsigned r = rowStart; r < rowStart + subgridSideLength; ++r)
    {
        for (unsigned c = columnStart; c < columnStart + subgridSideLength; ++c)
        {
            const auto& candidates = candidateDigits[r][c];
            if (contains(candidates, value))
            {
                points.emplace_back(r, c);
            }
        }
    }

    return points;
}

bool are_on_the_same_row(const std::vector<MatrixPoint<unsigned>>& points)
{
    const auto first = std::cbegin(points);
    const auto last = std::cend(points);
    assert(first != last);

    const auto row = first->Row;
    return std::all_of(std::next(first), last, [row](const auto& p) { return p.Row == row; });
}

bool are_on_the_same_column(const std::vector<MatrixPoint<unsigned>>& points)
{
    const auto first = std::cbegin(points);
    const auto last = std::cend(points);
    assert(first != last);

    const auto column = first->Column;
    return std::all_of(std::next(first), last, [column](const auto& p) { return p.Column == column; });
}

std::vector<MatrixPoint<unsigned>> unsolved_cells_in_this_grid_row(
       const ConstrainSolver::candidate_grid& candidates,
        unsigned row,
        unsigned column)
{
    const auto gridColumnStart = SudokuSubgridSide * (column / SudokuSubgridSide);
    const auto gridColumnEnd = gridColumnStart + SudokuSubgridSide;

    std::vector<MatrixPoint<unsigned>> unsolved;
    unsolved.reserve(SudokuSubgridSide);

    for (auto c = gridColumnStart; c < gridColumnEnd; ++c)
    {
        const auto& cellCandidates = candidates[row][c];
        if (0 != cellCandidates.size())
        {
            unsolved.emplace_back(row, c);
        }
    }

    return unsolved;
}

//std::vector<MatrixPoint<unsigned>> unsolved_cells_in_this_grid_column(
//       const ConstrainSolver::candidate_grid& candidates,
//        unsigned row,
//        unsigned column)
//{
//    const auto gridRowStart = SudokuSubgridSide * (row / SudokuSubgridSide);
//    const auto gridRowEnd = gridRowStart + SudokuSubgridSide;
//
//    std::vector<MatrixPoint<unsigned>> unsolved;
//    unsolved.reserve(SudokuSubgridSide);
//
//    for (auto r = gridRowStart; r < gridRowEnd; ++r)
//    {
//        const auto& cellCandidates = candidates[r][column];
//        if (0 != cellCandidates.size())
//        {
//            unsolved.emplace_back(r, column);
//        }
//    }
//
//    return unsolved;
//}

}

ConstrainSolver::ConstrainSolver(SudokuGrid& grid)
    : Solver(grid)
{
    candidate_collection forbiddenDigits;

    for (unsigned r = 0; r < SudokuGrid::rows(); ++r)
    {
        for (unsigned c = 0; c < SudokuGrid::columns(); ++c)
        {
            if (is_empty(grid[r][c]))
            {
                forbiddenDigits.clear();

                append_row_digits(grid, r, forbiddenDigits);
                append_column_digits(grid, c, forbiddenDigits);
                append_subgrid_digits(grid, r, c, forbiddenDigits);

                auto candidates = get_candidates(forbiddenDigits);
                auto& target = this->CandidateGrid_[r][c];
                std::copy(std::begin(candidates), std::end(candidates), std::back_inserter(target));
            }
        }
    }

}

unsigned ConstrainSolver::iterations() const
{
    return this->Iterations_;
}

bool ConstrainSolver::exec()
{
    do
    {
        for (unsigned r = 0; r < SudokuGrid::rows(); ++r)
        {
            for (unsigned c = 0; c < SudokuGrid::columns(); ++c)
            {
                // If there's no candidate, the cell is already full.
                if (this->CandidateGrid_[r][c].empty())
                    continue;

                // If there's only one possible candidate, that's the right digit to insert.
                SudokuGrid::value_type cellValue = 0;
                if (this->CandidateGrid_[r][c].size() == 1)
                {
                    cellValue = this->CandidateGrid_[r][c][0];
                }
                else
                {
                    // Copy the candidate values for the cell (r, c).
                    const auto candidateValues = this->CandidateGrid_[r][c];

                    {
                        const auto unsolvedRowCellsInSubgrid = unsolved_cells_in_this_grid_row(this->CandidateGrid_, r, c);
                        const auto constrained =
                                (unsolvedRowCellsInSubgrid.size() == candidateValues.size()) &&
                                std::all_of(unsolvedRowCellsInSubgrid.begin(), unsolvedRowCellsInSubgrid.end(),
                                    [&candidateValues, this](const auto& occurrencePosition) { return candidateValues == CandidateGrid_[occurrencePosition]; });

                        if (constrained)
                        {
                            for (const auto candidate : candidateValues)
                            {
                                remove_from_candidate_row(candidate, this->CandidateGrid_, r);
                                for (const auto& occurrence : unsolvedRowCellsInSubgrid)
                                {
                                    this->CandidateGrid_[occurrence].push_back(candidate);
                                }
                            }
                        }
                    }

                    // TODO: fix this!
//                    {
//                        const auto unsolvedCellsInSubgridColumn = unsolved_cells_in_this_grid_column(this->CandidateGrid_, r, c);
//                        const auto constrained =
//                                (unsolvedCellsInSubgridColumn.size() == candidateValues.size()) &&
//                                std::all_of(unsolvedCellsInSubgridColumn.begin(), unsolvedCellsInSubgridColumn.end(),
//                                    [&candidateValues, this](const auto& occurrencePosition) { return candidateValues == CandidateGrid_[occurrencePosition]; });
//
//                        if (constrained)
//                        {
////#ifndef NDEBUG
////                            puts("Column constrained");
////#endif
//                            for (const auto candidate : candidateValues)
//                            {
//                                remove_from_candidate_column(candidate, this->CandidateGrid_, c);
//                                for (const auto& occurrence : unsolvedCellsInSubgridColumn)
//                                {
//                                    this->CandidateGrid_[occurrence].push_back(candidate);
//                                }
//                            }
//                        }
//                    }

                    for (const auto candidate : candidateValues)
                    {
                        const auto gridOccurrences = get_occurrences_in_grid(candidate, this->CandidateGrid_, r, c);
                        if (are_on_the_same_row(gridOccurrences))
                        {
                            remove_from_candidate_row(candidate, this->CandidateGrid_, r);
                            for (const auto& gridOccurrence : gridOccurrences)
                            {
                                this->CandidateGrid_[gridOccurrence].push_back(candidate);
                            }
                        }
                        else if (are_on_the_same_column(gridOccurrences))
                        {
                            remove_from_candidate_column(candidate, this->CandidateGrid_, c);
                            for (const auto& gridOccurrence : gridOccurrences)
                            {
                                this->CandidateGrid_[gridOccurrence].push_back(candidate);
                            }
                        }

                        const auto gridOccurrenceCount = gridOccurrences.size();
                        assert(gridOccurrenceCount > 0);
                        if (1 == gridOccurrenceCount)
                        {
                            cellValue = candidate;
                            break;
                        }
                    }
                }

                if (0 != cellValue)
                {
                    // This cell is now fixed.
                    this->CandidateGrid_[r][c].clear();
                    remove_from_candidates(cellValue, this->CandidateGrid_, r, c);
                    (*this->Grid_)[r][c] = cellValue;
                    ++(this->InsertedDigits_);
                }
            }
        }

        ++(this->Iterations_);
    }
    while(this->InsertedDigits_ != this->NumberOfMissingDigits_);

    return this->InsertedDigits_ == this->NumberOfMissingDigits_;
}
