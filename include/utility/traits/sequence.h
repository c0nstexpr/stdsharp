// Created by BlurringShadow at 2021-03-04-下午 11:27

#pragma once

#include <algorithm>
#include <ranges>

#include "utility/functional.h"

namespace blurringshadow::utility::traits
{
    template<auto... Values>
    struct regular_sequence
    {
    };

    template<auto... Values>
    struct sequence;

    template<typename>
    struct as_seq;

    template<template<auto...> typename T, auto... Values>
    struct as_seq<T<Values...>>
    {
        template<template<auto...> typename U>
        using apply_t = U<Values...>;

        using sequence_t = sequence<Values...>;
    };

    namespace details
    {
        using namespace std;

        template<size_t I, auto Value>
        struct indexed_value : constant<Value>
        {
            template<size_t J>
                requires(I == J)
            static constexpr auto get_value() noexcept { return Value; }
        };

        template<typename, typename>
        struct sequence;

        template<auto... Values, size_t... I>
        struct sequence<traits::sequence<Values...>, index_sequence<I...>> :
            regular_sequence<Values...>,
            indexed_value<I, Values>...
        {
            using indexed_value<I, Values>::get_value...;

            static constexpr auto size() noexcept { return sizeof...(Values); }

            template<typename T, typename Proj = identity>
            static constexpr auto get_value_equality_comparer( // clang-format off
                const T& value,
                Proj proj = {}
            ) noexcept // clang-format on
            {
                using projected_t = std::invoke_result_t<Proj, T>;

                return [&]<typename U>(const U& other) noexcept( // clang-format off
                    !invocable<ranges::equal_to, projected_t, U> ||
                    nothrow_invocable<ranges::equal_to, projected_t, U>
                )
                {
                    if constexpr(invocable<ranges::equal_to, projected_t, U>)
                        return equal_to_v(value, proj(other));
                    else return false;
                }; // clang-format on
            }
        };

        template<typename Func, auto... Values>
        concept seq_predicate = (predicate<Func, decltype(Values)> && ...);

        template<typename Func, auto... Values>
        concept seq_nothrow_predicate = (nothrow_invocable_r<Func, bool, decltype(Values)> && ...);
    }

    template<typename Sequence>
    using from_seq_t = typename as_seq<Sequence>::sequence_t;

