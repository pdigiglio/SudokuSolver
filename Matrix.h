#pragma once

#include "fwd/Matrix.h"

#include "MatrixPoint.h"
#include "Tag.h"

#include <array>
#include <cassert>

template <size_t i, typename T, size_t N, typename U>
void set_data_impl(std::array<T, N>& data, U&& arg0)
{
    static_assert (i < N, "Index out of range");
    data[i] = std::forward<U>(arg0);
}

template <size_t i, typename T, size_t N, typename U>
void initialize_data_impl(std::array<T, N>& data, U&& arg0)
{
    set_data_impl<i>(data, std::forward<U>(arg0));
}

template <size_t i, typename T, size_t N, typename U, typename V, typename ... Args>
void initialize_data_impl(std::array<T, N>& data, U&& arg0, V&& arg1, Args&& ... args)
{
    set_data_impl<i>(data, std::forward<U>(arg0));
    initialize_data_impl<i + 1>(data, std::forward<V>(arg1), std::forward<Args>(args)...);
}

template <typename T, size_t N, typename U, typename ... Args>
void initialize_data(std::array<T, N>& data, U&& arg0, Args&& ... args)
{
    initialize_data_impl<0>(data, std::forward<U>(arg0), std::forward<Args>(args)...);
}

//struct MatrixRowIteratorAdvancer
//{
//    template <typename ValueType>
//    constexpr ValueType* operator()(ValueType* it) noexcept
//    {
//        return ++it;
//    }
//};
//
//template <unsigned Columns>
//struct MatrixColumnIteratorAdvancer
//{
//    template <typename ValueType>
//    constexpr ValueType* operator()(ValueType* it) noexcept
//    {
//        return it + Columns;
//    }
//};

template <unsigned Rows, unsigned Columns, unsigned SubgridRows, unsigned SubgridColumns>
struct MatrixSubgridIteratorAdvancer
{
    template <typename ValueType>
    constexpr ValueType* operator()(ValueType* it) noexcept
    {
        static_assert (SubgridColumns <= Columns, "Subgrid has more columns than Grid.");
        static_assert (SubgridRows <= Rows, "Subgrid has more columns than Grid.");

        ++i;
        const auto rowOffset = (0 == i % SubgridColumns) * (Columns - SubgridColumns);
        return it + 1 + rowOffset;
    }

private:
    unsigned i = 0;
};

template <typename ValueType, unsigned Rows, unsigned Columns, typename Advancer>
class MatrixIterator final
{
public:

    using value_type = typename std::remove_cv_t<ValueType>;
    using reference = ValueType&;
    using pointer = ValueType*;
    using difference_type = typename std::iterator_traits<pointer>::difference_type;
    using iterator_category = std::input_iterator_tag;
    using advancer = Advancer;

    constexpr explicit MatrixIterator(ValueType* it) noexcept
        : It_(it), Advancer_()
    { }

    constexpr MatrixIterator(const MatrixIterator&) noexcept = default;
    constexpr MatrixIterator(MatrixIterator&&) noexcept = default;

    constexpr MatrixIterator& operator=(const MatrixIterator&) noexcept = default;
    constexpr MatrixIterator& operator=(MatrixIterator&&) noexcept = default;

    ~MatrixIterator() noexcept = default;

    friend bool operator==(const MatrixIterator& lhs, const MatrixIterator& rhs) noexcept
    {
        return lhs.It_ == rhs.It_;
    }

    friend void swap(MatrixIterator& lhs, MatrixIterator& rhs) noexcept
    {
        using std::swap;
        swap(lhs.It_, rhs.It_);
        swap(lhs.Advancer_, rhs.Advancer_);
    }

    reference operator*() const
    {
        return *this->It_;
    }

    reference operator->() const
    {
        return *(*this);
    }

    /// @brief Pre-increment
    MatrixIterator& operator++()
    {
        this->It_ = this->Advancer_(this->It_);
        return *this;
    }

    /// @brief Post-increment
    MatrixIterator operator++(int)
    {
        auto old = *this;
        ++(*this);
        return old;
    }

private:
    ValueType* It_;
    Advancer Advancer_;
};

template <typename ValueType, unsigned Rows, unsigned Columns, typename Advancer>
bool operator!=(const MatrixIterator<ValueType, Rows, Columns, Advancer>& lhs, const MatrixIterator<ValueType, Rows, Columns, Advancer>& rhs)
{
    return !(lhs == rhs);
}

