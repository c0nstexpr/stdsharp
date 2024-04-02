
#pragma once

#include "../functional/always_return.h"
#include "indexed_traits.h"

#include <algorithm>

namespace stdsharp
{
    template<auto...>
    struct value_sequence;

    template<typename T>
    using to_value_sequence = decltype( //
        []<template<auto...> typename Inner, auto... Values>(const Inner<Values...>)
        {
            return value_sequence<Values...>{}; //
        }(std::declval<T>())
    );
}

namespace stdsharp::details
{
    template<typename T, typename Comp>
    struct value_comparer
    {
        const T& value;
        Comp& comp;

        constexpr value_comparer(const T& value, Comp& comp) noexcept: value(value), comp(comp) {}

        template<typename U>
            requires std::predicate<Comp, U, T>
        constexpr bool operator()(const U& other) const noexcept(nothrow_predicate<Comp, U, T>)
        {
            return invoke(comp, other, value);
        }

        constexpr auto operator()(const auto& /*unused*/) const noexcept { return false; }
    };
}

namespace stdsharp
{
    template<auto... Values>
    struct value_sequence : regular_value_sequence<Values...>
    {
    public:
        using regular_value_sequence<Values...>::size;

    private:
        static constexpr indexed_values values{Values...};

        using values_t = std::decay_t<decltype(values)>;

        template<std::size_t I>
            requires requires { requires I < size(); }
        [[nodiscard]] friend constexpr decltype(auto) get(const value_sequence /*unused*/) noexcept
        {
            return value<I>;
        }

        template<typename seq = value_sequence<Values...>>
        static consteval to_value_sequence<
            make_value_sequence_t<seq::size() - 1, seq::size(), std::minus{}>>::
            template apply_t<seq::template at_t>
            get_reverse();

        template<typename Func>
            requires(std::invocable<Func&, decltype(Values)> && ...)
        static consteval void invocable_test();

        template<typename Func>
        static constexpr auto nothrow_invocable_v =
            (nothrow_invocable<Func&, decltype(Values)> && ...);

        template<typename Func>
            requires(std::predicate<Func&, const decltype(Values)&> && ...)
        static consteval void predicate_test();

        template<typename Func>
        static constexpr auto nothrow_predicate_v =
            (nothrow_predicate<Func&, const decltype(Values)&> && ...);

    public:
        using reverse_t = decltype(get_reverse<Values...>());

        template<typename ResultType = void>
        struct invoke_fn
        {
            template<typename Func>
                requires requires {
                    invocable_test<Func>();
                    requires std::same_as<ResultType, void>;
                }
            constexpr auto operator()(Func&& func) const
                noexcept(nothrow_invocable_v<Func> && std::same_as<ResultType, void>)
            {
                (stdsharp::invoke(func, Values), ...);
            };

            template<typename Func>
                requires requires {
                    invocable_test<Func>();
                    requires std::constructible_from<
                        ResultType,
                        std::invoke_result_t<Func, decltype(Values)>...>;
                }
            constexpr auto operator()(Func&& func) const noexcept( //
                nothrow_invocable_v<Func> &&
                nothrow_constructible_from<
                    ResultType,
                    std::invoke_result_t<Func, decltype(Values)>...> //
            )
            {
                return ResultType{stdsharp::invoke(func, Values)...};
            };
        };

        template<typename ResultType = void>
        static constexpr invoke_fn<ResultType> invoke{};

        template<template<auto...> typename T>
        using apply_t = T<Values...>;

        template<std::size_t I>
        using value_type = std::tuple_element_t<I, values_t>;

        template<std::size_t I>
        static constexpr value_type<I> value = values.template get<I>();

        template<std::size_t... I>
        using at_t = regular_value_sequence<value<I>...>;

        template<auto... Func>
        struct transform_fn
        {
            [[nodiscard]] constexpr value_sequence<stdsharp::invoke(Func..., Values)...>
                operator()() const noexcept
                requires(sizeof...(Func) == 1)
            {
                return {};
            };

            [[nodiscard]] constexpr value_sequence<stdsharp::invoke(Func, Values)...>
                operator()() const noexcept
            {
                return {};
            };
        };

        template<auto... Func>
        static constexpr transform_fn<Func...> transform{};

        template<auto... Func>
            requires std::invocable<transform_fn<Func...>>
        using transform_t = decltype(transform<Func...>());

