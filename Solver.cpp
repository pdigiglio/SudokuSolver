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

void remove_from_candidates(
        char value,
        Matrix<StaticVector<char, SudokuGrid::rows()>, SudokuGrid::rows(), SudokuGrid::columns()>& missingDigits,
        size_t row,
        size_t column)
{
    using missing_container = StaticVector<char, SudokuGrid::rows()>;
    auto removeValue = [value](missing_container& missing)
    {
        const auto first = std::begin(missing);
        const auto last = std::end(missing);
        const auto it = std::find(first, last, value);
        missing.erase(it);
    };

    matrix_row_for_each(missingDigits, row, removeValue);
    matrix_column_for_each(missingDigits, column, removeValue);
    sudoku_subgrid_for_each(missingDigits, row, column, removeValue);
}

struct OccurrenceCounter
{
    size_t Occurrences = 0;
    SudokuGrid::value_type Value;

    explicit OccurrenceCounter(SudokuGrid::value_type value)
        :
          Occurrences(0),
          Value(value)
    { }

    void operator()(const Solver::cell_digit_collection& container)
    {
        this->Occurrences += contains(container, this->Value);
    }
};

size_t count_occurrences_in_grid(
        char value,
        const Matrix<StaticVector<char, SudokuGrid::rows()>, SudokuGrid::rows(), SudokuGrid::columns()>& candidateDigits,
        size_t row,
        size_t column)
{
    auto counter = sudoku_subgrid_for_each(candidateDigits, row, column, OccurrenceCounter(value));
    return counter.Occurrences;
}

//size_t count_occurrences_in_row(
//        char value,
//        const Matrix<StaticVector<char, SudokuGrid::rows()>, SudokuGrid::rows(), SudokuGrid::columns()>& missingDigits,
//        size_t row)
//{
//    auto counter = matrix_row_for_each(missingDigits, row, OccurrenceCounter(value));
//    return counter.Occurrences;
//}
//
//size_t count_occurrences_in_column(
//        char value,
//        const Matrix<StaticVector<char, SudokuGrid::rows()>, SudokuGrid::rows(), SudokuGrid::columns()>& missingDigits,
//        size_t row)
//{
//    auto counter = matrix_column_for_each(missingDigits, row, OccurrenceCounter(value));
//    return counter.Occurrences;
//}

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
                auto& cellCandidates = this->CandidateDigits_[r][c];
                if (1 == cellCandidates.size())
                {
                    const auto cellValue = cellCandidates[0];

                    // This cell is now fixed
                    cellCandidates.clear();
                    remove_from_candidates(cellValue, this->CandidateDigits_, r, c);
                    (*this->Grid_)[r][c] = cellValue;

                    ++inserted;
                }
                else if (cellCandidates.size() > 1)
                {
                    for (const auto cellValue : cellCandidates)
                    {
                        const auto gridOccurrences = count_occurrences_in_grid(cellValue, this->CandidateDigits_, r, c);
                        assert(gridOccurrences > 0);

                        //const auto rowOccurrences = count_occurrences_in_row(value, this->CandidateDigits_, r);
                        //assert(rowOccurrences > 0);

                        //const auto columnOccurrences = count_occurrences_in_column(value, this->CandidateDigits_, c);
                        //assert(columnOccurrences > 0);

                        if (1 == gridOccurrences) // || 1 == rowOccurrences || 1 == columnOccurrences)
                        {
                            // This cell is now fixed.
                            cellCandidates.clear();
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