//template <unsigned Rows, unsigned Columns, typename Advancer, typename ValueType>
//MatrixIterator<ValueType, Rows, Columns, Advancer> make_matrix_iterator(ValueType* ptr)
//{
//    return MatrixIterator<ValueType, Rows, Columns, Advancer>(ptr);
//}

template <typename T, unsigned Rows, unsigned Columns>
class Matrix
{
    template <unsigned SubgridRows, unsigned SubgridColumns>
    using subgrid_it_advancer = MatrixSubgridIteratorAdvancer<Rows, Columns, SubgridRows, SubgridColumns>;

    using row_it_advancer = subgrid_it_advancer<1, Columns>;
    using column_it_advancer = subgrid_it_advancer<Rows, 1>;

    template <typename Advancer>
    using it_type = MatrixIterator<T, Rows, Columns, Advancer>;

    template <typename Advancer>
    using c_it_type = MatrixIterator<const T, Rows, Columns, Advancer>;

public:

    using value_type = typename std::remove_cv_t<T>;

    using pointer = T*;
    using const_pointer = const value_type*;

    using reference = T&;
    using const_reference = const value_type&;

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

    pointer operator[](unsigned row)
    {
        return this->get_row_begin_ptr(this->Data_.data(), row);
    }

    const_pointer operator[](unsigned row) const
    {
        return this->get_row_begin_ptr(this->Data_.data(), row);
    }

    reference operator[](const point_type& p)
    {
        assert(p.Column < Columns);
        return (*this)[p.Row][p.Column];
    }

    const_reference operator[](const point_type& p) const
    {
        assert(p.Column < Columns);
        return (*this)[p.Row][p.Column];
    }

    using iterator = typename std::array<T, Rows * Columns>::iterator;
    using const_iterator = typename std::array<T, Rows * Columns>::const_iterator;

    iterator begin()
    {
        return std::begin(this->Data_);
    }

    iterator end()
    {
        return std::end(this->Data_);
    }

    const_iterator begin() const
    {
        return std::begin(this->Data_);
    }

    const_iterator end() const
    {
        return std::end(this->Data_);
    }

    const_iterator cbegin() const
    {
        return std::cbegin(this->Data_);
    }

    const_iterator cend() const
    {
        return std::cend(this->Data_);
    }

    using row_iterator = it_type<row_it_advancer>;
    using const_row_iterator = c_it_type<row_it_advancer>;

    row_iterator row_begin(unsigned row)
    {
        return this->begin_impl<row_it_advancer>(row, 0);
    }

    row_iterator row_end(unsigned row)
    {
        return this->end_impl<row_it_advancer>(row, 0);
    }

    const_row_iterator row_begin(unsigned row) const
    {
        return this->cbegin_impl<row_it_advancer>(row, 0);
    }

    const_row_iterator row_end(unsigned row) const
    {
        return this->cend_impl<row_it_advancer>(row, 0);
    }

    const_row_iterator row_cbegin(unsigned row) const
    {
        return this->cbegin_impl<row_it_advancer>(row, 0);
    }

    const_row_iterator row_cend(unsigned row) const
    {
        return this->cend_impl<row_it_advancer>(row, 0);
    }

    using column_iterator = it_type<column_it_advancer>;
    using const_column_iterator = c_it_type<column_it_advancer>;

    column_iterator column_begin(unsigned column)
    {
        return this->begin_impl<column_it_advancer>(0, column);
    }

    column_iterator column_end(unsigned column)
    {
        return this->end_impl<column_it_advancer>(0, column);
    }

    const_column_iterator column_begin(unsigned column) const
    {
        return this->cbegin_impl<column_it_advancer>(0, column);
    }

    const_column_iterator column_end(unsigned column) const
    {
        return this->cend_impl<column_it_advancer>(0, column);
    }

    const_column_iterator column_cbegin(unsigned column) const
    {
        return this->cbegin_impl<column_it_advancer>(0, column);
    }

    const_column_iterator column_cend(unsigned column) const
    {
        return this->cend_impl<column_it_advancer>(0, column);
    }

    template <unsigned SRows, unsigned SColumns>
    using subgrid_iterator = it_type<subgrid_it_advancer<SRows, SColumns>>;

    template <unsigned SRows, unsigned SColumns>
    using const_subgrid_iterator = c_it_type<subgrid_it_advancer<SRows, SColumns>>;

