#pragma once

enum class Compare
{
    Greater,
    Equal,
    Less
};

template <unsigned i, unsigned Value>
struct Comparer
{
    constexpr static Compare value =
        (i > Value)
        ? Compare::Greater
        : (i == Value) ? Compare::Equal : Compare::Less;
};

template <
    unsigned i,
    template <unsigned> typename IFunction,
    unsigned Value,
    Compare c>
struct EqualFinder;

template <
    unsigned i,
    template <unsigned> typename IFunction,
    unsigned Value
>
struct EqualFinder<i, IFunction, Value, Compare::Equal>
{
    constexpr static auto value = i;
};

template <
    unsigned i,
    template <unsigned> typename IFunction,
    unsigned Value>
struct EqualFinder<i, IFunction, Value, Compare::Less>
{
    constexpr static auto value =
        EqualFinder<
            i + 1,
            IFunction,
            Value,
            Comparer<IFunction<i + 1>::value, Value>::value
            >::value;
};

template <
    template <unsigned> typename IFunction,
    unsigned Value>
struct InverseEvaluator
{
    constexpr static auto value =
        EqualFinder<
            0,
            IFunction,
            Value,
            Comparer<IFunction<0>::value, Value>::value
        >::value;
};
