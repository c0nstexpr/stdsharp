// Created by BlurringShadow at 2021-03-04-下午 11:27

#pragma once

#include <algorithm>
#include <ranges>
#include <variant>

#include "utility/functional.h"

namespace blurringshadow::utility::traits
{
    template<auto...>
    struct regular_value_sequence
    {
    };

    template<auto...>
    struct value_sequence;

    template<typename>
    struct take_value_sequence;

    template<template<auto...> typename T, auto... Values>
    struct take_value_sequence<T<Values...>>
    {
        template<template<auto...> typename U>
        using apply_t = U<Values...>;

        using as_sequence_t = regular_value_sequence<Values...>;

        using as_value_sequence_t = value_sequence<Values...>;
    };

    template<typename Sequence>
    using to_regular_value_sequence_t = typename take_value_sequence<Sequence>::as_vsequence_t;

    template<typename Sequence>
    using to_value_sequence_t = typename take_value_sequence<Sequence>::as_value_sequence_t;

    namespace details
    {
        using namespace std;

        template<auto From, auto PlusF, std::size_t... I>
            requires requires { traits::regular_value_sequence<invoke(PlusF, From, I)...>{}; }
        constexpr auto make_sequence(std::index_sequence<I...>) noexcept
        {
            return traits::regular_value_sequence<invoke(PlusF, From, I)...>{};
        }
    }

    template<auto From, std::size_t Size, auto PlusF = plus_v>
    using make_value_sequence_t =
        decltype(details::make_sequence<From, PlusF>(std::make_index_sequence<Size>{}));

    namespace details
    {
        template<auto... Values>
        struct reverse_value_sequence
        {
            using seq = value_sequence<Values...>;
            using type = typename seq::template indexed_by_seq_t<
                make_value_sequence_t<seq::size() - 1, seq::size(), minus_v> // clang-format off
            >; // clang-format on
        };

        template<auto... Values>
        struct unique_value_sequence
        {
            using seq = value_sequence<Values...>;

            template<size_t I>
            static constexpr auto is_valid()
            {
                return seq::find_if( // clang-format off
                    [j = size_t{0}](const auto& v) mutable
                    {
                        if(j == I) return true;
                        ++j;
                        return equal_to_v(seq::template get<I>(), v);
                    } 
                ) == I; // clang-format on
            }

            static constexpr auto filtered_indices = []<size_t... I>(const index_sequence<I...>)
            {
                array<size_t, seq::size()> indices{};
                size_t valid_size = 0;
                const auto f = [&]<size_t J>(const constant<J>) noexcept
                {
                    if(is_valid<J>())
                    {
                        indices[valid_size] = J;
                        ++valid_size;
                    }
                };

                (f(constant<I>{}), ...);

                return pair{indices, valid_size};
            }
            (typename seq::index_seq{});

            template<std::size_t... I>
            using filtered_seq =
                regular_value_sequence<seq::template get<filtered_indices.first[I]>()...>;

            using type = typename take_value_sequence< //
                make_value_sequence_t<std::size_t{}, filtered_indices.second> // clang-format off
            >::template apply_t<filtered_seq>; // clang-format on
        };

        template<size_t I, auto Value>
        struct indexed_value : constant<Value>
        {
            template<size_t J>
                requires(I == J)
            static constexpr auto get_value() noexcept { return Value; }
        };

        template<typename, typename>
        struct value_sequence;

        template<auto... Values, size_t... I>
        struct value_sequence<traits::value_sequence<Values...>, index_sequence<I...>> :
            indexed_value<I, Values>...
        {
            using indexed_value<I, Values>::get_value...;

            using index_seq = index_sequence<I...>;

            template<typename T, typename Comp>
            static constexpr auto value_comparer(const T& value, Comp& comp = {}) noexcept
            {
                return [&comp, &value]<typename U>(const U& other) //
                    noexcept(!invocable<Comp, U, T> || nothrow_invocable_r<Comp, bool, U, T>)
                {
                    if constexpr(std::predicate<Comp, U, T>)
                        return invoke_r<bool>(comp, value, other); // clang-format off
                    else return false; // clang-format on
                };
            }
        };

        template<typename Proj, typename Func, auto... Values>
        concept value_sequence_invocable =
            ((std::invocable<Proj, decltype(Values)> &&
              std::invocable<Func, invoke_result_t<Proj, decltype(Values)>>)&&...);

        template<typename Proj, typename Func, auto... Values>
        concept value_sequence_nothrow_invocable =
            ((nothrow_invocable<Func, invoke_result_t<Proj, decltype(Values)>>)&&...);