    template <unsigned SRows, unsigned SColumns>
    subgrid_iterator<SRows, SColumns> subgrid_begin(unsigned row, unsigned column)
    {
        return this->begin_impl<subgrid_it_advancer<SRows, SColumns>>(row, column);
    }

    template <unsigned SRows, unsigned SColumns>
    subgrid_iterator<SRows, SColumns> subgrid_end(unsigned row, unsigned column)
    {
        return this->end_impl<subgrid_it_advancer<SRows, SColumns>>(row, column);
    }

    template <unsigned SRows, unsigned SColumns>
    const_subgrid_iterator<SRows, SColumns> subgrid_begin(unsigned row, unsigned column) const
    {
        return this->cbegin_impl<subgrid_it_advancer<SRows, SColumns>>(row, column);
    }

    template <unsigned SRows, unsigned SColumns>
    const_subgrid_iterator<SRows, SColumns> subgrid_end(unsigned row, unsigned column) const
    {
        return this->cend_impl<subgrid_it_advancer<SRows, SColumns>>(row, column);
    }

    template <unsigned SRows, unsigned SColumns>
    const_subgrid_iterator<SRows, SColumns> subgrid_cbegin(unsigned row, unsigned column) const
    {
        return this->subgrid_begin<SRows, SColumns>(row, column);
    }

    template <unsigned SRows, unsigned SColumns>
    const_subgrid_iterator<SRows, SColumns> subgrid_cend(unsigned row, unsigned column) const
    {
        return this->subgrid_end<SRows, SColumns>(row, column);
    }

    //subgrid_iterator subgrid_end(unsigned column)
    //{
    //    return this->end_impl<subgrid_it_advancer>(0, column);
    //}

    //const_subgrid_iterator subgrid_begin(unsigned column) const
    //{
    //    return this->cbegin_impl<subgrid_it_advancer>(0, column);
    //}

    //const_subgrid_iterator subgrid_end(unsigned column) const
    //{
    //    return this->cend_impl<subgrid_it_advancer>(0, column);
    //}

    //const_subgrid_iterator subgrid_cbegin(unsigned column) const
    //{
    //    return this->cbegin_impl<subgrid_it_advancer>(0, column);
    //}

    //const_subgrid_iterator subgrid_cend(unsigned column) const
    //{
    //    return this->cend_impl<subgrid_it_advancer>(0, column);
    //}

    //using row_iterator = MatrixIterator<T, Rows, Columns, MatrixRowIteratorAdvancer>;
    //using const_row_iterator = MatrixIterator<const T, Rows, Columns, MatrixRowIteratorAdvancer>;

    //row_iterator row_begin(unsigned row)
    //{
    //    return row_iterator(this->get_row_begin_ptr(this->Data_.data(), row));
    //}

    //row_iterator row_end(unsigned row)
    //{
    //    return row_iterator(this->get_row_end_ptr(this->Data_.data(), row));
    //}

    //const_row_iterator row_begin(unsigned row) const
    //{
    //    return const_row_iterator(this->get_row_begin_ptr(this->Data_.data(), row));
    //}

    //const_row_iterator row_end(unsigned row) const
    //{
    //    return const_row_iterator(this->get_row_end_ptr(this->Data_.data(), row));
    //}

    //const_row_iterator row_cbegin(unsigned row) const
    //{
    //    return this->row_begin(row);
    //}

    //const_row_iterator row_cend(unsigned row) const
    //{
    //    return this->row_end(row);
    //}

    //using column_iterator = MatrixIterator<T, Rows, Columns, MatrixColumnIteratorAdvancer<Columns>>;
    //using const_column_iterator = MatrixIterator<const T, Rows, Columns, MatrixColumnIteratorAdvancer<Columns>>;

    //column_iterator column_begin(unsigned column)
    //{
    //    return column_iterator(this->get_column_begin_ptr(this->Data_.data(), column));
    //}

    //column_iterator column_end(unsigned column)
    //{
    //    return column_iterator(this->get_column_end_ptr(this->Data_.data(), column));
    //}

    //const_column_iterator column_begin(unsigned column) const
    //{
    //    return const_column_iterator(this->get_column_begin_ptr(this->Data_.data(), column));
    //}

    //const_column_iterator column_end(unsigned column) const
    //{
    //    return const_column_iterator(this->get_column_end_ptr(this->Data_.data(), column));
    //}

