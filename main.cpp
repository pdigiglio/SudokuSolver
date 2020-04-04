#include <algorithm>
#include <array>
#include <cassert>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>

#include "inverse_evaluator.h"
#include "StaticVector.h"

template <unsigned i>
struct Square
{
    static constexpr auto value = i * i;
};

template <unsigned i>
struct Sqrt : public InverseEvaluator<Square, i>
{ };

template <typename Container, typename T>
bool contains(Container& container, const T value)
{
    auto last = std::end(container);
    return last != std::find(std::begin(container), last, value);
}

template <typename T, size_t N>
const T& get_cell_at(const std::array<T, N>& grid, size_t row, size_t column)
{
    constexpr auto digits = Sqrt<N>::value;
    assert(row < digits);
    assert(column < digits);
    return grid[row * digits + column];
}

template <typename T, size_t N>
T& get_cell_at(std::array<T, N>& grid, size_t row, size_t column)
{
    constexpr auto digits = Sqrt<N>::value;
    assert(row < digits);
    assert(column < digits);
    return grid[row * digits + column];
}

template <typename T, size_t N>
void set_cell_at(std::array<T, N>& grid, size_t row, size_t column, T value)
{
    get_cell_at(grid, row, column) = value;
}

template <typename T, size_t N, size_t Capacity>
void append_row_digits(const std::array<T, N>& grid, size_t row, StaticVector<T, Capacity>& digits)
{
    constexpr auto gridSideLength = Sqrt<N>::value;
    assert(row < gridSideLength);

    for (size_t c = 0; c < gridSideLength; ++c)
    {
        auto cellValue = get_cell_at(grid, row, c);
        if (0 != cellValue && !contains(digits, cellValue))
        {
            digits.push_back(cellValue);
        }
    }
}

template <typename T, size_t N, size_t Capacity>
void append_column_digits(const std::array<T, N>& grid, size_t column, StaticVector<T, Capacity>& digits)
{
    constexpr auto gridSideLength = Sqrt<N>::value;
    assert(column < gridSideLength);

    for (size_t r = 0; r < gridSideLength; ++r)
    {
        auto cellValue = get_cell_at(grid, r, column);
        if (0 != cellValue && !contains(digits, cellValue))
        {
            digits.push_back(cellValue);
        }
    }
}

template <typename T, size_t N, size_t Capacity>
void append_subgrid_digits(const std::array<T, N>& grid, size_t row, size_t column, StaticVector<T, Capacity>& digits)
{
    constexpr auto gridSideLength = Sqrt<N>::value;
    assert(column < gridSideLength);
    assert(row < gridSideLength);

    constexpr auto subgridSideLength = Sqrt<gridSideLength>::value;

    const auto rowStart = subgridSideLength * (row / subgridSideLength);
    const auto rowEnd = rowStart + subgridSideLength;

    const auto columnStart = subgridSideLength * (column / subgridSideLength);
    const auto columnEnd = columnStart + subgridSideLength;

    for (size_t r = rowStart; r < rowEnd; ++r)
    {
        for (size_t c = columnStart; c < columnEnd; ++c)
        {
            const auto cellValue = get_cell_at(grid, r, c);
            if (0 != cellValue && !contains(digits, cellValue))
            {
                digits.push_back(cellValue);
            }
        }
    }
}

template <typename T, size_t N>
void print_grid(const std::array<T, N>& grid)
{
    constexpr auto digits = Sqrt<N>::value;
    constexpr auto subgridSideLength = Sqrt<digits>::value;

    for (size_t sgr = 0; sgr < subgridSideLength; ++sgr)
    {
        const auto rStart = sgr * subgridSideLength;
        const auto rEnd = (sgr + 1) * subgridSideLength;
        for (size_t r = rStart; r < rEnd; ++r)
        {
            for (size_t sgc = 0; sgc < subgridSideLength; ++sgc)
            {
                const auto cStart = sgc * subgridSideLength;
                const auto cEnd = (sgc + 1) * subgridSideLength;
                for (size_t c = cStart; c < cEnd; ++c)
                {
                    printf(" %c ",  get_cell_at(grid, r, c) + 48);
                }
                printf("|");
            }
            printf("\n");
        }
        printf("------------------------------\n");
    }
}

