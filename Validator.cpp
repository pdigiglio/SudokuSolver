#include "Validator.h"

#include "Matrix.h"
#include "StaticVector.h"

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

}

bool Validator::validate()
{
    using cell_type = SudokuGrid::value_type;
    StaticVector<cell_type, SudokuGrid::rows()> digits;

    auto digitAppender = [&digits](cell_type cellValue)
    {
        if (0 != cellValue)
        {
            digits.push_back(cellValue);
        }
    };

    for (unsigned r = 0; r < SudokuGrid::rows(); ++r)
    {
        digits.clear();
        matrix_row_for_each(*this->Grid_, r, digitAppender);
        if (has_duplicates(digits))
        {
            this->DuplicateType_ = "row";
            this->DuplicateIndex_ = r;
            return false;
        }
    }

    for (unsigned c = 0; c < SudokuGrid::columns(); ++c)
    {
        digits.clear();
        matrix_column_for_each(*this->Grid_, c, digitAppender);
        if (has_duplicates(digits))
        {
            this->DuplicateType_ = "col";
            this->DuplicateIndex_ = c;
            return false;
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