    //const_column_iterator column_cbegin(unsigned column) const
    //{
    //    return this->column_begin(column);
    //}

    //const_column_iterator column_cend(unsigned column) const
    //{
    //    return this->column_end(column);
    //}

    //template <unsigned SubgridRows, unsigned SubgridColumns>
    //using subgrid_iterator = MatrixIterator<T, Rows, Columns, MatrixSubgridIteratorAdvancer<Rows, Columns, SubgridRows, SubgridColumns>>;

    //template <unsigned SubgridRows, unsigned SubgridColumns>
    //using const_subgrid_iterator = MatrixIterator<const T, Rows, Columns, MatrixSubgridIteratorAdvancer<Rows, Columns, SubgridRows, SubgridColumns>>;

    //template<unsigned SubgridRows, unsigned SubgridColumns>
    //subgrid_iterator<SubgridRows, SubgridColumns> subgrid_begin(unsigned row, unsigned column)
    //{
    //    auto* ptr = this->get_subgrid_begin_ptr<SubgridRows, SubgridColumns>(this->Data_.data(), row, column);
    //    return subgrid_iterator<SubgridRows, SubgridColumns>(ptr);
    //}

    //template<unsigned SubgridRows, unsigned SubgridColumns>
    //subgrid_iterator<SubgridRows, SubgridColumns> subgrid_end(unsigned row, unsigned column)
    //{
    //    auto* ptr = this->get_subgrid_end_ptr<SubgridRows, SubgridColumns>(this->Data_.data(), row, column);
    //    return subgrid_iterator<SubgridRows, SubgridColumns>(ptr);
    //}

    //template<unsigned SubgridRows, unsigned SubgridColumns>
    //const_subgrid_iterator<SubgridRows, SubgridColumns> subgrid_begin(unsigned row, unsigned column) const
    //{
    //    auto* ptr = this->get_subgrid_begin_ptr<SubgridRows, SubgridColumns>(this->Data_.data(), row, column);
    //    return const_subgrid_iterator<SubgridRows, SubgridColumns>(ptr);
    //}

    //template<unsigned SubgridRows, unsigned SubgridColumns>
    //const_subgrid_iterator<SubgridRows, SubgridColumns> subgrid_end(unsigned row, unsigned column) const
    //{
    //    auto* ptr = this->get_subgrid_end_ptr<SubgridRows, SubgridColumns>(this->Data_.data(), row, column);
    //    return const_subgrid_iterator<SubgridRows, SubgridColumns>(ptr);
    //}

    //template<unsigned SubgridRows, unsigned SubgridColumns>
    //const_subgrid_iterator<SubgridRows, SubgridColumns> subgrid_cbegin(unsigned row, unsigned column) const
    //{
    //    return this->subgrid_begin<SubgridRows, SubgridColumns>(row, column);
    //}

    //template<unsigned SubgridRows, unsigned SubgridColumns>
    //const_subgrid_iterator<SubgridRows, SubgridColumns> subgrid_cend(unsigned row, unsigned column) const
    //{
    //    return this->subgrid_end<SubgridRows, SubgridColumns>(row, column);
    //}

private:
    std::array<T, Rows * Columns> Data_;

    template<unsigned SRows, unsigned SColumns, typename U>
    U* get_subgrid_begin_ptr(
            subgrid_it_advancer<SRows, SColumns>,
            U* data,
            unsigned row,
            unsigned column) const
    {
        assert(row + SRows <= Rows);
        assert(column + SColumns <= Columns);
        return data + row * Columns + column;
    }

    template<unsigned SRows, unsigned SColumns, typename U>
    U* get_subgrid_end_ptr(
            subgrid_it_advancer<SRows, SColumns> advancer,
            U* data,
            unsigned row,
            unsigned column) const
    {
        return this->get_subgrid_begin_ptr(advancer, data, row, column) + SRows * Columns;
    }

    template <typename U>
    U* get_row_begin_ptr(U* data, unsigned row) const
    {
        return this->get_subgrid_begin_ptr(row_it_advancer{}, data, row, 0);
    }

    template <typename Advancer>
    it_type<Advancer> begin_impl(unsigned row, unsigned column)
    {
        auto* ptr = this->get_subgrid_begin_ptr(Advancer{}, this->Data_.data(), row, column);
        return it_type<Advancer>(ptr);
    }