template <typename T, size_t Capacity>
std::vector<T> get_missing(StaticVector<T, Capacity>& digits)
{
    auto first = std::begin(digits);
    auto last = std::end(digits);
    std::sort(first, last);

    std::vector<T> missing;
    missing.reserve(Capacity - digits.size());

    constexpr auto iBegin = 1;

    size_t i = iBegin;

    {
        size_t offset = iBegin;
        for (; i < digits.size() + offset; ++i)
        {
            const auto digit = digits[i - offset];
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

template <typename T, size_t Digits>
void remove_value(StaticVector<T, Digits>& missingDigits, T value)
{
    const auto first = std::begin(missingDigits);
    const auto last = std::end(missingDigits);
    const auto it = std::find(first, last, value);
    missingDigits.erase(it);
}

template <typename T, size_t N, size_t Digits = Sqrt<N>::value>
void remove_from_missing(T value, std::array<StaticVector<T, Digits>, N>& missingDigits, size_t row, size_t column)
{
    for (size_t c = 0; c < Digits; ++c)
    {
        auto& missing = get_cell_at(missingDigits, row, c);
        remove_value(missing, value);
    }

    for (size_t r = 0; r < Digits; ++r)
    {
        auto& missing = get_cell_at(missingDigits, r, column);
        remove_value(missing, value);
    }

    constexpr auto subgridSideLength = Sqrt<Digits>::value;

    const auto rowStart = subgridSideLength * (row / subgridSideLength);
    const auto rowEnd = rowStart + subgridSideLength;

    const auto columnStart = subgridSideLength * (column / subgridSideLength);
    const auto columnEnd = columnStart + subgridSideLength;

    for (size_t r = rowStart; r < rowEnd; ++r)
    {
        for (size_t c = columnStart; c < columnEnd; ++c)
        {
            auto& missing = get_cell_at(missingDigits, r, c);
            remove_value(missing, value);
        }
    }
}

template <typename T, size_t N, size_t Digits = Sqrt<N>::value>
size_t count_occurrences_in_row(T value, const std::array<StaticVector<T, Digits>, N>& missingDigits, size_t row)
{
    size_t occurrences = 0;
    for (size_t c = 0; c < Digits; ++c)
    {
        const auto& missing = get_cell_at(missingDigits, row, c);
        occurrences += contains(missing, value);
    }
    return occurrences;
}

template <typename T, size_t N, size_t Digits = Sqrt<N>::value>
size_t count_occurrences_in_column(T value, const std::array<StaticVector<T, Digits>, N>& missingDigits, size_t column)
{
    size_t occurrences = 0;
    for (size_t r = 0; r < Digits; ++r)
    {
        const auto& missing = get_cell_at(missingDigits, r, column);
        occurrences += contains(missing, value);
    }
    return occurrences;
}

template <typename T, size_t N, size_t Digits = Sqrt<N>::value>
size_t count_occurrences_in_grid(T value, const std::array<StaticVector<T, Digits>, N>& missingDigits, size_t row, size_t column)
{
    size_t occurrences = 0;

    constexpr auto subgridSideLength = Sqrt<Digits>::value;

    const auto rowStart = subgridSideLength * (row / subgridSideLength);
    const auto rowEnd = rowStart + subgridSideLength;

    const auto columnStart = subgridSideLength * (column / subgridSideLength);
    const auto columnEnd = columnStart + subgridSideLength;

    for (size_t r = rowStart; r < rowEnd; ++r)
    {
        for (size_t c = columnStart; c < columnEnd; ++c)
        {
            const auto& missing = get_cell_at(missingDigits, r, c);
            occurrences += contains(missing, value);
        }
    }

    return occurrences;
}

template <typename T, size_t N>
class Solver final
{
public:
    static constexpr size_t digits()
    {
        return Sqrt<N>::value;
    }

    explicit Solver(std::array<T, N>& grid)
        :
          MissingDigits_(),
          Grid_(std::addressof(grid)),
          NumberOfMissingDigits_(0)
    {
        StaticVector<T, Solver::digits()> forbiddenDigits;

        for (size_t r = 0; r < Solver::digits(); ++r)
        {
            for (size_t c = 0; c < Solver::digits(); ++c)
            {
                if (0 == get_cell_at(grid, r, c))
                {
                    forbiddenDigits.clear();

                    append_row_digits(grid, r, forbiddenDigits);
                    append_column_digits(grid, c, forbiddenDigits);
                    append_subgrid_digits(grid, r, c, forbiddenDigits);

                    auto missing = get_missing(forbiddenDigits);
                    auto& target = get_cell_at(this->MissingDigits_, r, c);
                    std::copy(std::begin(missing), std::end(missing), std::back_inserter(target));

                    ++(this->NumberOfMissingDigits_);
                }
            }
        }
    }

    bool exec()
    {
        size_t inserted = 0;

        do
        {
            inserted = 0;

            for (size_t r = 0; r < Solver::digits(); ++r)
            {
                for (size_t c = 0; c < Solver::digits(); ++c)
                {
                    auto& missing = get_cell_at(this->MissingDigits_, r, c);
                    if (1 == missing.size())
                    {
                        const auto value = missing[0];

                        // This cell is now fixed
                        missing.clear();
                        remove_from_missing(value, this->MissingDigits_, r, c);
                        set_cell_at(*this->Grid_, r, c, value);

                        ++inserted;
                    }
                    else if (missing.size() > 1)
                    {
                        for (const auto value : missing)
                        {
                            const auto gridOccurrences = count_occurrences_in_grid(value, this->MissingDigits_, r, c);
                            assert(gridOccurrences > 0);

                            const auto rowOccurrences = count_occurrences_in_row(value, this->MissingDigits_, r);
                            assert(rowOccurrences > 0);

                            const auto columnOccurrences = count_occurrences_in_column(value, this->MissingDigits_, c);
                            assert(columnOccurrences > 0);

                            if (1 == gridOccurrences || 1 == rowOccurrences || 1 == columnOccurrences)
                            {
                                // This cell is now fixed.
                                missing.clear();
                                remove_from_missing(value, this->MissingDigits_, r, c);
                                set_cell_at(*this->Grid_, r, c, value);

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

    size_t insertedDigits() const
    {
        return this->InsertedDigits_;
    }

    size_t originalNumberOfMissingDigits() const
    {
        return this->NumberOfMissingDigits_;
    }

    size_t iterations() const
    {
        return this->Iterations_;
    }

private:
    std::array<StaticVector<T, Solver::digits()>, N> MissingDigits_;
    std::array<T, N>* Grid_ = nullptr;
    size_t InsertedDigits_ = 0;
    size_t Iterations_ = 0;
    size_t NumberOfMissingDigits_ = 0;
};

template <typename T, size_t N>
Solver<T, N> make_solver(std::array<T, N>& grid)
{
    return Solver<T, N>(grid);
}

template <size_t N = 81>
bool fill_from_input_file(const char* filePath, std::array<char, N>& grid)
{
    std::ifstream inFile (filePath);
    if (!inFile.is_open())
    {
        fprintf(stderr, "Invalid input file '%s'\n", filePath);
        return false;
    }

    constexpr auto gridSideLength = Sqrt<N>::value;
    for (size_t r = 0; r < gridSideLength; ++r)
    {
        for (size_t c = 0; c < gridSideLength; ++c)
        {
            char buffer = 0;
            inFile >> buffer;

            if (buffer < '0' || buffer > '9')
            {
                fprintf(stderr, "Invalid input: '%c' at (%zu, %zu)\n", buffer, r, c);
                return false;
            }

            set_cell_at(grid, r, c, static_cast<char>(buffer - '0'));
        }
    }

    return true;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: SudokuSolver \"input file\"\n");
        return 1;
    }

    constexpr char digits = 9;

    std::array<char, digits * digits> grid;
    const auto readStatus = fill_from_input_file(argv[1], grid);
    if (!readStatus)
        return 1;

    print_grid(grid);

    auto solver = make_solver(grid);
    solver.exec();

    puts("\nSolved:\n");
    print_grid(grid);

    printf("Inserted %zu (of %zu) elements in %zu iterations.\n",
           solver.insertedDigits(),
           solver.originalNumberOfMissingDigits(),
           solver.iterations());

    return 0;
}