        template<typename Proj, typename Func, auto... Values>
        concept value_sequence_predicate =
            ((predicate<Func, invoke_result_t<Proj, decltype(Values)>>)&&...);

        template<typename Proj, typename Func, auto... Values>
        concept value_sequence_nothrow_predicate =
            ((nothrow_invocable_r<Func, bool, invoke_result_t<Proj, decltype(Values)>>)&&...);
    }

    template<auto... Values>
    using reverse_value_sequence_t = typename details::reverse_value_sequence<Values...>::type;

    template<auto... Values>
    using unique_value_sequence_t = typename details::unique_value_sequence<Values...>::type;

    template<auto... Values> // clang-format off
    struct value_sequence : private details::value_sequence<
        value_sequence<Values...>,
        std::make_index_sequence<sizeof...(Values)>
    > // clang-format on
    {
        template<std::size_t I>
        static constexpr auto get() noexcept
        {
            return base::template get_value<I>();
        }

        static constexpr std::size_t size() noexcept { return sizeof...(Values); }

    private:
        using base = details::value_sequence<
            value_sequence<Values...>,
            std::make_index_sequence<sizeof...(Values)> // clang-format off
        >; // clang-format on

    public:
        using index_seq = typename base::index_seq; // clang-format off

        static constexpr auto invoke = []<typename Func>(Func && func) // clang-format on
            noexcept((nothrow_invocable<Func, decltype(Values)> && ...)) //
            -> decltype(auto) //
            requires(std::invocable<Func, decltype(Values)>&&...)
        {
            const auto f = [&func]<typename T>(T&& v) noexcept(nothrow_invocable<Func, T>)
            {
                return std::invoke(std::forward<Func>(func), std::forward<T>(v)); //
            };
            return merge_invoke(std::bind(f, Values)...);
        };

        template<template<auto...> typename T>
        using apply_t = T<Values...>;

        template<std::size_t... OtherInts>
        using indexed_t = regular_value_sequence<get<OtherInts>()...>;

        template<typename Seq>
        using indexed_by_seq_t = typename take_value_sequence<Seq>::template apply_t<indexed_t>;

    private: //
        struct for_each_fn
        {
            template<
                typename Func,
                details::value_sequence_invocable<Func, Values...> Proj = std::identity>
            constexpr auto operator()(Func func, Proj proj = {}) const
                noexcept(details::value_sequence_nothrow_invocable<Proj, Func, Values...>)
            {
                (std::invoke(func, std::invoke(proj, Values)), ...);
                return func;
            }
        };

        struct for_each_n_fn
        {
            template<
                typename Func,
                details::value_sequence_invocable<Func, Values...> Proj = std::identity>
            constexpr auto operator()(auto size, Func func, Proj proj = {}) const
                noexcept(details::value_sequence_nothrow_invocable<Proj, Func, Values...>)
            {
                const auto f = [&]<typename T>(T&& v) //
                    noexcept(std::invoke(func, std::invoke(proj, std::declval<T>())))
                {
                    if(size == 0) return false;
                    std::invoke(func, std::invoke(proj, std::forward<T>(v)));
                    --size;
                    return true;
                };

                (f(Values) && ...);
                return func;
            }
        };

        struct find_if_fn
        {
            template<
                typename Func,
                details::value_sequence_predicate<Func, Values...> Proj = std::identity>
            [[nodiscard]] constexpr auto operator()(Func func, Proj proj = {}) const
                noexcept(details::value_sequence_nothrow_predicate<Proj, Func, Values...>)
            {
                std::size_t i = 0;
                const auto f = [&func, &proj, &i]<typename T>(T&& v) //
                    noexcept(noexcept(invoke_r<bool>(func, std::invoke(proj, std::forward<T>(v)))))
                {
                    if(invoke_r<bool>(func, std::invoke(proj, std::forward<T>(v)))) return false;
                    ++i;
                    return true;
                };

                (f(Values) && ...);

                return i;
            }
        };

        struct find_if_not_fn
        {
            template<typename Func, typename Proj = std::identity>
            [[nodiscard]] constexpr auto operator()(Func func, Proj&& proj = {}) const
                noexcept(details::value_sequence_nothrow_predicate<Proj, Func, Values...>)
            {
                return find_if(
                    [&](const auto& v) noexcept(nothrow_invocable_r<Func, bool, decltype(v)>)
                    {
                        return !invoke_r<bool>(func, v); //
                    },
                    std::forward<Proj>(proj) //
                );
            }
        };

        struct find_fn
        {
            template<typename Comp = std::ranges::equal_to, typename Proj = std::identity>
            [[nodiscard]] constexpr auto operator()( //
                const auto& v,
                Comp comp = {},
                Proj&& proj = {} // clang-format off
            ) const noexcept(nothrow_invocable<
                find_if_fn,
                decltype(base::value_comparer(v, comp)),
                Proj
            >) // clang-format on
            {
                return find_if(base::value_comparer(v, comp), std::forward<Proj>(proj));
            }
        };

        struct count_if_fn
        {
            template<typename Func, typename Proj = std::identity>
            [[nodiscard]] constexpr auto operator()(Func func, Proj&& proj = {}) const
                noexcept(details::value_sequence_nothrow_predicate<Proj, Func, Values...>)
            {
                std::size_t i = 0;
                for_each(
                    [&i, &func](const auto& v) //
                    noexcept(nothrow_invocable_r<Func, bool, decltype(v)>)
                    {
                        if(invoke_r<bool>(func, v)) ++i;
                        return true;
                    },
                    std::forward<Proj>(proj) //
                );

                return i;
            }
        };

        struct count_if_not_fn
        {
            template<typename Func, typename Proj = std::identity>
            [[nodiscard]] constexpr auto operator()(Func func, Proj&& proj = {}) const
                noexcept(details::value_sequence_nothrow_predicate<Proj, Func, Values...>)
            {
                return count_if(
                    [&](const auto& v) noexcept(nothrow_invocable_r<Func, bool, decltype(v)>)
                    {
                        return !invoke_r<bool>(func, v); //
                    },
                    std::forward<Proj>(proj) //
                );
            }
        };

        struct count_fn
        {
            template<typename Comp = std::ranges::equal_to, typename Proj = std::identity>
            [[nodiscard]] constexpr auto operator()( //
                const auto& v,
                Comp comp = {},
                Proj&& proj = {} // clang-format off
            ) const noexcept(nothrow_invocable<
                count_if_fn,
                decltype(base::value_comparer(v, comp)), 
                Proj
            >
            ) // clang-format on
            {
                return count_if(base::value_comparer(v, comp), std::forward<Proj>(proj));
            }
        };

        struct all_of_fn
        {
            template<typename Func, typename Proj = std::identity>
            [[nodiscard]] constexpr auto operator()(Func&& func, Proj&& proj = {}) const
                noexcept(nothrow_invocable<find_if_not_fn, Func, Proj>)
            {
                return find_if_not(std::forward<Func>(func), std::forward<Proj>(proj)) == size();
            }
        };

        struct any_of_fn
        {
            template<typename Func, typename Proj = std::identity>
            [[nodiscard]] constexpr auto operator()(Func&& func, Proj&& proj = {}) const
                noexcept(nothrow_invocable<find_if_fn, Func, Proj>)
            {
                return find_if(std::forward<Func>(func), std::forward<Proj>(proj)) != size();
            }
        };

        struct none_of_fn
        {
            template<typename Func, typename Proj = std::identity>
            [[nodiscard]] constexpr auto operator()(Func&& func, Proj&& proj = {}) const
                noexcept(nothrow_invocable<find_if_fn, Func, Proj>)
            {
                return find_if(std::forward<Func>(func), std::forward<Proj>(proj)) == size();
            }
        };

        struct contains_fn
        {
            template<typename Proj = std::identity>
            [[nodiscard]] constexpr auto operator()(const auto& v, Proj&& proj = {}) const
                noexcept(nothrow_invocable<find_fn, decltype(v), Proj>)
            {
                return find(v, std::forward<Proj>(proj)) != size();
            }
        };

        struct adjacent_find_fn
        {
        private:
            template<std::size_t I, typename Comp, typename Proj>
            struct by_index
            {
                using left_projected_t = std::invoke_result_t<Proj, decltype(get<I>())>;
                using right_projected_t = std::invoke_result_t<Proj, decltype(get<I + 1>())>;

                static constexpr auto invoke(Comp& comp, Proj& proj) noexcept(
                    nothrow_invocable_r<Comp, bool, left_projected_t, right_projected_t>) requires
                    invocable_r<Comp, bool, left_projected_t, right_projected_t>

                {
                    return invoke_r<bool>(
                        comp, std::invoke(proj, get<I>()), std::invoke(proj, get<I + 1>()) //
                    );
                }

                static constexpr auto invoke(Comp&, Proj&) noexcept { return false; }
            };

            struct impl
            {
                template<typename Comp, typename Proj, std::size_t... I>
                constexpr auto operator()(
                    Comp& comp,
                    Proj& proj,
                    const std::index_sequence<I...> // clang-format off
                ) noexcept(noexcept((by_index<I, Comp, Proj>::invoke(comp, proj), ...))) // clang-format on
                {
                    std::size_t res{};
                    ((
                         [&]<std::size_t J>(const constant<J>) //
                         noexcept(noexcept(by_index<J, Comp, Proj>::invoke(comp, proj)))
                         {
                             if(by_index<J, Comp, Proj>::invoke(comp, proj)) return true;
                             ++res;
                             return false;
                         }(constant<I>{})) ||
                     ...);
                    return res;
                }
            };

        public:
            template<typename Comp = std::ranges::equal_to, typename Proj = std::identity>
                requires(std::invocable<Proj, decltype(Values)>&&...)
            [[nodiscard]] constexpr auto operator()(Comp comp = {}, Proj proj = {}) const
                noexcept(nothrow_invocable<impl, Comp, Proj, std::make_index_sequence<size() - 2>>)
            {
                return impl{}(comp, proj, std::make_index_sequence<size() - 2>{});
            }

            template<typename Comp = std::nullptr_t, typename Proj = std::nullptr_t>
            [[nodiscard]] constexpr auto operator()(const Comp& = {}, const Proj& = {}) const //
                noexcept requires(size() < 2)
            {
                return size();
            }
        };

        template<std::size_t From, std::size_t... I>
        static constexpr indexed_t<From + I...> select_range_indexed( // clang-format off
            std::index_sequence<I...>
        ) noexcept; // clang-format on

    public:
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

        static constexpr adjacent_find_fn adjacent_find{};

        template<auto... Func>
        static constexpr auto transform() noexcept // clang-format off
        {         
            if constexpr(sizeof...(Func) == 1) return []<auto F>(const constant<F>) noexcept
            {
                return value_sequence<std::invoke(F, Values)...>{};
            }(constant<Func...>{}); // clang-format on
            else
            {
                static_assert((invocable_rnonvoid<decltype(Func), decltype(Values)> && ...));
                return value_sequence<std::invoke(Func, Values)...>{};
            }
        }

        template<auto... Func>
        using transform_t = decltype(transform<Func...>());

        template<std::size_t From, std::size_t Size>
        using select_range_indexed_t =
            decltype(select_range_indexed<From>(std::make_index_sequence<Size>{}));

        template<std::size_t Size>
        using back_t = select_range_indexed_t<size() - Size, Size>;

        template<std::size_t Size>
        using front_t = select_range_indexed_t<0, Size>;

        template<auto... Others>
        using append_t = regular_value_sequence<Values..., Others...>;

        template<typename Seq>
        using append_by_seq_t = typename take_value_sequence<Seq>::template apply_t<append_t>;

        template<auto... Others>
        using append_front_t = regular_value_sequence<Others..., Values...>;

        template<typename Seq>
        using append_front_by_seq_t =
            typename take_value_sequence<Seq>::template apply_t<append_front_t>;

    private:
        template<std::size_t Index>
        struct insert
        {
            template<auto... Others>
            using type = typename to_value_sequence_t<
                typename to_value_sequence_t<front_t<Index>>:: //
                template append_t<Others...> // clang-format off
            >::template append_by_seq_t<back_t<size() - Index>>; // clang-format on
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
            typename take_value_sequence<Seq>::template apply_t<insert<Index>::template type>;

        template<std::size_t... Index>
        using remove_at_t = typename remove_at<Index...>::type;

        template<typename Seq>
        using remove_at_by_seq_t = typename take_value_sequence<Seq>::template apply_t<remove_at_t>;

        template<std::size_t Index, auto Other>
        using replace_t = typename to_value_sequence_t<
            typename to_value_sequence_t<front_t<Index>>::template append_t<
                Other> // clang-format off
        >::template append_by_seq_t<back_t<size() - Index - 1>>; // clang-format on
    };
}

namespace std
{
    template<std::size_t I, auto... Values>
    struct std::tuple_element<I, blurringshadow::utility::traits::value_sequence<Values...>> :
        std::type_identity< // clang-format off
            decltype(
                typename blurringshadow::utility::traits::
                    value_sequence<Values...>::
                    template get<I>()
            )
        > // clang-format on
    {
    };

    template<auto... Values>
    struct std::tuple_size<blurringshadow::utility::traits::value_sequence<Values...>> :
        blurringshadow::utility::index_constant<
            blurringshadow::utility::traits::value_sequence<Values...>::size() // clang-format off
        > // clang-format on
    {
    };
}