    template <typename Advancer>
    c_it_type<Advancer> cbegin_impl(unsigned row, unsigned column) const
    {
        auto* ptr = this->get_subgrid_begin_ptr(Advancer{}, this->Data_.data(), row, column);
        return c_it_type<Advancer>(ptr);
    }

    template <typename Advancer>
    it_type<Advancer> end_impl(unsigned row, unsigned column)
    {
        auto* ptr = this->get_subgrid_end_ptr(Advancer{}, this->Data_.data(), row, column);
        return it_type<Advancer>(ptr);
    }

    template <typename Advancer>
    c_it_type<Advancer> cend_impl(unsigned row, unsigned column) const
    {
        auto* ptr = this->get_subgrid_end_ptr(Advancer{}, this->Data_.data(), row, column);
        return c_it_type<Advancer>(ptr);
    }

    //template <typename U>
    //U* get_row_begin_ptr(U* data, unsigned row) const
    //{
    //    assert(row < Rows);
    //    return data + row * Columns;
    //}

    //template <typename U>
    //U* get_row_end_ptr(U* data, unsigned row) const
    //{
    //    return get_row_begin_ptr(data, row) + Columns;
    //}

    //template <typename U>
    //U* get_column_begin_ptr(U* data, unsigned column) const
    //{
    //    assert(column < Columns);
    //    return data + column;
    //}

    //template <typename U>
    //U* get_column_end_ptr(U* data, unsigned column) const
    //{
    //    return this->get_column_begin_ptr(data, column) + Rows * Columns;
    //}

    //template<unsigned SubgridRows, unsigned SubgridColumns, typename U>
    //U* get_subgrid_begin_ptr(U* data, unsigned row, unsigned column) const
    //{
    //    assert(row + SubgridRows <= Rows);
    //    assert(column + SubgridColumns <= Columns);
    //    return this->get_row_begin_ptr(data, row) + column;
    //}

    //template<unsigned SubgridRows, unsigned SubgridColumns, typename U>
    //U* get_subgrid_end_ptr(U* data, unsigned row, unsigned column) const
    //{
    //    return this->get_subgrid_begin_ptr<SubgridRows, SubgridColumns>(data, row, column) + SubgridRows * Columns;
    //}
};

template <typename ... Args>
struct is_matrix : std::false_type { };

template <template <typename T, unsigned Rows, unsigned Columns> class Matrix, typename T, unsigned Rows, unsigned Columns>
struct is_matrix<Matrix<T, Rows, Columns>> : std::true_type { };

//template <typename Mat, typename Func>
//Func matrix_row_for_each(Mat& matrix, unsigned row, Func func)
//{
//    using mat_type = typename std::remove_cv<Mat>::type;
//    static_assert (is_matrix<mat_type>::value, "Not a Matrix<T, Rows, Columns> type");
//
//    assert(row < Mat::rows());
//
//    for (unsigned c = 0; c < Mat::columns(); ++c)
//    {
//        auto& element = matrix[row][c];
//        func(element);
//    }
//
//    return func;
//}
//
//template <typename Mat, typename Func>
//Func matrix_column_for_each(Mat& matrix, unsigned column, Func func)
//{
//    using mat_type = typename std::remove_cv<Mat>::type;
//    static_assert (is_matrix<mat_type>::value, "Not a Matrix<T, Rows, Columns> type");
//
//    assert(column < Mat::columns());
//
//    for (unsigned r = 0; r < Mat::rows(); ++r)
//    {
//        auto& element = matrix[r][column];
//        func(element);
//    }
//
//    return func;
//}
//
//template <unsigned GridRows, unsigned GridColumns, typename Mat, typename Func>
//Func matrix_grid_for_each(Mat& matrix, unsigned row, unsigned column, Func func)
//{
//    using mat_type = typename std::remove_cv<Mat>::type;
//    static_assert (is_matrix<mat_type>::value, "Not a Matrix<T, Rows, Columns> type");
//
//    const auto rowEnd = row + GridRows;
//    assert(rowEnd <= Mat::rows());
//
//    const auto columnEnd = column + GridColumns;
//    assert(columnEnd <= Mat::columns());
//
//    for (unsigned r = row; r < rowEnd; ++r)
//    {
//        for (unsigned c = column; c < columnEnd; ++c)
//        {
//            auto& element = matrix[r][c];
//            func(element);
//        }
//    }
//
//    return func;
//}