    template<auto... Values> // clang-format off
    struct sequence : private details::sequence<
        sequence<Values...>, std::make_index_sequence<sizeof...(Values)>
    > // clang-format on
    {
    private:
        using base = details::sequence<
            sequence<Values...>,
            std::make_index_sequence<sizeof...(Values)> // clang-format off
        >; // clang-format on

    public:
        using base::size;

        template<std::size_t I>
        static constexpr auto get_by_index() noexcept
        {
            return base::template get_value<I>();
        }

        template<typename Func> // clang-format off
            requires(std::invocable<Func, decltype(Values)> && ...)
        static constexpr decltype(auto) invoke(Func&& func)
            noexcept((nothrow_invocable<Func, decltype(Values)> && ...)) // clang-format on
        {
            return merge_invoke( // clang-format off
                [&func]() noexcept(nothrow_invocable<Func, decltype(Values)>)
                {
                    return std::invoke(std::forward<Func>(func), Values);
                }...
            ); // clang-format on
        }

        template<template<auto...> typename T>
        using apply_t = T<Values...>;

        template<std::size_t... OtherInts>
        using indexed_t = sequence<get_by_index<OtherInts>()...>;

        template<typename Seq>
        using indexed_by_seq_t = typename as_seq<Seq>::template apply_t<indexed_t>;

    private:
        struct for_each_fn
        {
            constexpr auto operator()(details::seq_predicate<Values...> auto func) const
                noexcept(noexcept((std::invoke(func, Values), ...)))
            {
                (std::invoke(func, Values) && ...);
                return func;
            }
        };

        struct for_each_n_fn
        {
            constexpr auto operator()(auto size, auto func) const
                noexcept(noexcept((std::invoke(func, Values), ...)))
            {
                return for_each(
                    [&size, &func](auto v) noexcept(noexcept(std::invoke(func, v)))
                    {
                        if(size == 0) return false;

                        std::invoke(func, v);
                        --size;
                        return true;
                    } // clang-format off
                ); // clang-format on
            }
        };

        struct find_if_fn
        {
            template<details::seq_predicate<Values...> Func>
            [[nodiscard]] constexpr auto operator()(Func func) const
                noexcept(details::seq_nothrow_predicate<Func, Values...>)
            {
                std::size_t i = 0;
                for_each(
                    [&i, &func](const auto& v) noexcept(noexcept(invoke_r<bool>(func, v)))
                    {
                        if(invoke_r<bool>(func, v)) return false;

                        ++i;
                        return true;
                    } // clang-format off
                ); // clang-format on

                return i;
            }
        };

        struct find_if_not_fn
        {
            template<details::seq_predicate<Values...> Func>
            [[nodiscard]] constexpr auto operator()(Func func) const
                noexcept(details::seq_nothrow_predicate<Func, Values...>)
            {
                return find_if( // clang-format off
                    [&](const auto& v) noexcept(nothrow_invocable_r<Func, bool, decltype(v)>)
                    {
                        return !invoke_r<bool>(func, v);
                    } // clang-format on
                );
            }
        };

        struct find_fn
        {
            template<typename Proj = std::identity> // clang-format off
            [[nodiscard]] constexpr auto operator()(const auto& v, Proj&& proj = {}) const noexcept(
                noexcept(find_if(base::get_value_equality_comparer(v, std::forward<Proj>(proj))))
            ) // clang-format on
            {
                return find_if(base::get_value_equality_comparer(v, std::forward<Proj>(proj)));
            }
        };

        struct count_if_fn
        {
            template<details::seq_predicate<Values...> Func>
            [[nodiscard]] constexpr auto operator()(Func func) const
                noexcept(details::seq_nothrow_predicate<Func, Values...>)
            {
                std::size_t i = 0;
                for_each(
                    [&i, &func](const auto& v) // clang-format off
                        noexcept(nothrow_invocable_r<Func, bool, decltype(v)>) // clang-format on
                    {
                        if(invoke_r<bool>(func, v)) ++i;
                        return true;
                    } // clang-format off
                ); // clang-format on

                return i;
            }
        };

        struct count_if_not_fn
        {
            template<details::seq_predicate<Values...> Func>
            [[nodiscard]] constexpr auto operator()(Func func) const
                noexcept(details::seq_nothrow_predicate<Func, Values...>)
            {
                return count_if( // clang-format off
                    [&](const auto& v) noexcept(nothrow_invocable_r<Func, bool, decltype(v)>)
                    {
                        return !invoke_r<bool>(func, v);
                    } // clang-format on
                );
            }
        };

        struct count_fn
        {
            template<typename Proj = std::identity> // clang-format off
            [[nodiscard]] constexpr auto operator()(const auto& v, Proj&& proj = {}) const noexcept(
                noexcept(count_if(base::get_value_equality_comparer(v, std::forward<Proj>(proj))))
            ) // clang-format on
            {
                return count_if(base::get_value_equality_comparer(v, std::forward<Proj>(proj)));
            }
        };

        struct all_of_fn
        {
            template<details::seq_predicate<Values...> Func>
            [[nodiscard]] constexpr auto operator()(Func func) const
                noexcept(details::seq_nothrow_predicate<Func, Values...>)
            {
                return find_if_not(func) == size();
            }
        };

        struct any_of_fn
        {
            template<details::seq_predicate<Values...> Func>
            [[nodiscard]] constexpr auto operator()(Func func) const
                noexcept(details::seq_nothrow_predicate<Func, Values...>)
            {
                return find_if(func) != size();
            }
        };

        struct none_of_fn
        {
            template<details::seq_predicate<Values...> Func>
            [[nodiscard]] constexpr auto operator()(Func func) const
                noexcept(details::seq_nothrow_predicate<Func, Values...>)
            {
                return find_if(func) == size();
            }
        };

        struct contains_fn
        {
            template<typename Proj = std::identity>
            [[nodiscard]] constexpr auto operator()(const auto& v, Proj&& proj = {}) const
                noexcept(noexcept(find(v, std::forward<Proj>(proj))))
            {
                return find(v, std::forward<Proj>(proj)) != size();
            }
        };

        template<auto... Func>
        static constexpr auto transform() noexcept // clang-format off
        {         
            if constexpr(sizeof...(Func) == 1) return []<auto F>(const constant<F>) noexcept
                {
                    return sequence<std::invoke(F, Values)...>{};
                }(constant<Func...>{}); // clang-format on
            else
            {
                static_assert((invocable_rnonvoid<decltype(Func), decltype(Values)> && ...));
                return sequence<std::invoke(Func, Values)...>{};
            }
        }

        // TODO replace it with lambda cause compile errors
        template<std::size_t From, std::size_t... I>
        static constexpr indexed_t<From + I...> select_range_indexed( // clang-format off
            std::index_sequence<I...>
        ) noexcept; // clang-format on

    public: // clang-format off
        static constexpr for_each_fn for_each{};

        static constexpr for_each_n_fn for_each_n{};

        static constexpr find_if_fn find_if{};

        static constexpr find_if_not_fn find_if_not{};

        static constexpr find_fn find{};

        static constexpr count_if_fn count_if{};

        static constexpr count_if_not_fn count_if_not{};

        static constexpr count_fn count{};

        static constexpr all_of_fn all_of{};

        static constexpr any_of_fn any_of{};

        static constexpr none_of_fn none_of{};

        static constexpr contains_fn contains{};

        template<auto... Func>
        using transform_t = decltype(transform<Func...>());

        template<std::size_t From, std::size_t Size>
        using select_range_indexed_t =
            decltype(select_range_indexed<From>(std::make_index_sequence<Size>{}));
        /*decltype([]<std::size_t... I>(const std::index_sequence<I...>) {
            return sequence::indexed_t<From + I...>{};
        }(std::make_index_sequence<Size>{}));*/

        template<std::size_t Size>
            requires(Size <= size())
        using back_t = select_range_indexed_t<size() - Size, Size>;

        template<std::size_t Size>
            requires(Size <= size())
        using front_t = select_range_indexed_t<0, Size>;

        template<auto... Others>
        using append_t = sequence<Values..., Others...>;

        template<typename Seq>
        using append_by_seq_t = typename as_seq<Seq>::template apply_t<append_t>;

        template<auto... Others>
        using append_front_t = sequence<Others..., Values...>;

        template<typename Seq>
        using append_front_by_seq_t = typename as_seq<Seq>::template apply_t<append_front_t>;

    private:
        template<std::size_t Index>
        struct insert
        {
            template<auto... Others>
            using type = typename front_t<Index>:: // clang-format off
                template append_t<Others...>::
                template append_by_seq_t<back_t<size() - Index>>; // clang-format on
        };

        // TODO replace it with lambda cause compile errors
        template<std::size_t... Index>
        struct remove_at
        {
            static constexpr auto select_indices = []() noexcept
            {
                using namespace std;
                using namespace std::ranges;

                array<std::size_t, size()> res{};
                array excepted = {Index...};

                ranges::sort(excepted);

                const auto& [_, last] = ranges::set_difference( // clang-format off
                    iota_view{size_t{0}, size()},
                    excepted,
                    res.begin() // clang-format on
                );

                return pair{res, last - res.cbegin()};
            }();

            template<std::size_t... I>
            static constexpr indexed_t<select_indices.first[I]...>
                get_type(std::index_sequence<I...>) noexcept;

            using type = decltype(get_type(std::make_index_sequence<select_indices.second>{}));
        };

    public:
        template<std::size_t Index, auto... Other>
        using insert_t = typename insert<Index>::template type<Other...>;

        template<std::size_t Index, typename Seq>
        using insert_by_seq_t =
            typename as_seq<Seq>::template apply_t<insert<Index>::template type>;

        template<std::size_t... Index>
        using remove_at_t = typename remove_at<Index...>::type;

        template<typename Seq>
        using remove_at_by_seq_t = typename as_seq<Seq>::template apply_t<remove_at_t>;
    };

    template<auto From, std::size_t Size>
    using make_sequence_t = decltype( // clang-format off
        []<std::size_t... I>(
            std::index_sequence<I...> = std::make_index_sequence<Size>{}
        ) noexcept { return traits::sequence<increase_v(From, I)...>{}; }()
    ); // clang-format on
}
