#pragma once

#include <array>
#include <cassert>

template <typename T, size_t Capacity>
class StaticVector
{
public:
    explicit StaticVector()
        : Data_(), Size_(0)
    { }

    const T& operator[](size_t i) const
    {
        assert(i < this->size());
        return this->Data_[i];
    }

    T& operator[](size_t i)
    {
        assert(i < this->size());
        return this->Data_[i];
    }

    size_t size() const
    {
        return this->Size_;
    }

    bool empty() const
    {
        return 0 == this->size();
    }

    static constexpr size_t capacity()
    {
        return Capacity;
    }

    void clear()
    {
        this->Size_ = 0;
    }

    void push_back(T v)
    {
        this->Data_[this->Size_] = v;
        ++this->Size_;
    }

    using iterator = typename std::array<T, Capacity>::iterator;
    using const_iterator = typename std::array<T, Capacity>::const_iterator;
    using value_type = T;

    iterator erase(const_iterator it)
    {
        const auto position = std::distance(std::cbegin(*this), it);
        assert(position >= 0);
        assert(static_cast<size_t>(position) <= this->size());

        auto targetBegin = std::next(std::begin(*this), position);
        const auto srcLast = std::cend(*this);
        if (it != srcLast)
        {
            const auto srcBegin = std::next(it);
            std::copy(srcBegin, srcLast, targetBegin);
            --this->Size_;
        }

        return targetBegin;
    }

    iterator begin()
    {
        return std::begin(this->Data_);
    }

    iterator end()
    {
        return std::next(this->begin(), this->size());
    }

    const_iterator begin() const
    {
        return std::begin(this->Data_);
    }

    const_iterator end() const
    {
        return std::next(this->begin(), this->size());
    }

    const_iterator cbegin() const
    {
        return this->begin();
    }

    const_iterator cend() const
    {
        return this->end();
    }

private:
    std::array<T, Capacity> Data_;
    size_t Size_;
};
