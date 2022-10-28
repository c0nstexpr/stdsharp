// Created by BlurringShadow at 2021-03-04-下午 11:27

#pragma once

#include <ranges>
#include <numeric>
#include <algorithm>

#include "../functional/invoke.h"

namespace stdsharp::type_traits
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
    }

    template<typename T>
    using as_value_sequence_t = ::meta::apply<details::as_value_sequence, T>;

    template<auto From, ::std::size_t Size, auto PlusF = ::std::plus{}>
    using make_value_sequence_t = decltype( //
        details::make_value_sequence<From, PlusF>(::std::make_index_sequence<Size>{}) //
    );

    namespace details
    {
        template<auto... Values>
        struct reverse_value_sequence
        {
            using seq = value_sequence<Values...>;
            using type = typename seq:: //
                template indexed_by_seq_t< //
                    make_value_sequence_t<
                        seq::size() - 1,
                        seq::size(),
                        ::std::minus{} // clang-format off
                    >
            >; // clang-format on
        };

        template<auto... Values>
        struct unique_value_sequence
        {
            using seq = value_sequence<Values...>;

            static constexpr auto filtered_indices = []
            {
                ::std::array<::std::size_t, unique_value_sequence::seq::size()> res{
                    unique_value_sequence::seq::find(Values)... //
                };
                ::std::ranges::sort(res);

                // TODO: replace with ranges algorithm
                return ::std::pair{res, ::std::unique(res.begin(), res.end()) - res.cbegin()};
            }();

            template<::std::size_t... I>
            using filtered_seq = regular_value_sequence< //
                get<filtered_indices.first[I]>(unique_value_sequence::seq{})... // clang-format off
            >; // clang-format on

            using type = typename as_value_sequence_t< //
                make_value_sequence_t<::std::size_t{}, filtered_indices.second> // clang-format off
            >::template apply_t<filtered_seq>; // clang-format on
        };

        template<::std::size_t I, auto Value>
        struct indexed_value : constant<Value>
        {
            template<::std::size_t J>
                requires(I == J)
            static constexpr decltype(auto) get() noexcept
            {
                return Value;
            }
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
            constexpr auto operator()(const U& other) const
                noexcept(concepts::nothrow_predicate<Comp, U, T>)
            {
                return functional::invoke_r<bool>(comp, other, value);
            }

            constexpr auto operator()(const auto&) const noexcept { return false; }
        };

        template<typename T, typename Comp>
        constexpr auto make_value_comparer(const T& value, Comp& comp = {}) noexcept
        {
            return value_comparer<T, Comp>(value, comp);
        }
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
        struct tag{};

        template<::std::size_t I>
            requires (I < size())
        [[nodiscard]] friend constexpr decltype(auto) get(const value_sequence)noexcept
        {
            return get_impl<I>(tag{});
        }

        template<::std::size_t I>
            requires (I < size())
        friend constexpr decltype(auto) get_impl(const tag) noexcept
        {
            return ::std::decay_t<decltype(get<I>(indexed_types<constant<Values>...>{}))>::value;
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
                (concepts::nothrow_invocable<Func, decltype(Values)> && ...) &&
                (::std::same_as<ResultType, void> ||
                 concepts::nothrow_constructible_from<
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

        template<::std::size_t... OtherInts>
        using indexed_t = regular_value_sequence<get_impl<OtherInts>(tag{})...>;

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
        static constexpr indexed_t<From + I...>
            select_range_indexed(::std::index_sequence<I...>) noexcept;

        template<typename IfFunc>
        struct if_not_fn : private IfFunc
        {
            template<typename Func>
                requires ::std::invocable<IfFunc, Func>
            [[nodiscard]] constexpr auto operator()(Func func) const
                noexcept(concepts::nothrow_invocable<IfFunc, Func>)
            {
                return IfFunc::operator()(func);
            }
        };

        template<typename IfFunc>
        struct do_fn : private IfFunc
        {
            template<typename T, typename Comp = ::std::ranges::equal_to>
                requires ::std::invocable<IfFunc, details::value_comparer<T, Comp>>
            [[nodiscard]] constexpr auto operator()(const T& v, Comp comp = {}) const
                noexcept(concepts::nothrow_invocable<IfFunc, details::value_comparer<T, Comp>>)
            {
                return IfFunc::operator()(details::make_value_comparer(v, comp));
            }
        };

        template<typename FindFunc, bool Equal = true>
        struct algo_fn : private FindFunc
        {
            template<typename Func>
                requires ::std::invocable<FindFunc, Func>
            [[nodiscard]] constexpr auto operator()(Func func) const
                noexcept(concepts::nothrow_invocable<FindFunc, Func>)
            {
                const auto v = FindFunc::operator()(::std::forward<Func>(func));
                if constexpr(Equal) return v == size();
                else return v != size();
            }
        };

    public:
        static constexpr struct
        {
            template<typename Func>
                requires(::std::invocable<Func, decltype(Values)> && ...)
            constexpr auto operator()(Func func) const
                noexcept((concepts::nothrow_invocable<Func, decltype(Values)> && ...))
            {
                (::std::invoke(func, Values), ...);
                return func;
            }
        } for_each{};

        static constexpr struct
        {
            template<typename Func>
                requires(::std::invocable<Func, decltype(Values)> && ...)
            constexpr auto operator()(auto for_each_n_count, Func func) const
                noexcept((concepts::nothrow_invocable<Func, decltype(Values)> && ...))
            {
                ((for_each_n_count == 0 ?
                      false :
                      (::std::invoke(func, Values), --for_each_n_count, true)) &&
                 ...);
                return func;
            }
        } for_each_n{};

        static constexpr struct
        {
            template<typename Func>
                requires(::std::predicate<Func, decltype(Values)> && ...)
            [[nodiscard]] constexpr auto operator()(Func func) const
                noexcept((concepts::nothrow_predicate<Func, decltype(Values)> && ...))
            {
                ::std::size_t i = 0;
                type_traits::empty =
                    ((functional::invoke_r<bool>(func, Values) ? false : (++i, true)) && ...);
                return i;
            }
        } find_if{};

        static constexpr value_sequence::if_not_fn<decltype(find_if)> find_if_not{};

        static constexpr value_sequence::do_fn<decltype(find_if)> find{};

        static constexpr struct
        {
            template<typename Func>
                requires(::std::predicate<Func, decltype(Values)> && ...)
            [[nodiscard]] constexpr auto operator()(Func func) const
                noexcept((concepts::nothrow_predicate<Func, decltype(Values)> && ...))
            {
                ::std::size_t i = 0;
                for_each(
                    [&i, &func](const auto& v
                    ) noexcept(concepts::nothrow_invocable_r<Func, bool, decltype(v)> //
                    )
                    {
                        if(functional::invoke_r<bool>(func, v)) ++i;
                        return true;
                    } //
                );

                return i;
            }
        } count_if{};

        static constexpr value_sequence::if_not_fn<decltype(count_if)> count_if_not{};

        static constexpr value_sequence::do_fn<decltype(count_if)> count{};

        static constexpr value_sequence::algo_fn<decltype(find_if_not)> all_of{};

        static constexpr value_sequence::algo_fn<decltype(find_if), false> any_of{};

        static constexpr value_sequence::algo_fn<decltype(find_if)> none_of{};

        static constexpr value_sequence::algo_fn<decltype(find), false> contains{};

        static constexpr struct
        {
        private:
            template<::std::size_t I>
            struct by_index
            {
                template<typename Comp>
                    requires ::std::predicate<Comp>
                constexpr auto operator()(Comp& comp) const
                    noexcept(concepts::nothrow_predicate<Comp, bool>)
                {
                    return functional::invoke_r<bool>(comp, get_impl<I>(), get_impl<I + 1>());
                }

                constexpr auto operator()(const auto&, const auto&) const noexcept { return false; }
            };

            struct impl
            {
                template<typename Comp, ::std::size_t... I>
                constexpr auto operator()(Comp& comp, const ::std::index_sequence<I...>) const
                    noexcept((concepts::nothrow_invocable<by_index<I>, Comp> && ...))
                {
                    ::std::size_t res{};
                    ( //
                        (by_index<I>{}(comp) ? true : (++res, false)) || ... //
                    );
                    return res;
                }

                template<typename Comp, typename Seq = ::std::make_index_sequence<size() - 2>>
                constexpr auto operator()(Comp& comp) const
                    noexcept(concepts::nothrow_invocable<impl, Comp&, Seq>)
                {
                    return (*this)(comp, Seq{});
                }
            };

        public:
            template<typename Comp = ::std::ranges::equal_to>
                requires((::std::invocable<decltype(Values)> && ...) && size() >= 2)
            [[nodiscard]] constexpr auto operator()(Comp comp = {}) const
                noexcept(concepts::nothrow_invocable<impl, Comp>)
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
        using select_range_indexed_t = decltype( //
            value_sequence::select_range_indexed<From>( //
                ::std::make_index_sequence<Size>{} // clang-format off
            ) // clang-format on
        );

        template<::std::size_t Size>
        using back_t = select_range_indexed_t<size() - Size, Size>;

        template<::std::size_t Size>
        using front_t = select_range_indexed_t<0, Size>;

        template<auto... Others>
        using append_t = regular_value_sequence<Values..., Others...>;

        template<auto... Others>
        using append_front_t = regular_value_sequence<Others..., Values...>;

    private:
        template<::std::size_t Index>
        struct insert
        {
            template<auto... Others, auto... Front, auto... Back>
            static constexpr regular_value_sequence<Front..., Others..., Back...> impl(
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
            static constexpr indexed_t<select_indices[I]...>
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
    struct tuple_size<::stdsharp::type_traits::value_sequence<Values...>> :// NOLINT(cert-dcl58-cpp)
        ::stdsharp::type_traits:: // clang-format off
            index_constant<::stdsharp::type_traits::value_sequence<Values...>::size()>
    // clang-format on
    {
    };
}