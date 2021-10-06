// Created by BlurringShadow at 2021-03-04-下午 11:27

#pragma once

#include <algorithm>
#include <ranges>
#include <variant>

#include "functional/functional.h"

namespace stdsharp::type_traits
{
    namespace details
    {
        template<auto From, auto PlusF, ::std::size_t... I>
            requires requires
            {
                ::stdsharp::type_traits::regular_value_sequence<::std::invoke(PlusF, From, I)...>{};
            }
        constexpr auto make_value_sequence(::std::index_sequence<I...>) noexcept
        {
            return ::stdsharp::type_traits::regular_value_sequence<::std::invoke(
                PlusF, From, I)...>{};
        }

        template<typename Func>
        constexpr auto make_value_predicator(Func& func) noexcept
        {
            return [&func](const auto& v) // clang-format off
                noexcept(::stdsharp::concepts::nothrow_predicate<Func, decltype(v)>) // clang-format on
            {
                return !::stdsharp::functional::invoke_r<bool>(func, v); //
            };
        }
    }

    template<auto From, ::std::size_t Size, auto PlusF = ::stdsharp::functional::plus_v>
    using make_value_sequence_t = decltype( //
        ::stdsharp::type_traits::details::make_value_sequence<From, PlusF>(
            ::std::make_index_sequence<Size>{} // clang-format off
        ) // clang-format on
    );

