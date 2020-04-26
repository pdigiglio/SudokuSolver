#include "Validator.h"

#include "SudokuGrid.h"

#include <algorithm>
#include <iterator>
#include <utility>

Validator::Validator(const SudokuGrid& grid)
    : Grid_(&grid)
{ }

namespace
{

template <typename It>
std::pair<It, It> duplicate_non_empty_cells(It begin, It end)
{
    for (; begin != end; ++begin)
    {
        if (!is_empty(*begin))
        {
            auto it = std::find(std::next(begin), end, *begin);
            if (end != it)
            {
                return std::make_pair(begin, it);
            }
        }
    }

    return std::make_pair(end, end);
}

}

bool Validator::validate()
{
    for (unsigned r = 0; r < SudokuGrid::rows(); ++r)
    {
        const auto begin = this->Grid_->row_cbegin(r);
        const auto duplicates = duplicate_non_empty_cells(begin, this->Grid_->row_cend(r));
        if (duplicates.first != duplicates.second)
        {
            this->FirstDuplicate_.Row = r;
            this->FirstDuplicate_.Column = std::distance(begin, duplicates.first);

            this->SecondDuplicate_.Row = r;
            this->SecondDuplicate_.Column = std::distance(begin, duplicates.second);

            return false;
        }
    }

    for (unsigned c = 0; c < SudokuGrid::columns(); ++c)
    {
        const auto begin = this->Grid_->column_cbegin(c);
        const auto duplicates = duplicate_non_empty_cells(begin, this->Grid_->column_end(c));
        if (duplicates.first != duplicates.second)
        {
            this->FirstDuplicate_.Row = std::distance(begin, duplicates.first);
            this->FirstDuplicate_.Column = c;

            this->SecondDuplicate_.Row = std::distance(begin, duplicates.second);
            this->SecondDuplicate_.Column = c;

            return false;
        }
    }

    for (unsigned r = 0; r < SudokuGridSide; r += SudokuSubgridSide)
    {
        for (unsigned c = 0; c < SudokuGridSide; c += SudokuSubgridSide)
        {
            const auto begin = this->Grid_->subgrid_cbegin<SudokuSubgridSide, SudokuSubgridSide>(r,c);
            const auto end = this->Grid_->subgrid_cend<SudokuSubgridSide, SudokuSubgridSide>(r,c);
            const auto duplicates = duplicate_non_empty_cells(begin, end);
            if (duplicates.first != duplicates.second)
            {
                const auto i = std::distance(begin, duplicates.first);
                this->FirstDuplicate_.Row = r + i / SudokuSubgridSide;
                this->FirstDuplicate_.Column = c + i % SudokuSubgridSide;

                const auto j = std::distance(begin, duplicates.first);
                this->SecondDuplicate_.Row = r + j / SudokuSubgridSide;
                this->SecondDuplicate_.Column = c + j % SudokuSubgridSide;

                return false;
            }
        }
    }

    return true;
}

const MatrixPoint<unsigned>& Validator::firstDuplicate() const noexcept
{
    return this->FirstDuplicate_;
}

const MatrixPoint<unsigned>& Validator::secondDuplicate() const noexcept
{
    return this->SecondDuplicate_;
}
