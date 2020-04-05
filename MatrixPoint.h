#pragma once

#include <type_traits>

template <typename Integer>
struct MatrixPoint
{
    constexpr explicit MatrixPoint(Integer r = 0, Integer c = 0) noexcept
        :
          Row(r),
          Column(c)
    {
        static_assert (std::is_integral<Integer>::value, "Integer is not an integral type");
    }

    MatrixPoint(const MatrixPoint&) = default;
    MatrixPoint(MatrixPoint&&) = default;

    MatrixPoint& operator=(const MatrixPoint&) = default;
    MatrixPoint& operator=(MatrixPoint&&) = default;

    ~MatrixPoint() = default;

    Integer Row;
    Integer Column;
};
