// Created by BlurringShadow at 2021-03-04-下午 11:27

#pragma once

#include <algorithm>
#include <ranges>
#include <variant>

#include "utility/functional.h"

namespace std_sharp::utility::traits
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

        using as_sequence_t = ::std_sharp::utility::traits::regular_value_sequence<Values...>;

        using as_value_sequence_t = ::std_sharp::utility::traits::value_sequence<Values...>;
    };

    template<typename Sequence>
    using to_regular_value_sequence_t =
        typename ::std_sharp::utility::traits::take_value_sequence<Sequence>::as_vsequence_t;

    template<typename Sequence> // clang-format off
    using to_value_sequence_t = typename ::std_sharp::utility::traits::
        take_value_sequence<Sequence>::as_value_sequence_t; // clang-format on

    namespace details
    {
        template<auto From, auto PlusF, ::std::size_t... I>
            requires requires
            {
                ::std_sharp::utility::traits::regular_value_sequence<::std::invoke(
                    PlusF, From, I)...>{};
            }
        constexpr auto make_sequence(::std::index_sequence<I...>) noexcept
        {
            return ::std_sharp::utility::traits::regular_value_sequence<::std::invoke(
                PlusF, From, I)...>{};
        }
    }

    template<auto From, ::std::size_t Size, auto PlusF = ::std_sharp::utility::plus_v>
    using make_value_sequence_t = decltype( //
        ::std_sharp::utility::traits::details::make_sequence<From, PlusF>(
            ::std::make_index_sequence<Size>{} // clang-format off
        ) // clang-format on
    );

    namespace details
    {
        template<auto... Values>
        struct reverse_value_sequence
        {
            using seq = ::std_sharp::utility::traits::value_sequence<Values...>;
            using type = typename seq::template indexed_by_seq_t<
                ::std_sharp::utility::traits::
                    make_value_sequence_t<seq::size - 1, seq::size, minus_v> // clang-format off
            >; // clang-format on
        };

        template<auto... Values>
        struct unique_value_sequence
        {
            using seq = value_sequence<Values...>;

            template<::std::size_t I>
            static constexpr auto is_valid()
            {
                return seq::find_if( // clang-format off
                    [j = ::std::size_t{0}](const auto& v) mutable
                    {
                        if(j == I) return true;
                        ++j;
                        return ::std_sharp::utility::equal_to_v(seq::template get<I>(), v);
                    } 
                ) == I; // clang-format on
            }

            static constexpr auto filtered_indices =
                []<size_t... I>(const ::std::index_sequence<I...>)
            {
                ::std::array<size_t, seq::size> indices{};
                ::std::size_t valid_size = 0;
                const auto f =
                    [&]<::std::size_t J>(const ::std_sharp::utility::constant<J>) noexcept
                {
                    if(is_valid<J>())
                    {
                        indices[valid_size] = J;
                        ++valid_size;
                    }
                };

                (f(::std_sharp::utility::constant<I>{}), ...);

                return ::std::pair{indices, valid_size};
            }
            (typename seq::index_seq{});

            template<::std::size_t... I>
            using filtered_seq = ::std_sharp::utility::traits:: //
                regular_value_sequence<seq::template get<filtered_indices.first[I]>()...>;

            using type = typename ::std_sharp::utility::traits::take_value_sequence< //
                ::std_sharp::utility::traits:: //
                make_value_sequence_t<::std::size_t{}, filtered_indices.second> // clang-format off
            >::template apply_t<filtered_seq>; // clang-format on
        };

        template<::std::size_t I, auto Value>
        struct indexed_value : ::std_sharp::utility::constant<Value>
        {
            template<::std::size_t J>
                requires(I == J)
            static constexpr decltype(auto) get() noexcept { return Value; }
        };

        template<typename, typename>
        struct value_sequence;

        template<auto... Values, size_t... I>
        struct value_sequence<traits::value_sequence<Values...>, ::std::index_sequence<I...>> :
            ::std_sharp::utility::traits::details::indexed_value<I, Values>...
        {
            using indexed_value<I, Values>::get...;

            using index_seq = ::std::index_sequence<I...>;

            template<typename T, typename Comp>
            static constexpr auto value_comparer(const T& value, Comp& comp = {}) noexcept
            {
                return [&comp, &value]<typename U>(const U& other) noexcept( // clang-format off
                    !::std::invocable<Comp, U, T> ||
                    ::std_sharp::utility::nothrow_invocable_r<Comp, bool, U, T>
                ) // clang-format on
                {
                    if constexpr(::std::predicate<Comp, U, T>)
                        return ::std_sharp::utility::invoke_r<bool>(
                            comp, value, other); // clang-format off
                    else return false; // clang-format on
                };
            }
        };

        template<typename Proj, typename Func, auto... Values>
        concept value_sequence_invocable =
            ((::std::invocable<Proj, decltype(Values)> &&
              ::std::invocable<Func, ::std::invoke_result_t<Proj, decltype(Values)>>)&&...);

        template<typename Proj, typename Func, auto... Values> // clang-format off
        concept value_sequence_nothrow_invocable = (
            ::std_sharp::utility::
                nothrow_invocable<Func, ::std::invoke_result_t<Proj, decltype(Values)>> &&
            ...
        );

        template<typename Proj, typename Func, auto... Values>
        concept value_sequence_predicate =
            (::std::predicate<Func, ::std::invoke_result_t<Proj, decltype(Values)>> && ...);

        template<typename Proj, typename Func, auto... Values>
        concept value_sequence_nothrow_predicate = (
            ::std_sharp::utility::
                nothrow_invocable_r<bool, Func, ::std::invoke_result_t<Proj, decltype(Values)>> &&
            ...
        );
    } // clang-format on

    template<auto... Values>
    using reverse_value_sequence_t = typename ::std_sharp::utility::traits::details:: //
        reverse_value_sequence<Values...>::type;

    template<auto... Values>
    using unique_value_sequence_t =
        typename ::std_sharp::utility::traits::details::unique_value_sequence<Values...>::type;

    template<auto... Values> // clang-format off
    struct value_sequence : private ::std_sharp::utility::traits::details::value_sequence<
        ::std_sharp::utility::traits::value_sequence<Values...>,
        ::std::make_index_sequence<sizeof...(Values)>
    > // clang-format on
    {
    private:
        using base = ::std_sharp::utility::traits::details::value_sequence<
            ::std_sharp::utility::traits::value_sequence<Values...>,
            ::std::make_index_sequence<sizeof...(Values)> // clang-format off
        >; // clang-format on

        template<::std::size_t I>
        struct get_fn
        {
            [[nodiscard]] constexpr decltype(auto) operator()() const noexcept
            {
                return base::template get<I>();
            }
        };

    public:
        template<::std::size_t I>
        static constexpr value_sequence::get_fn<I> get{};

        static constexpr auto size = sizeof...(Values);

        using index_seq = typename base::index_seq;

        static constexpr auto invoke = []<typename Func>
            requires(std::invocable<Func, decltype(Values)>&&...) // clang-format off
            (Func&& func) noexcept(
                (::std_sharp::utility::nothrow_invocable<Func, decltype(Values)> && ...)
            ) ->decltype(auto) // clang-format on
        {
            return ::std_sharp::utility::merge_invoke(::std::bind_front(func, Values)...);
        };

        template<template<auto...> typename T>
        using apply_t = T<Values...>;

        template<::std::size_t... OtherInts>
        using indexed_t =
            ::std_sharp::utility::traits::regular_value_sequence<get<OtherInts>()...>;

        template<typename Seq>
        using indexed_by_seq_t = typename ::std_sharp::utility::traits:: //
            take_value_sequence<Seq>::template apply_t<indexed_t>;

    private:
        template<auto... Func>
        struct transform_fn
        {
            [[nodiscard]] constexpr auto operator()() const noexcept
            {
                if constexpr(sizeof...(Func) == 1)
                    return []<auto F>(const ::std_sharp::utility::constant<F>) noexcept
                    {
                        return ::std_sharp::utility::traits:: //
                            value_sequence<::std::invoke(F, Values)...>{};
                    }
                (::std_sharp::utility::constant<Func...>{});
                else return ::std_sharp::utility::traits:: //
                    value_sequence<::std::invoke(Func, Values)...>{}; // clang-format on
            };
        };

        template<::std::size_t From, ::std::size_t... I>
        static constexpr indexed_t<From + I...> select_range_indexed( // clang-format off
            ::std::index_sequence<I...>
        ) noexcept; // clang-format on

    public:
        static constexpr struct
        {
            template<
                typename Func,
                ::std_sharp::utility::traits::details:: //
                value_sequence_invocable<Func, Values...> Proj = ::std::identity // clang-format off
                > // clang-format on
            constexpr auto operator()(Func func, Proj proj = {}) const noexcept( //
                ::std_sharp::utility::traits::details::
                    value_sequence_nothrow_invocable<Proj, Func, Values...> //
            )
            {
                (::std::invoke(func, ::std::invoke(proj, Values)), ...);
                return func;
            }
        } for_each{};

        static constexpr struct
        {
            template<
                typename Func,
                ::std_sharp::utility::traits::details:: //
                value_sequence_invocable<Func, Values...> Proj = ::std::identity // clang-format off
            > // clang-format on
            constexpr auto operator()(auto for_each_n_count, Func func, Proj proj = {}) const
                noexcept( //
                    ::std_sharp::utility::traits::details::
                        value_sequence_nothrow_invocable<Proj, Func, Values...> //
                )
            {
                const auto f = [&]<typename T>(T&& v) noexcept( // clang-format off
                    ::std_sharp::utility::nothrow_invocable<Func, std::invoke_result_t<Proj, T>>
                ) // clang-format on
                {
                    if(for_each_n_count == 0) return false;
                    ::std::invoke(func, ::std::invoke(proj, ::std::forward<T>(v)));
                    --for_each_n_count;
                    return true;
                };

                (f(Values) && ...);
                return func;
            }
        } for_each_n{};

        static constexpr struct
        {
            template<
                typename Func,
                ::std_sharp::utility::traits::details:: //
                value_sequence_predicate<Func, Values...> Proj = ::std::identity // clang-format off
            > // clang-format on
            [[nodiscard]] constexpr auto operator()(Func func, Proj proj = {}) const
                noexcept(details::value_sequence_nothrow_predicate<Proj, Func, Values...>)
            {
                ::std::size_t i = 0; // clang-format off
                const auto f = [&func, &proj, &i]<typename T>(T&& v) noexcept(
                    ::std_sharp::utility::
                        nothrow_invocable_r<bool, Func, ::std::invoke_result_t<Proj, T>>
                ) // clang-format on
                {
                    if(::std_sharp::utility:: //
                       invoke_r<bool>(func, ::std::invoke(proj, ::std::forward<T>(v))))
                        return false;
                    ++i;
                    return true;
                };

                (f(Values) && ...);

                return i;
            }
        } find_if{};

        static constexpr struct
        {
            template<typename Func, typename Proj = ::std::identity>
            [[nodiscard]] constexpr auto operator()(Func func, Proj&& proj = {}) const
                noexcept(::std_sharp::utility::traits::details::
                             value_sequence_nothrow_predicate<Proj, Func, Values...>)
            {
                return find_if(
                    [&](const auto& v) noexcept(
                        ::std_sharp::utility::nothrow_invocable_r<bool, Func, decltype(v)>)
                    {
                        return !::std_sharp::utility::invoke_r<bool>(func, v); //
                    },
                    ::std::forward<Proj>(proj) //
                );
            }
        } find_if_not{};

        static constexpr struct
        {
            template<typename Comp = ::std::ranges::equal_to, typename Proj = ::std::identity>
            [[nodiscard]] constexpr auto operator()( //
                const auto& v,
                Comp comp = {},
                Proj&& proj = {} // clang-format off
            ) const noexcept(
                ::std_sharp::utility::nothrow_invocable<
                    decltype(value_sequence::find_if),
                    decltype(base::value_comparer(v, comp)),
                    Proj
                >
            ) // clang-format on
            {
                return find_if(base::value_comparer(v, comp), std::forward<Proj>(proj));
            }
        } find{};

        static constexpr struct
        {
            template<typename Func, typename Proj = std::identity>
            [[nodiscard]] constexpr auto operator()(Func func, Proj&& proj = {}) const
                noexcept(::std_sharp::utility::traits::details::
                             value_sequence_nothrow_predicate<Proj, Func, Values...>)
            {
                std::size_t i = 0;
                for_each(
                    [&i, &func](const auto& v) noexcept(
                        ::std_sharp::utility::nothrow_invocable_r<bool, Func, decltype(v)> //
                    )
                    {
                        if(::std_sharp::utility::invoke_r<bool>(func, v)) ++i;
                        return true;
                    },
                    ::std::forward<Proj>(proj) //
                );

                return i;
            }
        } count_if{};

        static constexpr struct
        {
            template<typename Func, typename Proj = std::identity>
            [[nodiscard]] constexpr auto operator()(Func func, Proj&& proj = {}) const
                noexcept(::std_sharp::utility::traits::details::
                             value_sequence_nothrow_predicate<Proj, Func, Values...>)
            {
                return count_if(
                    [&](const auto& v) noexcept(
                        ::std_sharp::utility::nothrow_invocable_r<bool, Func, decltype(v)>)
                    {
                        return !::std_sharp::utility::invoke_r<bool>(func, v); //
                    },
                    ::std::forward<Proj>(proj) //
                );
            }
        } count_if_not{};

        static constexpr struct
        {
            template<typename Comp = ::std::ranges::equal_to, typename Proj = ::std::identity>
            [[nodiscard]] constexpr auto operator()( //
                const auto& v,
                Comp comp = {},
                Proj&& proj = {} // clang-format off
            ) const noexcept(::std_sharp::utility::nothrow_invocable<
                decltype(count_if),
                decltype(base::value_comparer(v, comp)),
                Proj
            >
            ) // clang-format on
            {
                return count_if(base::value_comparer(v, comp), ::std::forward<Proj>(proj));
            }
        } count{};

        static constexpr struct
        {
            template<typename Func, typename Proj = ::std::identity>
            [[nodiscard]] constexpr auto operator()(Func&& func, Proj&& proj = {}) const
                noexcept(::std_sharp::utility::
                             nothrow_invocable<decltype(value_sequence::find_if_not), Func, Proj>)
            {
                return find_if_not(std::forward<Func>(func), std::forward<Proj>(proj)) == size;
            }
        } all_of{};

        static constexpr struct
        {
            template<typename Func, typename Proj = ::std::identity>
            [[nodiscard]] constexpr auto operator()(Func&& func, Proj&& proj = {}) const
                noexcept(::std_sharp::utility::
                             nothrow_invocable<decltype(value_sequence::find_if), Func, Proj>)
            {
                return find_if(::std::forward<Func>(func), ::std::forward<Proj>(proj)) != size;
            }
        } any_of{};

        static constexpr struct
        {
            template<typename Func, typename Proj = ::std::identity>
            [[nodiscard]] constexpr auto operator()(Func&& func, Proj&& proj = {}) const
                noexcept(::std_sharp::utility::
                             nothrow_invocable<decltype(value_sequence::find_if), Func, Proj>)
            {
                return find_if(::std::forward<Func>(func), ::std::forward<Proj>(proj)) == size;
            }
        } none_of{};

        static constexpr struct
        {
            template<typename Proj = ::std::identity>
            [[nodiscard]] constexpr auto operator()(const auto& v, Proj&& proj = {}) const
                noexcept(::std_sharp::utility::
                             nothrow_invocable<decltype(value_sequence::find), decltype(v), Proj>)
            {
                return find(v, ::std::forward<Proj>(proj)) != size;
            }
        } contains{};

        static constexpr struct
        {
        private:
            template<::std::size_t I, typename Comp, typename Proj>
            struct by_index
            {
                using left_projected_t = std::invoke_result_t<Proj, decltype(get<I>())>;
                using right_projected_t = std::invoke_result_t<Proj, decltype(get<I + 1>())>;

                static constexpr auto invoke(Comp& comp, Proj& proj) noexcept(
                    nothrow_invocable_r<bool, Comp, left_projected_t, right_projected_t>) requires
                    invocable_r<bool, Comp, left_projected_t, right_projected_t>

                {
                    return ::std_sharp::utility::invoke_r<bool>(
                        comp, ::std::invoke(proj, get<I>()), ::std::invoke(proj, get<I + 1>()) //
                    );
                }

                static constexpr auto invoke(Comp&, Proj&) noexcept { return false; }
            };

            struct impl
            {
                template<typename Comp, typename Proj, ::std::size_t... I>
                constexpr auto operator()(
                    Comp& comp,
                    Proj& proj,
                    const ::std::index_sequence<I...> // clang-format off
                ) noexcept(noexcept((by_index<I, Comp, Proj>::invoke(comp, proj), ...))) // clang-format on
                {
                    ::std::size_t res{};
                    ((
                         [&]<::std::size_t J>(const ::std_sharp::utility::constant<J>) //
                         noexcept(noexcept(by_index<J, Comp, Proj>::invoke(comp, proj)))
                         {
                             if(by_index<J, Comp, Proj>::invoke(comp, proj)) return true;
                             ++res;
                             return false;
                         }(::std_sharp::utility::constant<I>{})) ||
                     ...);
                    return res;
                }
            };

        public:
            template<typename Comp = ::std::ranges::equal_to, typename Proj = ::std::identity>
                requires(::std::invocable<Proj, decltype(Values)>&&...)
            [[nodiscard]] constexpr auto operator()(Comp comp = {}, Proj proj = {}) const noexcept(
                ::std_sharp::utility::
                    nothrow_invocable<impl, Comp, Proj, ::std::make_index_sequence<size - 2>> //
            )
            {
                return impl{}(comp, proj, ::std::make_index_sequence<size - 2>{});
            }

            template<typename Comp = ::std::nullptr_t, typename Proj = ::std::nullptr_t>
            [[nodiscard]] constexpr auto operator()(const Comp& = {}, const Proj& = {}) const //
                noexcept requires(size < 2)
            {
                return size;
            }
        } adjacent_find{};

        template<auto... Func>
        static constexpr transform_fn<Func...> transform{};

        template<auto... Func>
        using transform_t = decltype(transform<Func...>());

        template<std::size_t From, std::size_t Size>
        using select_range_indexed_t =
            decltype(select_range_indexed<From>(std::make_index_sequence<Size>{}));

        template<std::size_t Size>
        using back_t = select_range_indexed_t<size - Size, Size>;

        template<std::size_t Size>
        using front_t = select_range_indexed_t<0, Size>;

        template<auto... Others>
        using append_t =
            ::std_sharp::utility::traits::regular_value_sequence<Values..., Others...>;

        template<typename Seq>
        using append_by_seq_t = typename ::std_sharp::utility::traits:: //
            take_value_sequence<Seq>::template apply_t<append_t>;

        template<auto... Others>
        using append_front_t =
            ::std_sharp::utility::traits::regular_value_sequence<Others..., Values...>;

        template<typename Seq>
        using append_front_by_seq_t = typename ::std_sharp::utility::traits:: //
            take_value_sequence<Seq>::template apply_t<append_front_t>;

    private:
        template<::std::size_t Index>
        struct insert
        {
            template<auto... Others>
            using type = typename ::std_sharp::utility::traits::to_value_sequence_t<
                typename ::std_sharp::utility::traits::to_value_sequence_t<front_t<Index>>:: //
                template append_t<Others...> // clang-format off
            >::template append_by_seq_t<back_t<size - Index>>; // clang-format on
        };

        // TODO replace it with lambda cause compile errors
        template<::std::size_t... Index>
        struct remove_at
        {
            static constexpr auto select_indices = []() noexcept
            {
                ::std::array<std::size_t, size> res{};
                ::std::array excepted = {Index...};
                ::std::size_t res_i = 0;

                ::std::ranges::sort(excepted);

                {
                    ::std::size_t index = 0;
                    for(const auto excepted_i : excepted)
                    {
                        for(; index < excepted_i; ++index, ++res_i) res[res_i] = index;
                        ++index;
                    }
                    for(; index < size; ++index, ++res_i) res[res_i] = index;
                }

                return ::std::pair{res, res_i};
            }();

            template<::std::size_t... I>
            static constexpr indexed_t<select_indices.first[I]...>
                get_type(::std::index_sequence<I...>) noexcept;

            using type = decltype(get_type(::std::make_index_sequence<select_indices.second>{}));
        };

    public:
        template<::std::size_t Index, auto... Other>
        using insert_t = typename insert<Index>::template type<Other...>;

        template<::std::size_t Index, typename Seq>
        using insert_by_seq_t =
            typename take_value_sequence<Seq>::template apply_t<insert<Index>::template type>;

        template<::std::size_t... Index>
        using remove_at_t = typename remove_at<Index...>::type;

        template<typename Seq>
        using remove_at_by_seq_t = typename take_value_sequence<Seq>::template apply_t<remove_at_t>;

        template<::std::size_t Index, auto Other>
        using replace_t = typename ::std_sharp::utility::traits::to_value_sequence_t<
            typename ::std_sharp::utility::traits:: //
            to_value_sequence_t<front_t<Index>>::template append_t<Other> // clang-format off
        >::template append_by_seq_t<back_t<size - Index - 1>>; // clang-format on
    };
}

namespace std
{
    template<::std::size_t I, auto... Values>
    struct tuple_element<I, ::std_sharp::utility::traits::value_sequence<Values...>> :
        ::std::type_identity< // clang-format off
            decltype(
                typename ::std_sharp::utility::traits::
                    value_sequence<Values...>::
                    template get<I>()
            )
        > // clang-format on
    {
    };

    template<auto... Values>
    struct tuple_size<::std_sharp::utility::traits::value_sequence<Values...>> :
        ::std_sharp::utility::index_constant<
            ::std_sharp::utility::traits::value_sequence<Values...>::size // clang-format off
        > // clang-format on
    {
    };
}
