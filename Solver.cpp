#include "Solver.h"

#include "StaticVector.h"
#include "constexpr_functions.h"

#include <algorithm>
#include <vector>

namespace
{

template <typename Grid, typename Func>
Func sudoku_subgrid_for_each(Grid& grid, unsigned row, unsigned column, Func func)
{
    using grid_type = typename std::remove_cv<Grid>::type;
    static_assert (is_matrix<grid_type>::value, "Grid is not a Matrix<> type" );

    static_assert (Grid::columns() == SudokuGrid::columns(), "" );
    static_assert (Grid::rows() == SudokuGrid::rows(), "" );

    constexpr auto subgridSideLength = Sqrt<SudokuGrid::rows()>::value;
    const auto rowStart = subgridSideLength * (row / subgridSideLength);
    const auto columnStart = subgridSideLength * (column / subgridSideLength);

    return matrix_grid_for_each<subgridSideLength, subgridSideLength>(grid, rowStart, columnStart, func);
}

template <typename Container, typename T>
bool contains(Container& container, const T value)
{
    auto last = std::end(container);
    return last != std::find(std::begin(container), last, value);
}

struct NonZeroDigitAppender
{
    explicit NonZeroDigitAppender(Solver::cell_digit_collection& digits) noexcept
        :
          Digits_(digits)
    { }

