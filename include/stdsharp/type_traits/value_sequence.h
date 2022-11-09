// Created by BlurringShadow at 2021-03-04-下午 11:27

#pragma once

#include <ranges>
#include <numeric>
#include <algorithm>
#include <functional>

#include "core_traits.h"

namespace stdsharp
{
    template<auto...>
    struct value_sequence;

    namespace details
    {
        template<auto From, auto PlusF, ::std::size_t... I>
            requires requires { regular_value_sequence<::std::invoke(PlusF, From, I)...>{}; }
        constexpr auto make_value_sequence(::std::index_sequence<I...>) noexcept
        {
            return regular_value_sequence<::std::invoke(PlusF, From, I)...>{};
        }

        struct as_value_sequence
        {
            template<typename... Constant>
            using invoke = value_sequence<Constant::value...>;
        };

        template<::std::array Array, ::std::size_t... Index>
            requires nttp_able<typename decltype(Array)::value_type>
        consteval auto array_to_sequence(const ::std::index_sequence<Index...>)
        {
            return regular_value_sequence<Array[Index]...>{};
        }

        template<
            constant_value T,
            typename Range = decltype(T::value),
            nttp_able ValueType = ::std::ranges::range_value_t<Range> // clang-format off
        > // clang-format on
            requires requires //
        {
            index_constant<::std::ranges::size(T::value)>{};
            ::std::array<ValueType, 1>{};
            requires ::std::copyable<ValueType>;
        }
        struct rng_to_sequence
        {
            static constexpr auto rng = T::value;
            static constexpr auto size = ::std::ranges::size(rng);

            static constexpr auto array = []
            {
                if constexpr( //
                    requires { array_to_sequence<rng>(::std::make_index_sequence<size>{}); } //
                )
                    return rng;
                else
                {
                    ::std::array<ValueType, size> array{};
                    ::std::ranges::copy(rng, array.begin());
                    return array;
                }
            }();

            using type =
                decltype(array_to_sequence<array>(::std::make_index_sequence<array.size()>{}));
        };
    }

    template<typename T>
    using as_value_sequence_t = ::meta::apply<details::as_value_sequence, T>;

    template<auto From, ::std::size_t Size, auto PlusF = ::std::plus{}>
    using make_value_sequence_t = decltype( //
        details::make_value_sequence<From, PlusF>(::std::make_index_sequence<Size>{})
    );

    template<typename Rng>
    using rng_to_sequence_t = typename details::rng_to_sequence<Rng>::type;

    template<auto Rng>
    using rng_v_to_sequence_t = rng_to_sequence_t<constant<Rng>>;

    namespace details
    {
        template<auto... Values>
        struct reverse_value_sequence
        {
            using seq = value_sequence<Values...>;

            using type = typename as_value_sequence_t< //
                make_value_sequence_t<
                    seq::size() - 1,
                    seq::size(),
                    ::std::minus{} // clang-format off
                >
            >::template apply_t<seq::template at_t>; // clang-format on
        };

        template<auto... Values>
        struct unique_value_sequence
        {
            using seq = value_sequence<Values...>;

            static constexpr auto unique_indices = []
            {
                ::std::array<::std::size_t, seq::size()> indices{seq::find(Values)...};

                ::std::ranges::sort(indices);

                const auto res = ::std::ranges::unique(indices);

                if(res) res.front() = seq::size();

                return indices;
            }();

            static constexpr auto unique_indices_size =
                ::std::ranges::find(unique_indices, seq::size()) - unique_indices.cbegin();

            static constexpr auto unique_indices_value = []
            {
                ::std::array<::std::size_t, unique_indices_size> value{};

                ::std::ranges::copy_n(unique_indices.cbegin(), value.size(), value.begin());

                return value;
            }();

            using type = typename as_value_sequence_t<
                rng_v_to_sequence_t<unique_indices_value> // clang-format off
            >::template apply_t<seq::template at_t>; // clang-format on
        };

        template<typename T, typename Comp>
        struct value_comparer
        {
            const T& value;
            Comp& comp;