    private:
        template<std::size_t From, std::size_t... I>
        static consteval at_t<(From + I)...> select_range(std::index_sequence<I...>) noexcept;

        template<typename IfFunc>
        struct if_not_fn
        {
            template<typename Func>
                requires std::invocable<IfFunc, Func>
            [[nodiscard]] constexpr auto operator()(Func&& func) const
                noexcept(nothrow_invocable<IfFunc, Func>)
            {
                return IfFunc{}(std::not_fn(cpp_forward(func)));
            }
        };

        template<typename IfFunc>
        struct from_value_fn
        {
            template<typename T, typename Comp = std::ranges::equal_to>
                requires std::invocable<IfFunc, details::value_comparer<T, Comp>>
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
                requires std::invocable<FindFunc, Func>
            [[nodiscard]] constexpr auto operator()(Func&& func) const
                noexcept(nothrow_invocable<FindFunc, Func>)
            {
                const auto v = FindFunc{}(cpp_forward(func));
                if constexpr(Equal) return v == size();
                else return v != size();
            }
        };

    public: // TODO: P1306 Expansion statements
        static constexpr struct for_each_fn
        {
            template<typename Func, typename Condition = always_true_fn>
                requires requires {
                    invocable_test<Func>();
                    predicate_test<Condition>();
                }
            constexpr auto operator()(Func func, Condition condition = {}) const
                noexcept(nothrow_invocable_v<Func> && nothrow_predicate_v<Condition>)
            {
                ((invoke_r<bool>(condition, Values) && (stdsharp::invoke(func, Values), true)) &&
                 ...);
                return func;
            }
        } for_each{};

        static constexpr struct find_if_fn
        {
            template<typename Func>
                requires requires { predicate_test<Func>(); }
            [[nodiscard]] constexpr auto operator()(Func func) const
                noexcept(nothrow_predicate_v<Func>)
            {
                std::size_t i = 0;

                for_each(
                    [&func](const auto& v) noexcept(nothrow_predicate<Func, decltype(v)>)
                    {
                        return !invoke_r<bool>(func, v); //
                    },
                    [&i](const auto& /*unused*/) noexcept { ++i; }
                );

                return i;
            }
        } find_if{};

        static constexpr struct for_each_n_fn
        {
            template<typename Func>
                requires requires { invocable_test<Func>(); }
            constexpr auto operator()(auto count, Func func) const
                noexcept(nothrow_invocable_v<Func>)
            {
                for_each(
                    [&count](const auto& /*unused*/) noexcept { return count > 0; },
                    [&count, &func](const auto& v) noexcept(nothrow_invocable<Func, decltype(v)>)
                    {
                        stdsharp::invoke(func, v);
                        --count;
                    }
                );
            }
        } for_each_n{};

        using find_if_not_fn = if_not_fn<find_if_fn>;

        static constexpr find_if_not_fn find_if_not{};

        using find_fn = from_value_fn<find_if_fn>;

        static constexpr find_fn find{};

