#pragma once

#include "../functional/always_return.h"
#include "indexed_traits.h"
#include "regular_value_sequence.h"
#include "stdsharp/functional/empty_invoke.h"

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
    struct value_indexer
    {
        template<auto V>
        struct filter
        {
            consteval decltype(V) get() const noexcept { return V; }
        };

        template<std::size_t>
        struct invalid
        {
        };

    public:
        template<std::size_t I, std::size_t... J, auto... V>
        static consteval decltype(auto) impl(
            const std::index_sequence<J...> /*unused*/,
            const regular_value_sequence<V...> /*unused*/
        )
        {
            constexpr struct STDSHARP_EBO : std::conditional_t<I == J, filter<V>, invalid<J>>...
            {
            } f{};

            return f.get();
        }

        template<std::size_t I, auto... V>
        static constexpr decltype(auto) value =
            impl<I>(std::make_index_sequence<sizeof...(V)>{}, regular_value_sequence<V...>{});
    };
}

namespace stdsharp
{
    template<auto... Values>
    struct value_sequence
    {
        static constexpr std::size_t size() noexcept { return sizeof...(Values); }

        template<std::size_t I>
        using value_type = type_at<I, decltype(Values)...>;

        template<std::size_t I>
            requires requires { requires I < size(); }
        [[nodiscard]] static constexpr value_type<I> get() noexcept
        {
            return details::value_indexer::template value<I, Values...>;
        }

        template<std::size_t I>
        static constexpr auto const_value = get<I>();

        template<template<auto...> typename T>
        using apply_t = T<Values...>;

        template<std::size_t... I>
        using at_t = regular_value_sequence<get<I>()...>;

        template<auto... Func>
        struct transform_fn
        {
            [[nodiscard]] constexpr value_sequence<stdsharp::invoke(Func, Values)...>
                operator()() const noexcept
            {
                return {};
            };
        };

        template<auto Func>
        struct transform_fn<Func>
        {
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
        template<typename Func>
            requires(std::invocable<Func&, decltype(Values)> && ...)
        static consteval void invocable_test()
            noexcept((nothrow_invocable<Func&, decltype(Values)> && ...));

        template<typename Func>
            requires(std::predicate<Func&, const decltype(Values)&> && ...)
        static consteval void predicate_test()
            noexcept((nothrow_predicate<Func&, const decltype(Values)&> && ...));

    public:
        // TODO: P1306 Expansion statements
        template<typename ResultType = void>
        struct invoke_fn
        {
            template<typename Func>
                requires requires {
                    invocable_test<Func>();
                    requires std::same_as<ResultType, void>;
                }
            constexpr auto operator()(Func&& func) const noexcept(noexcept(invocable_test<Func>()))
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
                noexcept(invocable_test<Func>()) &&
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

        static constexpr struct for_each_fn
        {
        private:
            static constexpr auto impl(auto&& value, auto& func, auto& condition) noexcept(
                nothrow_invocable<decltype(func), decltype(value)> &&
                nothrow_predicate<decltype(condition), const decltype(value)&> //
            )
            {
                if(!invoke_r<bool>(condition, std::as_const(value))) return false;

                stdsharp::invoke(func, cpp_forward(value));
                return true;
            }

        public:
            template<typename Func, typename Condition = always_true_fn>
                requires requires { invocable_test<Func>(), predicate_test<Condition>(); }
            constexpr void operator()(Func func, Condition condition = {}) const
                noexcept(noexcept(invocable_test<Func>(), predicate_test<Condition>()))
            {
                empty = (impl(Values, func, condition) && ...);
            }
        } for_each{};

        template<std::size_t From, std::size_t Size>
        using select_range_t =
            to_value_sequence<make_value_sequence<From, Size>>::template apply_t<at_t>;

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
            static constexpr at_t<select_indices[I]...> impl(std::index_sequence<I...>) noexcept;

            using type = decltype(impl(std::make_index_sequence<select_indices.size()>{}));
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
    };

}

namespace stdsharp::details
{
    struct value_sequence_algo
    {
        template<typename T, typename Comp>
        struct value_comparer
        {
            using value_type = const T&;

            value_type value;
            Comp& comp;

            template<typename U, typename ConstU = const U&>
                requires std::predicate<Comp&, ConstU, value_type>
            constexpr bool operator()(const U& other) const
                noexcept(nothrow_predicate<Comp&, ConstU, value_type>)
            {
                return invoke(comp, other, value);
            }
        };

        template<typename Seq, typename Fn, typename ForEach = Seq::for_each_fn>
            requires std::invocable<ForEach, Fn>
        static consteval void invocable_test() noexcept(nothrow_invocable<ForEach, Fn>);