    void operator() (const SudokuGrid::value_type& value)
    {
        if (0 != value && !contains(this->Digits_, value))
        {
            this->Digits_.push_back(value);
        }
    }

private:
    Solver::cell_digit_collection& Digits_;
};

void append_row_digits(
        const SudokuGrid& grid,
        size_t row,
        Solver::cell_digit_collection& digits)
{
    matrix_row_for_each(grid, row, NonZeroDigitAppender(digits));
}

void append_column_digits(
        const SudokuGrid& grid,
        size_t column,
        Solver::cell_digit_collection& digits)
{
    matrix_column_for_each(grid, column, NonZeroDigitAppender(digits));
}

void append_subgrid_digits(
        const SudokuGrid& grid,
        size_t row,
        size_t column,
        Solver::cell_digit_collection& digits)
{
    sudoku_subgrid_for_each(grid, row, column, NonZeroDigitAppender(digits));
}

template <size_t Capacity>
std::vector<SudokuGrid::value_type> get_candidates(StaticVector<SudokuGrid::value_type, Capacity>& forbiddenDigits)
{
    // Sort the input vector.
    std::sort(std::begin(forbiddenDigits), std::end(forbiddenDigits));

    std::vector<SudokuGrid::value_type> missing;
    missing.reserve(Capacity - forbiddenDigits.size());

    constexpr auto iBegin = 1;

    size_t i = iBegin;

    {
        size_t offset = iBegin;
        for (; i < forbiddenDigits.size() + offset; ++i)
        {
            const auto digit = forbiddenDigits[i - offset];
            if (static_cast<size_t>(digit) != i)
            {
                missing.emplace_back(i);
                ++offset;
            }
        }
    }

    for (; i < Capacity + iBegin; ++i)
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

void remove_from_candidate_row(
        SudokuGrid::value_type value,
        Solver::candidate_digit_collection& candidateDigits,
        size_t row)
{
    matrix_row_for_each(candidateDigits, row, [value](auto& digits) { erase_value(digits, value); });
}

void remove_from_candidate_column(
        SudokuGrid::value_type value,
        Solver::candidate_digit_collection& candidateDigits,
        size_t column)
{
    matrix_column_for_each(candidateDigits, column, [value](auto& digits) { erase_value(digits, value); });
}

void remove_from_candidates(
        char value,
        Matrix<StaticVector<char, SudokuGrid::rows()>, SudokuGrid::rows(), SudokuGrid::columns()>& missingDigits,
        size_t row,
        size_t column)
{
    remove_from_candidate_row(value, missingDigits, row);
    remove_from_candidate_column(value, missingDigits, column);
    sudoku_subgrid_for_each(missingDigits, row, column, [value](auto& digits) { erase_value(digits, value); });
}

std::vector<MatrixPoint<unsigned>> get_occurrences_in_grid(
        SudokuGrid::value_type value,
        const Solver::candidate_digit_collection& candidateDigits,
        size_t row,
        size_t column)
{
    assert(row < candidateDigits.rows());
    assert(column < candidateDigits.columns());

    constexpr auto subgridSideLength = Sqrt<SudokuGrid::rows()>::value;
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

std::vector<MatrixPoint<unsigned>> get_occurrences_in_row(
        SudokuGrid::value_type value,
        const Solver::candidate_digit_collection& candidateDigits,
        size_t row)
{
    assert(row < candidateDigits.rows());

    std::vector<MatrixPoint<unsigned>> points;
    for (unsigned c = 0; c < candidateDigits.columns(); ++c)
    {
        const auto& candidates = candidateDigits[row][c];
        if (contains(candidates, value))
        {
            points.emplace_back(row, c);
        }
    }

    return points;
}

std::vector<MatrixPoint<unsigned>> get_occurrences_in_column(
        SudokuGrid::value_type value,
        const Solver::candidate_digit_collection& candidateDigits,
        size_t column)
{
    assert(column < candidateDigits.columns());

    std::vector<MatrixPoint<unsigned>> points;
    for (unsigned r = 0; r < candidateDigits.rows(); ++r)
    {
        const auto& candidates = candidateDigits[r][column];
        if (contains(candidates, value))
        {
            points.emplace_back(r, column);
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

void print_cell_candidates(const Solver::cell_digit_collection& candidates)
{
    for (auto candidate : candidates)
    {
        printf("%c ", candidate + 48);
    }
    printf("\n");
}

void print_candidates(const Solver::candidate_digit_collection& candidates)
{
    SudokuGrid g;

    constexpr auto maxDigit = static_cast<char>(SudokuGrid::rows());
    for (char digit = 1; digit <= maxDigit; ++digit)
    {

        size_t candidateCount = 0;
        for (unsigned r = 0; r < candidates.rows(); ++r)
        {
            for (unsigned c = 0; c < candidates.columns(); ++c)
            {
                g[r][c] = (contains(candidates[r][c], digit) * digit);
                candidateCount += (g[r][c] != 0);
            }
        }

        if (candidateCount > 0)
        {
            printf("Possible positions of '%c':\n", digit + 48);
            print_grid(g);
            puts("");
        }
    }
}

std::vector<MatrixPoint<unsigned>> unsolved_cells_in_this_grid_row(
       const Solver::candidate_digit_collection& candidates,
       size_t row,
       size_t column)
{
    const size_t gridColumnStart = 3 * (column / 3);
    const size_t gridColumnEnd = gridColumnStart + 3;

    std::vector<MatrixPoint<unsigned>> unsolved;
    unsolved.reserve(3);

    for (size_t c = gridColumnStart; c < gridColumnEnd; ++c)
    {
        const auto& cellCandidates = candidates[row][c];
        if (0 != cellCandidates.size())
        {
            unsolved.emplace_back(row, c);
        }
    }

    return unsolved;
}

std::vector<MatrixPoint<unsigned>> unsolved_cells_in_this_grid_column(
       const Solver::candidate_digit_collection& candidates,
       size_t row,
       size_t column)
{
    const size_t rowColumnStart = 3 * (row / 3);
    const size_t rowColumnEnd = rowColumnStart + 3;

    std::vector<MatrixPoint<unsigned>> unsolved;
    unsolved.reserve(3);

    for (size_t r = rowColumnStart; r < rowColumnEnd; ++r)
    {
        const auto& cellCandidates = candidates[r][column];
        if (0 != cellCandidates.size())
        {
            unsolved.emplace_back(r, column);
        }
    }

    return unsolved;
}

}

Solver::Solver(SudokuGrid& grid)
    :
      CandidateDigits_(),
      Grid_(std::addressof(grid)),
      NumberOfMissingDigits_(0)
{
    cell_digit_collection forbiddenDigits;

    for (size_t r = 0; r < SudokuGrid::rows(); ++r)
    {
        for (size_t c = 0; c < SudokuGrid::columns(); ++c)
        {
            if (0 == grid[r][c])
            {
                forbiddenDigits.clear();

                append_row_digits(grid, r, forbiddenDigits);
                append_column_digits(grid, c, forbiddenDigits);
                append_subgrid_digits(grid, r, c, forbiddenDigits);

                auto candidates = get_candidates(forbiddenDigits);
                auto& target = this->CandidateDigits_[r][c];
                std::copy(std::begin(candidates), std::end(candidates), std::back_inserter(target));

                ++(this->NumberOfMissingDigits_);
            }
        }
    }
}

bool Solver::exec()
{
    size_t inserted = 0;

    do
    {
        inserted = 0;

        for (size_t r = 0; r < SudokuGrid::rows(); ++r)
        {
            for (size_t c = 0; c < SudokuGrid::columns(); ++c)
            {

                const auto cellCandidates = this->CandidateDigits_[r][c];

                {
                    //const auto cellCandidates = this->CandidateDigits_[r][c];
                    const auto rc = unsolved_cells_in_this_grid_row(this->CandidateDigits_, r, c);
                    if (rc.size() != 0 && rc.size() == cellCandidates.size())
                    {
                        const auto constained = std::all_of(std::cbegin(rc), std::cend(rc),
                                            [&cellCandidates, this](const auto& occurrence){ return cellCandidates == CandidateDigits_[occurrence]; });

                        if (constained)
                        {
                            for (const auto cellValue : cellCandidates)
                            {
                                remove_from_candidate_row(cellValue, this->CandidateDigits_, r);
                                for (const auto& occurrence : rc)
                                {
                                    this->CandidateDigits_[occurrence].push_back(cellValue);
                                }
                            }
                        }
                    }
                }

                if (1 == cellCandidates.size())
                {
                    const auto cellValue = cellCandidates[0];

                    // This cell is now fixed
                    this->CandidateDigits_[r][c].clear();
                    remove_from_candidates(cellValue, this->CandidateDigits_, r, c);
                    (*this->Grid_)[r][c] = cellValue;

                    ++inserted;
                }
                else if (cellCandidates.size() > 1)
                {

                    for (const auto cellValue : cellCandidates)
                    {
                        const auto gridOccurrences = get_occurrences_in_grid(cellValue, this->CandidateDigits_, r, c);
                        if (are_on_the_same_row(gridOccurrences))
                        {
                            remove_from_candidate_row(cellValue, this->CandidateDigits_, r);
                            for (const auto& gridOccurrence : gridOccurrences)
                            {
                                this->CandidateDigits_[gridOccurrence].push_back(cellValue);
                            }
                        }
                        else if (are_on_the_same_column(gridOccurrences))
                        {
                            remove_from_candidate_column(cellValue, this->CandidateDigits_, c);
                            for (const auto& gridOccurrence : gridOccurrences)
                            {
                                this->CandidateDigits_[gridOccurrence].push_back(cellValue);
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
                            this->CandidateDigits_[r][c].clear();
                            remove_from_candidates(cellValue, this->CandidateDigits_, r, c);
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

    //print_candidates(this->CandidateDigits_);
    //for (int c = 0; c < 9; ++c)
    //{
    //    const auto& candidates = this->CandidateDigits_[8][c];
    //    if (candidates.size())
    //    {
    //        printf("Missing on cell (%d, %d)\n:", 9, c+1);
    //        print_cell_candidates(candidates);
    //    }
    //}

    return this->InsertedDigits_ == this->NumberOfMissingDigits_;
}

unsigned Solver::insertedDigits() const
{
    return this->InsertedDigits_;
}

unsigned Solver::originalNumberOfMissingDigits() const
{
    return this->NumberOfMissingDigits_;
}

unsigned Solver::iterations() const
{
    return this->Iterations_;
}
