#include "ConstrainSolver.h"

#include "StaticVector.h"
#include "constexpr_functions.h"

#include <algorithm>
#include <cstddef>
#include <vector>

namespace
{

template <typename Container, typename T>
bool contains(Container& container, const T value)
{
    auto last = std::end(container);
    return last != std::find(std::begin(container), last, value);
}

constexpr bool is_empty(const SudokuGrid::value_type& cellValue) noexcept
{
    return 0 == cellValue;
}

template <typename InputIterator>
constexpr void append_non_empty_values(
        InputIterator beginCell,
        InputIterator endCell,
        ConstrainSolver::candidate_collection& digits)
{
    for (auto it = beginCell; it != endCell; ++it)
    {
        const auto value = *it;
        if (!is_empty(value) && !contains(digits, value))
        {
            digits.push_back(value);
        }
    }
}

void append_row_digits(
        const SudokuGrid& grid,
        unsigned row,
        ConstrainSolver::candidate_collection& digits)
{
    append_non_empty_values(grid.row_begin(row), grid.row_end(row), digits);
}

void append_column_digits(
        const SudokuGrid& grid,
        unsigned column,
        ConstrainSolver::candidate_collection& digits)
{
    append_non_empty_values(grid.column_begin(column), grid.column_end(column), digits);
}

void append_subgrid_digits(
        const SudokuGrid& grid,
        unsigned row,
        unsigned column,
        ConstrainSolver::candidate_collection& digits)
{
    constexpr auto subgridSide = Sqrt<SudokuGrid::rows()>::value;
    const auto rowStart = subgridSide * (row / subgridSide);
    const auto columnStart = subgridSide * (column / subgridSide);
    append_non_empty_values(
                grid.subgrid_begin<subgridSide, subgridSide>(rowStart, columnStart),
                grid.subgrid_end<subgridSide, subgridSide>(rowStart, columnStart),
                digits);
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

template <typename InputIterator, typename T>
void remove_from_candidates(
        InputIterator beginCandidate,
        InputIterator endCandidate,
        const T& value)
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

    constexpr auto subgridSide = Sqrt<SudokuGrid::sideLength()>::value;
    const auto rowStart = subgridSide * (row / subgridSide);
    const auto columnStart = subgridSide * (column / subgridSide);
    remove_from_candidates(
        missingDigits.subgrid_begin<subgridSide, subgridSide>(rowStart, columnStart),
        missingDigits.subgrid_end<subgridSide, subgridSide>(rowStart, columnStart),
        value);
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
    const auto gridColumnStart = 3 * (column / 3);
    const auto gridColumnEnd = gridColumnStart + 3;

    std::vector<MatrixPoint<unsigned>> unsolved;
    unsolved.reserve(3);

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

}

ConstrainSolver::ConstrainSolver(SudokuGrid& grid)
    : Solver(grid)
{
    candidate_collection forbiddenDigits;

    for (unsigned r = 0; r < SudokuGrid::rows(); ++r)
    {
        for (unsigned c = 0; c < SudokuGrid::columns(); ++c)
        {
            if (0 == grid[r][c])
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
    unsigned inserted = 0;

    do
    {
        inserted = 0;

        for (unsigned r = 0; r < SudokuGrid::rows(); ++r)
        {
            for (unsigned c = 0; c < SudokuGrid::columns(); ++c)
            {

                const auto cellCandidates = this->CandidateGrid_[r][c];

                {
                    //const auto cellCandidates = this->CandidateDigits_[r][c];
                    const auto rc = unsolved_cells_in_this_grid_row(this->CandidateGrid_, r, c);
                    if (rc.size() != 0 && rc.size() == cellCandidates.size())
                    {
                        const auto constained = std::all_of(std::cbegin(rc), std::cend(rc),
                                            [&cellCandidates, this](const auto& occurrence){ return cellCandidates == CandidateGrid_[occurrence]; });

                        if (constained)
                        {
                            for (const auto cellValue : cellCandidates)
                            {
                                remove_from_candidate_row(cellValue, this->CandidateGrid_, r);
                                for (const auto& occurrence : rc)
                                {
                                    this->CandidateGrid_[occurrence].push_back(cellValue);
                                }
                            }
                        }
                    }
                }

                {
                    const auto cc = unsolved_cells_in_this_grid_row(this->CandidateGrid_, r, c);
                    if (cc.size() != 0 && cc.size() == cellCandidates.size())
                    {
                        const auto constained = std::all_of(std::cbegin(cc), std::cend(cc),
                                            [&cellCandidates, this](const auto& occurrence){ return cellCandidates == CandidateGrid_[occurrence]; });

                        if (constained)
                        {
                            for (const auto cellValue : cellCandidates)
                            {
                                remove_from_candidate_column(cellValue, this->CandidateGrid_, c);
                                for (const auto& occurrence : cc)
                                {
                                    this->CandidateGrid_[occurrence].push_back(cellValue);
                                }
                            }
                        }
                    }
                }

                if (1 == cellCandidates.size())
                {
                    const auto cellValue = cellCandidates[0];

                    // This cell is now fixed
                    this->CandidateGrid_[r][c].clear();
                    remove_from_candidates(cellValue, this->CandidateGrid_, r, c);
                    (*this->Grid_)[r][c] = cellValue;

                    ++inserted;
                }
                else if (cellCandidates.size() > 1)
                {

                    for (const auto cellValue : cellCandidates)
                    {
                        const auto gridOccurrences = get_occurrences_in_grid(cellValue, this->CandidateGrid_, r, c);
                        if (are_on_the_same_row(gridOccurrences))
                        {
                            remove_from_candidate_row(cellValue, this->CandidateGrid_, r);
                            for (const auto& gridOccurrence : gridOccurrences)
                            {
                                this->CandidateGrid_[gridOccurrence].push_back(cellValue);
                            }
                        }
                        else if (are_on_the_same_column(gridOccurrences))
                        {
                            remove_from_candidate_column(cellValue, this->CandidateGrid_, c);
                            for (const auto& gridOccurrence : gridOccurrences)
                            {
                                this->CandidateGrid_[gridOccurrence].push_back(cellValue);
                            }
                        }

                        const auto gridOccurrenceCount = gridOccurrences.size();
                        assert(gridOccurrenceCount > 0);

                        //const auto rowOccurrenceCount = get_occurrences_in_row(cellValue, this->CandidateDigits_, r).size();
                        //assert(rowOccurrenceCount > 0);

                        //const auto columnOccurrenceCount = get_occurrences_in_column(cellValue, this->CandidateDigits_, c).size();
                        //assert(columnOccurrenceCount > 0);

                        if (1 == gridOccurrenceCount) // || 1 == rowOccurrenceCount || 1 == columnOccurrenceCount)
                        {
                            // This cell is now fixed.
                            this->CandidateGrid_[r][c].clear();
                            remove_from_candidates(cellValue, this->CandidateGrid_, r, c);
                            (*this->Grid_)[r][c] = cellValue;

                            ++inserted;
                            break;
                        }
                    }
                }
            }
        }

        this->InsertedDigits_ += inserted;
        ++(this->Iterations_);
    }
    while(0 != inserted);

    return this->InsertedDigits_ == this->NumberOfMissingDigits_;
}