            constexpr value_comparer(const T& value, Comp& comp) noexcept: value(value), comp(comp)
            {
            }

            template<typename U>
                requires ::std::predicate<Comp, U, T>
            constexpr auto operator()(const U& other) const noexcept(nothrow_predicate<Comp, U, T>)
            {
                return static_cast<bool>(::std::invoke(comp, other, value));
            }

            constexpr auto operator()(const auto&) const noexcept { return false; }
        };
    } // clang-format on

    template<auto... Values>
    using reverse_value_sequence_t = typename details::reverse_value_sequence<Values...>::type;

    template<auto... Values>
    using unique_value_sequence_t = typename details::unique_value_sequence<Values...>::type;

    template<auto... Values> // clang-format off
    struct value_sequence : regular_value_sequence<Values...>
    {
    public:
        using regular_value_sequence<Values...>::size;

    private:
        template<::std::size_t I>
            requires (I < size())
        [[nodiscard]] friend constexpr decltype(auto) get(const value_sequence)noexcept
        {
            return at_v<I>;
        }

    public:
        template<typename ResultType = void>
        struct invoke_fn
        {
            template<typename Func>
                requires requires //
            {
                requires(::std::invocable<Func, decltype(Values)> && ...);
                requires ::std::same_as<ResultType, void> ||
                    ::std::constructible_from< // clang-format off
                        ResultType,
                        ::std::invoke_result_t<Func, decltype(Values)>...
                    >; // clang-format on
            }
            constexpr auto operator()(Func&& func) const noexcept(
                (nothrow_invocable<Func, decltype(Values)> && ...) &&
                (::std::same_as<ResultType, void> ||
                 nothrow_constructible_from<
                     ResultType,
                     ::std::invoke_result_t<Func, decltype(Values)>... // clang-format off
                >) // clang-format on
            )
            {
                if constexpr(::std::same_as<ResultType, void>) (::std::invoke(func, Values), ...);
                else return ResultType{::std::invoke(func, Values)...};
            };
        };

        template<typename ResultType = void>
        static constexpr invoke_fn<ResultType> invoke{};

        template<template<auto...> typename T>
        using apply_t = T<Values...>;

        template<::std::size_t I>
        static constexpr decltype(auto) at_v =
            ::std::decay_t<decltype(get<I>(indexed_types<constant<Values>...>{}))>::value;

        template<::std::size_t... I>
        using at_t = regular_value_sequence<at_v<I>...>;

    private:
        template<auto... Func>
        struct transform_fn
        {
            [[nodiscard]] constexpr auto operator()() const noexcept
                requires requires //
            {
                requires(sizeof...(Func) == 1);
                typename value_sequence<::std::invoke(Func..., Values)...>;
            }
            {
                return value_sequence<::std::invoke(Func..., Values)...>{};
            };

            [[nodiscard]] constexpr auto operator()() const noexcept
                requires requires { typename value_sequence<::std::invoke(Func, Values)...>; }
            {
                return value_sequence<::std::invoke(Func, Values)...>{}; // clang-format on
            };
        };

        template<::std::size_t From, ::std::size_t... I>
        static consteval at_t<(From + I)...> select_range(::std::index_sequence<I...>) noexcept;

        template<typename IfFunc>
        struct if_not_fn
        {
            template<typename Func>
                requires ::std::invocable<IfFunc, Func>
            [[nodiscard]] constexpr auto operator()(Func&& func) const
                noexcept(nothrow_invocable<IfFunc, Func>)
            {
                return IfFunc{}(::std::not_fn(::std::forward<Func>(func)));
            }
        };

        template<typename IfFunc>
        struct from_value_fn
        {
            template<typename T, typename Comp = ::std::ranges::equal_to>
                requires ::std::invocable<IfFunc, details::value_comparer<T, Comp>>
            [[nodiscard]] constexpr auto operator()(const T& v, Comp comp = {}) const
                noexcept(nothrow_invocable<IfFunc, details::value_comparer<T, Comp>>)
            {
                return IfFunc{}(details::value_comparer<T, Comp>{v, comp});
            }
        };