    namespace details
    {
        template<auto... Values>
        struct reverse_value_sequence
        {
            using seq = ::stdsharp::type_traits::value_sequence<Values...>;
            using type = typename seq::template indexed_by_seq_t<
                ::stdsharp::type_traits::make_value_sequence_t<
                    seq::size - 1,
                    seq::size,
                    ::stdsharp::functional::minus_v> // clang-format off
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
                        return ::stdsharp::functional::equal_to_v(seq::template get<I>(), v);
                    }
                ) == I; // clang-format on
            }

            static constexpr auto filtered_indices =
                []<size_t... I>(const ::std::index_sequence<I...>)
            {
                ::std::array<size_t, seq::size> indices{};
                ::std::size_t valid_size = 0;
                const auto f = [&]<::std::size_t J>(const type_traits::constant<J>) noexcept
                {
                    if(is_valid<J>())
                    {
                        indices[valid_size] = J;
                        ++valid_size;
                    }
                };

                (f(type_traits::constant<I>{}), ...);

                return ::std::pair{indices, valid_size};
            }
            (typename seq::index_seq{});

            template<::std::size_t... I>
            using filtered_seq = ::stdsharp::type_traits:: //
                regular_value_sequence<seq::template get<filtered_indices.first[I]>()...>;

            using type = typename ::stdsharp::type_traits::take_value_sequence< //
                ::stdsharp::type_traits:: //
                make_value_sequence_t<::std::size_t{}, filtered_indices.second> // clang-format off
            >::template apply_t<filtered_seq>; // clang-format on
        };

        template<::std::size_t I, auto Value>
        struct indexed_value : stdsharp::type_traits::constant<Value>
        {
            template<::std::size_t J>
                requires(I == J)
            static constexpr decltype(auto) get() noexcept { return Value; }
        };

        template<typename, typename>
        struct value_sequence;

        template<auto... Values, size_t... I>
        struct value_sequence<
            ::stdsharp::type_traits::value_sequence<Values...>,
            ::std::index_sequence<I...>> :
            ::stdsharp::type_traits::details::indexed_value<I, Values>...
        {
            using indexed_value<I, Values>::get...;

            using index_seq = ::std::index_sequence<I...>;
        };

        template<typename T, typename Comp>
        static constexpr auto value_comparer(const T& value, Comp& comp = {}) noexcept
        {
            return ::ranges::overload(
                [&comp, &value]<typename U> requires ::std::invocable<Comp, U, T> //
                (const U& other) // clang-format off
                    noexcept(::stdsharp::concepts::nothrow_invocable_r<Comp, bool, U, T>)
                { // clang-format on
                    return ::stdsharp::functional::invoke_r<bool>(comp, other, value);
                },
                [](const auto&) noexcept { return false; } //
            );
        }

        template<typename Proj, typename Func, auto... Values>
        concept value_sequence_invocable =
            ((::std::invocable<Proj, decltype(Values)> &&
              ::std::invocable<Func, ::std::invoke_result_t<Proj, decltype(Values)>>)&&...);

        template<typename Proj, typename Func, auto... Values> // clang-format off
        concept value_sequence_nothrow_invocable = (
            ::stdsharp::concepts::
                nothrow_invocable<Func, ::std::invoke_result_t<Proj, decltype(Values)>> &&
            ...
        );

        template<typename Proj, typename Func, auto... Values>
        concept value_sequence_predicate =
            (::std::predicate<Func, ::std::invoke_result_t<Proj, decltype(Values)>> && ...);

        template<typename Proj, typename Func, auto... Values>
        concept value_sequence_nothrow_predicate = (
            ::stdsharp::concepts::
                nothrow_invocable_r<Func, bool, ::std::invoke_result_t<Proj, decltype(Values)>> &&
            ...
        );
    } // clang-format on

    template<auto... Values>
    using reverse_value_sequence_t = typename ::stdsharp::type_traits::details:: //
        reverse_value_sequence<Values...>::type;

    template<auto... Values>
    using unique_value_sequence_t =
        typename ::stdsharp::type_traits::details::unique_value_sequence<Values...>::type;

    template<auto... Values> // clang-format off
    struct value_sequence : private ::stdsharp::type_traits::details::value_sequence<
        ::stdsharp::type_traits::value_sequence<Values...>,
        ::std::make_index_sequence<sizeof...(Values)>
    > // clang-format on
    {
    private:
        using base = ::stdsharp::type_traits::details::value_sequence<
            ::stdsharp::type_traits::value_sequence<Values...>,
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
                (::stdsharp::concepts::nothrow_invocable<Func, decltype(Values)> && ...)
            ) ->decltype(auto) // clang-format on
        {
            return ::stdsharp::functional::merge_invoke<>(::std::bind_front(func, Values)...);
        };

        template<template<auto...> typename T>
        using apply_t = T<Values...>;

        template<::std::size_t... OtherInts>
        using indexed_t = ::stdsharp::type_traits::regular_value_sequence<get<OtherInts>()...>;

        template<typename Seq>
        using indexed_by_seq_t = typename ::stdsharp::type_traits:: //
            take_value_sequence<Seq>::template apply_t<indexed_t>;

    private:
        template<auto... Func>
        struct transform_fn
        {
            [[nodiscard]] constexpr auto operator()() const noexcept
            {
                if constexpr(sizeof...(Func) == 1)
                    return []<auto F>(const ::stdsharp::type_traits::constant<F>) noexcept
                    {
                        return ::stdsharp::type_traits:: //
                            value_sequence<::std::invoke(F, Values)...>{};
                    }
                (::stdsharp::type_traits::constant<Func...>{});
                else return ::stdsharp::type_traits:: //
                    value_sequence<::std::invoke(Func, Values)...>{}; // clang-format on
            };
        };

        template<::std::size_t From, ::std::size_t... I>
        static constexpr indexed_t<From + I...> select_range_indexed( // clang-format off
            ::std::index_sequence<I...>
        ) noexcept; // clang-format on

        template<typename IfFunc>
        struct if_not_fn : private IfFunc
        {
            template<typename Func, typename Proj = ::std::identity>
            [[nodiscard]] constexpr auto operator()(Func func, Proj&& proj = {}) const noexcept( //
                ::stdsharp::type_traits::details::
                    value_sequence_nothrow_predicate<Proj, Func, Values...> // clang-format off
            ) // clang-format on
            {
                return IfFunc::operator()(
                    ::stdsharp::type_traits::details::make_value_predicator(func),
                    ::std::forward<Proj>(proj) //
                );
            }
        };

        template<typename IfFunc>
        struct do_fn : private IfFunc
        {
            template<
                typename T,
                typename Comp = ::std::ranges::equal_to,
                typename Proj = ::std::identity // clang-format off
            > // clang-format on
                requires requires(T v, Comp comp)
                {
                    requires ::std::invocable<
                        IfFunc,
                        decltype(stdsharp::type_traits::details::value_comparer(v, comp)),
                        Proj // clang-format off
                    >; // clang-format on
                }
            [[nodiscard]] constexpr auto operator()( //
                const T& v,
                Comp comp = {},
                Proj&& proj = {} //
            ) const noexcept( //
                ::stdsharp::concepts::nothrow_invocable<
                    IfFunc,
                    decltype(stdsharp::type_traits::details::value_comparer(v, comp)),
                    Proj // clang-format off
                >
            ) // clang-format on
            {
                return IfFunc::operator()(
                    ::stdsharp::type_traits::details::value_comparer(v, comp),
                    ::std::forward<Proj>(proj) //
                );
            } //
        };

        template<typename FindFunc, bool Equal = true>
        struct algo_fn : private FindFunc
        {
            template<typename Func, typename Proj = ::std::identity>
                requires ::std::invocable<FindFunc, Func, Proj>
            [[nodiscard]] constexpr auto operator()(Func func, Proj&& proj = {}) const
                noexcept(::stdsharp::concepts::nothrow_invocable<FindFunc, Func, Proj>)
            {
                const auto v =
                    FindFunc::operator()(std::forward<Func>(func), std::forward<Proj>(proj));
                if constexpr(Equal) return v == size; // clang-format off
                else return v != size; // clang-format on
            }
        };

    public:
        static constexpr struct
        {
            template<
                typename Func,
                ::stdsharp::type_traits::details:: //
                value_sequence_invocable<Func, Values...> Proj = ::std::identity // clang-format off
            > // clang-format on
            constexpr auto operator()(Func func, Proj proj = {}) const noexcept( //
                ::stdsharp::type_traits::details::
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
                ::stdsharp::type_traits::details:: //
                value_sequence_invocable<Func, Values...> Proj = ::std::identity // clang-format off
            > // clang-format on
            constexpr auto operator()(auto for_each_n_count, Func func, Proj proj = {}) const
                noexcept( //
                    ::stdsharp::type_traits::details::
                        value_sequence_nothrow_invocable<Proj, Func, Values...> //
                )
            {
                const auto f = [&]<typename T>(T&& v) noexcept( // clang-format off
                    ::stdsharp::concepts::nothrow_invocable<Func, std::invoke_result_t<Proj, T>>
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
                ::stdsharp::type_traits::details:: //
                value_sequence_predicate<Func, Values...> Proj = ::std::identity // clang-format off
            > // clang-format on
            [[nodiscard]] constexpr auto operator()(Func func, Proj proj = {}) const
                noexcept(details::value_sequence_nothrow_predicate<Proj, Func, Values...>)
            {
                ::std::size_t i = 0; // clang-format off
                const auto f = [&func, &proj, &i]<typename T>(T&& v) noexcept(
                    ::stdsharp::concepts::
                        nothrow_invocable_r<Func, bool, ::std::invoke_result_t<Proj, T>>
                ) // clang-format on
                {
                    if(::stdsharp::functional:: //
                       invoke_r<bool>(func, ::std::invoke(proj, ::std::forward<T>(v))))
                        return false;
                    ++i;
                    return true;
                };

                (f(Values) && ...);

                return i;
            }
        } find_if{};

        static constexpr value_sequence::if_not_fn<decltype(find_if)> find_if_not{};

        static constexpr value_sequence::do_fn<decltype(find_if)> find{};

        static constexpr struct
        {
            template<typename Func, typename Proj = std::identity>
            [[nodiscard]] constexpr auto operator()(Func func, Proj&& proj = {}) const
                noexcept(::stdsharp::type_traits::details::
                             value_sequence_nothrow_predicate<Proj, Func, Values...>)
            {
                std::size_t i = 0;
                for_each(
                    [&i, &func](const auto& v) noexcept(
                        ::stdsharp::concepts::nothrow_invocable_r<Func, bool, decltype(v)> //
                    )
                    {
                        if(::stdsharp::functional::invoke_r<bool>(func, v)) ++i;
                        return true;
                    },
                    ::std::forward<Proj>(proj) //
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
            template<::std::size_t I, typename Comp, typename Proj>
            struct by_index
            {
                using left_projected_t = std::invoke_result_t<Proj, decltype(get<I>())>;
                using right_projected_t = std::invoke_result_t<Proj, decltype(get<I + 1>())>;

                static constexpr auto invoke(Comp& comp, Proj& proj) noexcept(
                    ::stdsharp::concepts:: // clang-format off
                        nothrow_invocable_r<Comp, bool, left_projected_t, right_projected_t>
                ) requires ::stdsharp::concepts::
                    invocable_r<Comp, bool, left_projected_t, right_projected_t> // clang-format on
                {
                    return ::stdsharp::functional::invoke_r<bool>(
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
                         [&]<::std::size_t J>(const ::stdsharp::type_traits::constant<J>) //
                         noexcept(noexcept(by_index<J, Comp, Proj>::invoke(comp, proj)))
                         {
                             if(by_index<J, Comp, Proj>::invoke(comp, proj)) return true;
                             ++res;
                             return false;
                         }(::stdsharp::type_traits::constant<I>{})) ||
                     ...);
                    return res;
                }
            };

        public:
            template<typename Comp = ::std::ranges::equal_to, typename Proj = ::std::identity>
                requires(::std::invocable<Proj, decltype(Values)>&&...)
            [[nodiscard]] constexpr auto operator()(Comp comp = {}, Proj proj = {}) const noexcept(
                ::stdsharp::concepts::
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
        using append_t = ::stdsharp::type_traits::regular_value_sequence<Values..., Others...>;

        template<typename Seq>
        using append_by_seq_t = typename ::stdsharp::type_traits:: //
            take_value_sequence<Seq>::template apply_t<append_t>;

        template<auto... Others>
        using append_front_t =
            ::stdsharp::type_traits::regular_value_sequence<Others..., Values...>;

        template<typename Seq>
        using append_front_by_seq_t = typename ::stdsharp::type_traits:: //
            take_value_sequence<Seq>::template apply_t<append_front_t>;

    private:
        template<::std::size_t Index>
        struct insert
        {
            template<auto... Others>
            using type = typename ::stdsharp::type_traits::to_value_sequence_t<
                typename ::stdsharp::type_traits::to_value_sequence_t<front_t<Index>>:: //
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
        using replace_t = typename ::stdsharp::type_traits::to_value_sequence_t<
            typename ::stdsharp::type_traits:: //
            to_value_sequence_t<front_t<Index>>::template append_t<Other> // clang-format off
        >::template append_by_seq_t<back_t<size - Index - 1>>; // clang-format on
    };
}

namespace std
{
    template<::std::size_t I, auto... Values>
    struct tuple_element<I, ::stdsharp::type_traits::value_sequence<Values...>> :
        ::std::type_identity< // clang-format off
            decltype(
                typename ::stdsharp::type_traits::
                    value_sequence<Values...>::
                    template get<I>()
            )
        > // clang-format on
    {
    };

    template<auto... Values>
    struct tuple_size<::stdsharp::type_traits::value_sequence<Values...>> :
        ::stdsharp::type_traits::index_constant<
            ::stdsharp::type_traits::value_sequence<Values...>::size // clang-format off
        > // clang-format on
    {
    };
}