        static constexpr struct count_if_fn
        {
            template<typename Func>
                requires requires { predicate_test<Func>(); }
            [[nodiscard]] constexpr auto operator()(Func func) const
                noexcept(nothrow_predicate_v<Func>)
            {
                std::size_t i = 0;
                for_each(
                    [&i, &func](const auto& v) noexcept(nothrow_predicate<Func, decltype(v)>)
                    {
                        if(invoke_r<bool>(func, v)) ++i;
                    }
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
            template<std::size_t I>
            struct by_index
            {
                template<std::predicate<value_type<I>, value_type<I + 1>> Comp>
                constexpr bool operator()(Comp& comp) const
                    noexcept(nothrow_predicate<Comp, value_type<I>, value_type<I + 1>>)
                {
                    return invoke_r<bool>(comp, value<I>, value<I + 1>);
                }

                constexpr auto
                    operator()(const auto& /*unused*/, const auto& /*unused*/) const noexcept
                {
                    return false;
                }
            };

            template<typename Comp, std::size_t... I>
            static constexpr auto impl(Comp& comp, const std::index_sequence<I...> /*unused*/)
                noexcept((nothrow_invocable<by_index<I>, Comp> && ...))
            {
                std::size_t res{};
                ((by_index<I>{}(comp) || (++res, false)) || ...);
                return res;
            }

        public:
            template<
                typename Comp = std::ranges::equal_to,
                typename Seq = std::make_index_sequence<size() - 2>>
            [[nodiscard]] constexpr auto operator()(Comp comp = {}) const
                noexcept(noexcept(impl(comp, Seq{})))
                requires requires {
                    requires size() >= 2;
                    impl(comp, Seq{});
                }
            {
                return impl(comp, Seq{});
            }

            [[nodiscard]] constexpr auto operator()(const int /*unused*/ = {}) const noexcept
            {
                return size();
            }
        } adjacent_find{};

        template<std::size_t From, std::size_t Size>
        using select_range_t =
            to_value_sequence<make_value_sequence_t<From, Size>>::template apply_t<at_t>;

        template<std::size_t Size>
        using back_t = select_range_t<size() - Size, Size>;

        template<std::size_t Size>
        using front_t = select_range_t<0, Size>;

        template<auto... Others>
        using append_t = regular_value_sequence<Values..., Others...>;

        template<auto... Others>
        using append_front_t = regular_value_sequence<Others..., Values...>;

    private:
        template<std::size_t Index>
        struct insert
        {
            template<auto... Others, auto... Front, auto... Back>
            static consteval regular_value_sequence<Front..., Others..., Back...> impl(
                const regular_value_sequence<Front...> /*unused*/,
                const regular_value_sequence<Back...> /*unused*/
            )
            {
                return {};
            }

            template<auto... Others>
            using type = decltype(impl<Others...>(front_t<Index>{}, back_t<size() - Index>{}));
        };

        template<std::size_t... Index>
        struct remove_at
        {
            static constexpr std::array select_indices = []
            {
                constexpr std::pair pair = []
                {
                    std::array<std::size_t, size()> res{};
                    std::array excepted = {Index...};

                    std::ranges::sort(excepted);

                    const auto size = static_cast<std::size_t>(
                        std::ranges::set_difference(
                            std::views::iota(std::size_t{0}, res.size()),
                            excepted,
                            res.begin()
                        )
                            .out -
                        res.cbegin()
                    );

                    return std::pair{res, size};
                }();

                std::array<std::size_t, pair.second> res{};
                std::ranges::copy_n(pair.first.cbegin(), pair.second, res.begin());

                return res;
            }();

            template<std::size_t... I>
            static constexpr at_t<select_indices[I]...>
                impl(std::index_sequence<I...>) noexcept;

            using type = decltype(impl(std::make_index_sequence<select_indices.size()>{}));
        };

        template<typename Comp>
        struct unique_value_sequence
        {
            static constexpr auto unique_indices_value = []
            {
                constexpr auto unique_indices = []
                {
                    std::array<std::size_t, size()> indices{find(Comp{}, Values)...};

                    std::ranges::sort(indices);

                    if(const auto res = std::ranges::unique(indices); res) res.back() = size();

                    return indices;
                }();

                constexpr auto unique_indices_size =
                    std::ranges::find(unique_indices, size()) - unique_indices.cbegin();

                std::array<std::size_t, unique_indices_size> value{};

                std::ranges::copy_n(unique_indices.cbegin(), value.size(), value.begin());

                return value;
            }();

            using type =
                to_value_sequence<rng_v_to_sequence<unique_indices_value>>::template apply_t<at_t>;
        };

    public:
        template<std::size_t Index, auto... Other>
        using insert_t = insert<Index>::template type<Other...>;

        template<std::size_t... Index>
        using remove_at_t = remove_at<Index...>::type;

        template<std::size_t Index, auto Other>
        using replace_t =
            to_value_sequence<typename to_value_sequence<front_t<Index>>::template append_t<
                Other>>::template append_by_seq_t<back_t<size() - Index - 1>>;

        template<typename Comp>
        using unique_t = unique_value_sequence<Comp>::type;
    };
}

namespace std
{
    template<auto... Values>
    struct tuple_size<stdsharp::value_sequence<Values...>> :
        stdsharp::index_constant<stdsharp::value_sequence<Values...>::size()>
    {
    };

    template<std::size_t I, auto... Values>
    struct tuple_element<I, stdsharp::value_sequence<Values...>>
    {
        using type = typename stdsharp::value_sequence<Values...>::template value_type<I>;
    };
}