        template<typename FindFunc, bool Equal = true>
        struct result_compare_to_size
        {
            template<typename Func>
                requires ::std::invocable<FindFunc, Func>
            [[nodiscard]] constexpr auto operator()(Func func) const
                noexcept(nothrow_invocable<FindFunc, Func>)
            {
                const auto v = FindFunc{}(::std::forward<Func>(func));
                if constexpr(Equal) return v == size();
                else return v != size();
            }
        };

    public:
        static constexpr struct for_each_fn
        {
            template<typename Func>
                requires(::std::invocable<Func, decltype(Values)> && ...)
            constexpr auto operator()(Func func) const
                noexcept((nothrow_invocable<Func, decltype(Values)> && ...))
            {
                (::std::invoke(func, Values), ...);
                return func;
            }
        } for_each{};

        static constexpr struct for_each_n_fn
        {
            template<typename Func>
                requires(::std::invocable<Func, decltype(Values)> && ...)
            constexpr auto operator()(auto for_each_n_count, Func func) const
                noexcept((nothrow_invocable<Func, decltype(Values)> && ...))
            {
                ((for_each_n_count == 0 ?
                      false :
                      (::std::invoke(func, Values), --for_each_n_count, true)) &&
                 ...);
                return func;
            }
        } for_each_n{};

        static constexpr struct find_if_fn
        {
            template<typename Func>
                requires(::std::predicate<Func, decltype(Values)> && ...)
            [[nodiscard]] constexpr auto operator()(Func func) const
                noexcept((nothrow_predicate<Func, decltype(Values)> && ...))
            {
                ::std::size_t i = 0;
                empty =
                    ((static_cast<bool>(::std::invoke(func, Values)) ? false : (++i, true)) && ...);
                return i;
            }
        } find_if{};

        using find_if_not_fn = if_not_fn<find_if_fn>;

        static constexpr find_if_not_fn find_if_not{};

        using find_fn = from_value_fn<find_if_fn>;

        static constexpr find_fn find{};

        static constexpr struct count_if_fn
        {
            template<typename Func>
                requires(::std::predicate<Func, decltype(Values)> && ...)
            [[nodiscard]] constexpr auto operator()(Func func) const
                noexcept((nothrow_predicate<Func, decltype(Values)> && ...))
            {
                ::std::size_t i = 0;
                for_each(
                    [&i,
                     &func](const auto& v) noexcept(nothrow_invocable_r<Func, bool, decltype(v)> //
                    )
                    {
                        if(static_cast<bool>(::std::invoke(func, v))) ++i;
                        return true;
                    } //
                );

                return i;
            }
        } count_if{};

        using count_if_not_fn = if_not_fn<count_if_fn>;

        static constexpr count_if_not_fn count_if_not{};

        using count_fn = from_value_fn<count_if_fn>;

        static constexpr count_fn count{};

        static constexpr result_compare_to_size<find_if_not_fn> all_of{};

        static constexpr result_compare_to_size<find_if_fn> none_of{};

        static constexpr result_compare_to_size<find_if_fn, false> any_of{};

        static constexpr result_compare_to_size<find_fn, false> contains{};