        template<typename Seq, typename Predicate, typename ForEach = Seq::for_each_fn>
            requires std::invocable<ForEach, empty_invoke_fn, Predicate>
        static consteval void predicate_test()
            noexcept(nothrow_invocable<ForEach, empty_invoke_fn, Predicate>);

        template<typename IfFunc>
        struct if_not_fn
        {
            template<typename Func>
                requires std::invocable<IfFunc, Func&>
            [[nodiscard]] constexpr auto operator()(Func func) const
                noexcept(nothrow_invocable<IfFunc, Func&>)
            {
                return IfFunc{}(std::not_fn(std::ref(func)));
            }
        };

        template<typename IfFunc>
        struct from_value_fn
        {
            template<
                typename T,
                typename Comp = std::ranges::equal_to,
                typename Comparer = value_comparer<T, Comp>>
                requires std::invocable<IfFunc, Comparer>
            [[nodiscard]] constexpr auto operator()(const T& v, Comp comp = {}) const
                noexcept(nothrow_invocable<IfFunc, Comparer>)
            {
                return IfFunc{}(Comparer{v, comp});
            }
        };

        template<typename FindFunc, std::size_t Size, bool Equal = true>
        struct result_compare_to_size
        {
            template<typename Func>
                requires std::invocable<FindFunc, Func>
            [[nodiscard]] constexpr auto operator()(Func&& func) const
                noexcept(nothrow_invocable<FindFunc, Func>)
            {
                const auto v = FindFunc{}(cpp_forward(func));
                if constexpr(Equal) return v == Size;
                else return v != Size;
            }
        };
    };
}

namespace stdsharp::value_sequence_algo
{
    template<typename Seq>
    struct find_if_fn
    {
        using algo = details::value_sequence_algo;

        template<typename Func>
            requires requires { algo::predicate_test<Seq, Func>(); }
        [[nodiscard]] constexpr auto operator()(Func func) const
            noexcept(noexcept(algo::predicate_test<Seq, Func>()))
        {
            std::size_t i = 0;
            Seq::for_each(
                [&i](const auto& /*unused*/) noexcept { ++i; },
                std::not_fn(cpp_move(func))
            );
            return i;
        }
    };

    template<typename Seq>
    inline constexpr find_if_fn<Seq> find_if{};

    template<typename Seq>
    struct for_each_n_fn
    {
        using algo = details::value_sequence_algo;

        template<typename Func>
            requires requires { algo::invocable_test<Seq, Func>(); }
        constexpr auto operator()(auto count, Func func) const
            noexcept(noexcept(algo::invocable_test<Seq, Func>()))
        {
            Seq::for_each(
                [&count, &func](const auto& v) noexcept(nothrow_invocable<Func, decltype(v)>)
                {
                    stdsharp::invoke(func, v);
                    --count;
                },
                [&count](const auto& /*unused*/) noexcept { return count > 0; }
            );
        }
    };

    template<typename Seq>
    inline constexpr for_each_n_fn<Seq> for_each_n{};

    template<typename Seq>
    using find_if_not_fn = details::value_sequence_algo::if_not_fn<find_if_fn<Seq>>;

    template<typename Seq>
    inline constexpr find_if_not_fn<Seq> find_if_not{};

    template<typename Seq>
    using find_fn = details::value_sequence_algo::from_value_fn<find_if_fn<Seq>>;

    template<typename Seq>
    inline constexpr find_fn<Seq> find{};

    template<typename Seq>
    struct count_if_fn
    {
        using algo = details::value_sequence_algo;

        template<typename Func>
            requires requires { algo::predicate_test<Seq, Func>(); }
        [[nodiscard]] constexpr auto operator()(Func func) const
            noexcept(noexcept(algo::predicate_test<Seq, Func>()))
        {
            std::size_t i = 0;
            Seq::for_each(
                [&i, &func](const auto& v) noexcept(nothrow_predicate<Func, decltype(v)>)
                {
                    if(invoke_r<bool>(func, v)) ++i;
                }
            );
            return i;
        }
    };

    template<typename Seq>
    inline constexpr count_if_fn<Seq> count_if{};

    template<typename Seq>
    using count_if_not_fn = details::value_sequence_algo::if_not_fn<count_if_fn<Seq>>;

    template<typename Seq>
    inline constexpr count_if_not_fn<Seq> count_if_not{};

    template<typename Seq>
    using count_fn = details::value_sequence_algo::from_value_fn<count_if_fn<Seq>>;

    template<typename Seq>
    inline constexpr count_fn<Seq> count{};

    template<typename Seq>
    using all_of_fn =
        details::value_sequence_algo::result_compare_to_size<find_if_not_fn<Seq>, Seq::size()>;

