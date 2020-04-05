#pragma once

#include "fwd/Matrix.h"

#include "MatrixPoint.h"

#include <array>
#include <cassert>

template <size_t i, typename T, size_t Columns, size_t Rows, typename U>
void set_data_impl(std::array<std::array<T, Columns>, Rows>& data, U&& arg0)
{
    static_assert (i < Columns * Rows, "Index out of range");

    constexpr auto row = i / Columns;
    constexpr auto column = i % Columns;

    data[row][column] = std::forward<U>(arg0);
}

template <size_t i, typename T, size_t Columns, size_t Rows, typename U>
void initialize_data_impl(std::array<std::array<T, Columns>, Rows>& data, U&& arg0)
{
    set_data_impl<i>(data, std::forward<U>(arg0));
}

template <size_t i, typename T, size_t Columns, size_t Rows, typename U, typename V, typename ... Args>
void initialize_data_impl(std::array<std::array<T, Columns>, Rows>& data, U&& arg0, V&& arg1, Args&& ... args)
{
    set_data_impl<i>(data, std::forward<U>(arg0));
    initialize_data_impl<i + 1>(data, std::forward<V>(arg1), std::forward<Args>(args)...);
}

template <typename T, size_t Columns, size_t Rows, typename U, typename ... Args>
void initialize_data(std::array<std::array<T, Columns>, Rows>& data, U&& arg0, Args&& ... args)
{
    initialize_data_impl<0>(data, std::forward<U>(arg0), std::forward<Args>(args)...);
}

template <typename T, unsigned Rows, unsigned Columns>
class Matrix final
{
public:
    using value_type = T;

    using iterator = value_type*;
    using const_iterator = const value_type*;

    using reference = value_type&;
    using const_reference = const value_type&;

    using row_type = std::array<T, Columns>;

    using row_reference = row_type&;
    using const_row_reference = const row_type&;

    using point_type = MatrixPoint<unsigned>;

    constexpr static unsigned size() noexcept
    {
        return Matrix::rows() * Matrix::columns();
    }

    constexpr static unsigned rows() noexcept
    {
        return Rows;
    }

    constexpr static unsigned columns() noexcept
    {
        return Columns;
    }

    template <typename U, typename ... Args>
    explicit Matrix(U&& arg0, Args&& ... args)
        :
          Data_({ })
    {
        initialize_data(this->Data_, std::forward<U>(arg0), std::forward<Args>(args)...);
    }

    explicit Matrix() = default;

    Matrix(const Matrix&) = default;
    Matrix(Matrix&&) = default;

    Matrix& operator=(const Matrix&) = default;
    Matrix& operator=(Matrix&&) = default;

    ~Matrix() = default;

    row_reference operator[](unsigned i)
    {
        assert(i < Columns);
        return this->Data_[i];
    }

    const_row_reference operator[](unsigned i) const
    {
        assert(i < Columns);
        return this->Data_[i];
    }

    reference operator[](const point_type& p)
    {
        (*this)[p.Row][p.Column];
    }

    const_reference operator[](const point_type& p) const
    {
        (*this)[p.Row][p.Column];
    }

private:
    std::array<std::array<T, Columns>, Rows> Data_;
};

template <typename ... Args>
struct is_matrix : std::false_type { };

template <template <typename T, unsigned Rows, unsigned Columns> class Matrix, typename T, unsigned Rows, unsigned Columns>
struct is_matrix<Matrix<T, Rows, Columns>> : std::true_type { };

template <typename Mat, typename Func>
Func matrix_row_for_each(Mat& matrix, unsigned row, Func func)
{
    using mat_type = typename std::remove_cv<Mat>::type;
    static_assert (is_matrix<mat_type>::value, "Not a Matrix<T, Rows, Columns> type");

    assert(row < Mat::rows());

    for (unsigned c = 0; c < Mat::columns(); ++c)
    {
        auto& element = matrix[row][c];
        func(element);
    }

    return func;
}

template <typename Mat, typename Func>
Func matrix_column_for_each(Mat& matrix, unsigned column, Func func)
{
    using mat_type = typename std::remove_cv<Mat>::type;
    static_assert (is_matrix<mat_type>::value, "Not a Matrix<T, Rows, Columns> type");

    assert(column < Mat::columns());

    for (unsigned r = 0; r < Mat::rows(); ++r)
    {
        auto& element = matrix[r][column];
        func(element);
    }

    return func;
}

template <unsigned GridRows, unsigned GridColumns, typename Mat, typename Func>
Func matrix_grid_for_each(Mat& matrix, unsigned row, unsigned column, Func func)
{
    using mat_type = typename std::remove_cv<Mat>::type;
    static_assert (is_matrix<mat_type>::value, "Not a Matrix<T, Rows, Columns> type");

    const auto rowEnd = row + GridRows;
    assert(rowEnd <= Mat::rows());

    const auto columnEnd = column + GridColumns;
    assert(columnEnd <= Mat::columns());

    for (unsigned r = row; r < rowEnd; ++r)
    {
        for (unsigned c = column; c < columnEnd; ++c)
        {
            auto& element = matrix[r][c];
            func(element);
        }
    }

    return func;
}

bool fill_from_input_file(const char* filePath, SudokuGrid& grid);
void print_grid(const SudokuGrid& grid);