        static constexpr struct adjacent_find_fn
        {
        private:
            template<::std::size_t I>
            struct by_index
            {
                template<typename Comp>
                    requires ::std::predicate<Comp>
                constexpr auto operator()(Comp& comp) const noexcept(nothrow_predicate<Comp, bool>)
                {
                    return static_cast<bool>(::std::invoke(comp, at_v<I>, at_v<I + 1>));
                }

                constexpr auto operator()(const auto&, const auto&) const noexcept { return false; }
            };

            struct impl
            {
                template<typename Comp, ::std::size_t... I>
                constexpr auto operator()(Comp& comp, const ::std::index_sequence<I...>) const
                    noexcept((nothrow_invocable<by_index<I>, Comp> && ...))
                {
                    ::std::size_t res{};
                    ( //
                        (by_index<I>{}(comp) ? true : (++res, false)) || ... //
                    );
                    return res;
                }

                template<typename Comp, typename Seq = ::std::make_index_sequence<size() - 2>>
                constexpr auto operator()(Comp& comp) const
                    noexcept(nothrow_invocable<impl, Comp&, Seq>)
                {
                    return (*this)(comp, Seq{});
                }
            };

        public:
            template<typename Comp = ::std::ranges::equal_to>
                requires((::std::invocable<decltype(Values)> && ...) && size() >= 2)
            [[nodiscard]] constexpr auto operator()(Comp comp = {}) const
                noexcept(nothrow_invocable<impl, Comp>)
            {
                return impl{}(comp);
            }

            [[nodiscard]] constexpr auto operator()(const int = {}) const noexcept
            {
                return size();
            }
        } adjacent_find{};

        template<auto... Func>
        static constexpr transform_fn<Func...> transform{};

        template<auto... Func>
            requires ::std::invocable<transform_fn<Func...>>
        using transform_t = decltype(transform<Func...>());

        template<::std::size_t From, ::std::size_t Size>
        using select_range_t = typename as_value_sequence_t<make_value_sequence_t<From, Size>>:: //
            template apply_t<at_t>;

        template<::std::size_t Size>
        using back_t = select_range_t<size() - Size, Size>;

        template<::std::size_t Size>
        using front_t = select_range_t<0, Size>;

        template<auto... Others>
        using append_t = regular_value_sequence<Values..., Others...>;

        template<auto... Others>
        using append_front_t = regular_value_sequence<Others..., Values...>;

    private:
        template<::std::size_t Index>
        struct insert
        {
            template<auto... Others, auto... Front, auto... Back>
            static consteval regular_value_sequence<Front..., Others..., Back...> impl(
                const regular_value_sequence<Front...>,
                const regular_value_sequence<Back...> //
            )
            {
                return {};
            }

            template<auto... Others>
            using type = decltype(impl<Others...>(front_t<Index>{}, back_t<size() - Index>{}));
        };

        template<::std::size_t... Index>
        struct remove_at
        {
            static constexpr ::std::array select_indices = []
            {
                constexpr ::std::pair pair = []
                {
                    ::std::array<::std::size_t, size()> res{};
                    ::std::array excepted = {Index...};

                    ::std::ranges::sort(excepted);

                    const auto size = static_cast<::std::size_t>(
                        ::std::ranges::set_difference(
                            ::std::views::iota(::std::size_t{0}, res.size()),
                            excepted,
                            res.begin()
                        )
                            .out -
                        res.cbegin()
                    );

                    return ::std::pair{res, size};
                }();

                ::std::array<::std::size_t, pair.second> res{};
                ::std::ranges::copy_n(pair.first.cbegin(), pair.second, res.begin());

                return res;
            }();

            template<::std::size_t... I>
            static constexpr at_t<select_indices[I]...>
                get_type(::std::index_sequence<I...>) noexcept;

            using type = decltype(get_type(::std::make_index_sequence<select_indices.size()>{}));
        };

    public:
        template<::std::size_t Index, auto... Other>
        using insert_t = typename insert<Index>::template type<Other...>;

        template<::std::size_t... Index>
        using remove_at_t = typename remove_at<Index...>::type;

        template<::std::size_t Index, auto Other>
        using replace_t = typename as_value_sequence_t<
            typename as_value_sequence_t<front_t<Index>>::template append_t<Other>
            // clang-format off
        >::template append_by_seq_t<back_t<size() - Index - 1>>; // clang-format on
    };
}

namespace std
{
    template<auto... Values>
    struct tuple_size<::stdsharp::value_sequence<Values...>> :// NOLINT(cert-dcl58-cpp)
        ::stdsharp:: // clang-format off
            index_constant<::stdsharp::value_sequence<Values...>::size()>
    // clang-format on
    {
    };
}