    template<typename Seq>
    inline constexpr all_of_fn<Seq> all_of{};

    template<typename Seq>
    using none_of_fn =
        details::value_sequence_algo::result_compare_to_size<find_if_fn<Seq>, Seq::size()>;

    template<typename Seq>
    inline constexpr none_of_fn<Seq> none_of{};

    template<typename Seq>
    using any_of_fn =
        details::value_sequence_algo::result_compare_to_size<find_if_fn<Seq>, Seq::size(), false>;

    template<typename Seq>
    inline constexpr any_of_fn<Seq> any_of{};

    template<typename Seq>
    using contains_fn =
        details::value_sequence_algo::result_compare_to_size<find_fn<Seq>, Seq::size(), false>;

    template<typename Seq>
    inline constexpr contains_fn<Seq> contains{};

    template<typename Seq>
    struct adjacent_find_fn
    {
    private:
        static constexpr auto size = Seq::size();

        template<std::size_t I>
        struct by_index
        {
            template<
                typename Comp,
                typename FirstV = const Seq::template value_type<I>&,
                typename SecondV = const Seq::template value_type<I + 1>&>
                requires std::predicate<Comp&, FirstV, SecondV>
            constexpr bool operator()(Comp& comp, std::size_t& i) const
                noexcept(nothrow_predicate<Comp&, FirstV, SecondV>)
            {
                if(invoke(comp, Seq::template const_value<I>, Seq::template const_value<I + 1>))
                    return true;
                ++i;
                return false;
            }
        };

        template<>
        struct by_index<size - 1>
        {
            constexpr bool operator()(auto& /*unused*/, std::size_t& /*unused*/) const noexcept
            {
                return false;
            }
        };

        template<typename Comp, std::size_t... I>
            requires(std::invocable<by_index<I>, Comp&, std::size_t&> && ...)
        static constexpr auto impl(Comp& comp, const std::index_sequence<I...> /*unused*/) //
            noexcept((nothrow_invocable<by_index<I>, Comp&, std::size_t&> && ...))
        {
            std::size_t i{};
            empty = (by_index<I>{}(comp, i) || ...);
            return i;
        }

    public:
        template<
            typename Comp = std::ranges::equal_to,
            typename IdxSeq = std::make_index_sequence<size>>
        [[nodiscard]] constexpr auto operator()(Comp comp = {}) const
            noexcept(noexcept(impl(comp, IdxSeq{})))
            requires requires { impl(comp, IdxSeq{}); }
        {
            return impl(comp, IdxSeq{});
        }
    };

    template<typename Seq>
    inline constexpr adjacent_find_fn<Seq> adjacent_find{};

    template<typename Seq>
    using reverse_t =
        to_value_sequence<make_value_sequence<Seq::size() - 1, Seq::size(), std::minus{}>>::
            template apply_t<Seq::template at_t>;
}

namespace stdsharp::details
{
    template<typename, typename>
    struct unique_value_sequence;

    template<auto... Values, typename Comp>
    struct unique_value_sequence<value_sequence<Values...>, Comp>
    {
        using seq = value_sequence<Values...>;

        static constexpr auto size() noexcept { return seq::size(); }

        static constexpr auto unique_indices_value = []
        {
            constexpr auto unique_indices = []
            {
                std::array<std::size_t, size()> indices{
                    stdsharp::value_sequence_algo::find<seq>(Values, Comp{})...
                };

                std::ranges::sort(indices);

                if(const auto res = std::ranges::unique(indices); res) res.front() = size();

                return indices;
            }();

            constexpr auto unique_indices_size =
                std::ranges::find(unique_indices, size()) - unique_indices.cbegin();

            std::array<std::size_t, unique_indices_size> value{};

            std::ranges::copy_n(unique_indices.cbegin(), value.size(), value.begin());

            return value;
        }();

        using type = to_value_sequence<rng_v_to_sequence<unique_indices_value>>:: //
            template apply_t<value_sequence<Values...>::template at_t>;
    };
}

namespace stdsharp::value_sequence_algo
{
    template<typename Seq, typename Comp = std::ranges::equal_to>
        requires(cpp_is_constexpr(Comp{}))
    using unique_t = details::unique_value_sequence<Seq, Comp>::type;
}

namespace std
{
    template<auto... Values>
    struct tuple_size<::stdsharp::value_sequence<Values...>> :
        ::stdsharp::index_constant<::stdsharp::value_sequence<Values...>::size()>
    {
    };

    template<std::size_t I, auto... Values>
    struct tuple_element<I, ::stdsharp::value_sequence<Values...>>
    {
        using type = typename ::stdsharp::value_sequence<Values...>::template value_type<I>;
    };
}