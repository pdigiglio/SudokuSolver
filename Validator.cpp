#include "Validator.h"

#include "StaticVector.h"
#include "SudokuGrid.h"
#include "constexpr_functions.h"

#include <algorithm>

Validator::Validator(const SudokuGrid& grid)
    : Grid_(std::addressof(grid))
{ }

namespace
{

template <typename Container>
bool has_duplicates(Container& container)
{
    auto first = std::begin(container);
    auto last = std::end(container);
    std::sort(first, last);

    auto it = std::unique(first, last);
    return it != last;
}

template <typename InputIterator>
bool has_duplicate_non_empty_cells(InputIterator beginCell, InputIterator endCell)
{
    using cell_type = SudokuGrid::value_type;
    StaticVector<cell_type, SudokuGrid::rows()> nonEmptyDigits;

    auto outputIterator = std::back_inserter(nonEmptyDigits);
    std::copy_if(beginCell, endCell, outputIterator,
         [](const cell_type& value) { return 0 != value; });

    return has_duplicates(nonEmptyDigits);
}

}

bool Validator::validate()
{
    for (unsigned r = 0; r < SudokuGrid::rows(); ++r)
    {
        const auto rowHasDuplicates = has_duplicate_non_empty_cells(
                    this->Grid_->row_cbegin(r), this->Grid_->row_cend(r));

        if (rowHasDuplicates)
        {
            this->DuplicateType_ = "row";
            this->DuplicateIndex_ = r;
            return false;
        }
    }

    for (unsigned c = 0; c < SudokuGrid::columns(); ++c)
    {
        const auto columnHasDUplicates = has_duplicate_non_empty_cells(
                    this->Grid_->column_cbegin(c), this->Grid_->column_cend(c));
        if (columnHasDUplicates)
        {
            this->DuplicateType_ = "col";
            this->DuplicateIndex_ = c;
            return false;
        }
    }

    constexpr auto subgridSide = Sqrt<SudokuGrid::rows()>::value;
    for (unsigned r = 0; r < SudokuGrid::rows(); r += subgridSide)
    {
        for (unsigned c = 0; c < SudokuGrid::columns(); c += subgridSide)
        {
            const auto gridHasDuplicates = has_duplicate_non_empty_cells(
                        this->Grid_->subgrid_cbegin<subgridSide, subgridSide>(r, c),
                        this->Grid_->subgrid_cend<subgridSide, subgridSide>(r, c));

            if (gridHasDuplicates)
            {
                this->DuplicateType_ = "grd";
                this->DuplicateIndex_ = r * 10 + c;
                return false;
            }
        }
    }

    return true;
}

const char* Validator::type() const
{
    return this->DuplicateType_;
}

int Validator::index() const
{
    return this->DuplicateIndex_;
}
