#pragma once

#include <range/v3/utility/tuple_algorithm.hpp>

#include "../functional/operations.h"

namespace stdsharp
{
    namespace cpo
    {
        namespace details
        {
            template<::std::size_t>
            void get(auto) = delete;

            template<::std::size_t N>
            struct get_fn
            {
                template<typename T>
                    requires requires { get<N>(::std::declval<T>()); }
                [[nodiscard]] constexpr decltype(auto) operator()(T&& t) const
                    noexcept(noexcept(get<N>(::std::declval<T>())))
                {
                    return get<N>(::std::forward<T>(t));
                }
            };
        }

        inline namespace cpo_impl
        {
            using details::get_fn;

            template<::std::size_t N>
            inline constexpr get_fn<N> get{};
        }
    }

    template<::std::size_t N, typename T>
    using get_t = ::std::invoke_result_t<cpo::get_fn<N>, T>;

    template<typename T>
    using type_size_seq_t = ::std::make_index_sequence<::std::tuple_size_v<::std::decay_t<T>>>;

    inline constexpr struct tuples_apply_fn
    {
    private:
        struct apply_coord
        {
            template<
                auto Coords,
                ::std::size_t... First,
                ::std::size_t... Second,
                typename Fn,
                typename... Tuple // clang-format off
            > // clang-format on
                requires ::std::invocable<Fn, get_t<Second, pack_get_t<First, Tuple>>...>
            constexpr decltype(auto) operator()(
                const type_traits::constant<Coords>,
                const ::std::index_sequence<First...>,
                const ::std::index_sequence<Second...>,
                Fn&& fn,
                Tuple&&... tuple // clang-format off
            ) const noexcept(concepts::nothrow_invocable<Fn, get_t<Second, pack_get_t<First, Tuple>>...>)
            { // clang-format on
                return ::std::invoke(
                    ::std::forward<Fn>(fn),
                    get<Second>(pack_get<First>(::std::forward<Tuple>(tuple)...))... //
                );
            }
        };

        struct construct_coords_seq
        {
            template<
                typename ConstantT,
                ::std::size_t... I,
                typename Fn,
                typename... Tuple,
                auto Coords = ConstantT::value,
                typename FirstSeq = ::std::index_sequence<Coords[I].first...>,
                typename SecondSeq = ::std::index_sequence<Coords[I].second...> // clang-format off
            > // clang-format on
                requires ::std::invocable<apply_coord, ConstantT, FirstSeq, SecondSeq, Fn, Tuple...>
            constexpr decltype(auto) operator()(
                const ConstantT,
                const ::std::index_sequence<I...>,
                Fn&& fn,
                Tuple&&... tuple // clang-format off
            ) const noexcept( // clang-format on
                concepts::
                    nothrow_invocable<apply_coord, ConstantT, FirstSeq, SecondSeq, Fn, Tuple...> //
            )
            {
                return apply_coord{}(
                    FirstSeq{},
                    SecondSeq{},
                    ::std::forward<Fn>(fn),
                    ::std::forward<Tuple>(tuple)... //
                );
            }
        };

        template<typename... Tuple>
        static constexpr auto get_coords() noexcept
        {
            struct local
            {
                ::std::size_t i, j;
            };

            constexpr ::std::array sizes{::std::tuple_size_v<Tuple>...};
            ::std::array<local, (::std::tuple_size_v<Tuple> + ...)> coords{};

            for(::std::size_t i = 0, index = 0; i < sizeof...(Tuple); ++i)
            {
                const auto size = sizes[i];
                for(::std::size_t j = 0; j < size; ++j) coords[index++] = {i, j};
            }
            return coords;
        }

        struct impl
        {
            template<
                typename Fn,
                typename... Tuple,
                auto Coords = get_coords<Tuple...>(),
                typename Constant = type_traits::constant<Coords>,
                typename Seq = ::std::index_sequence<Coords.size()> // clang-format off
            > // clang-format on
                requires ::std::invocable<construct_coords_seq, Constant, Seq, Fn, Tuple...>
            constexpr decltype(auto) operator()(Fn&& fn, Tuple&&... tuple) const
                noexcept(concepts::nothrow_invocable<construct_coords_seq, Seq, Fn, Tuple...>)
            {
                return construct_coords_seq{}(
                    Constant{},
                    Seq{},
                    ::std::forward<Fn>(fn),
                    ::std::forward<Tuple>(tuple)... //
                );
            };
        };

    public:
        template<typename Fn, typename... Tuple>
            requires ::std::invocable<impl, Fn, Tuple...>
        constexpr decltype(auto) operator()(Fn&& fn, Tuple&&... tuple) const
            noexcept(concepts::nothrow_invocable<impl, Fn, Tuple...>)
        {
            return impl{}(::std::forward<Fn>(fn), ::std::forward<Tuple>(tuple)...);
        };
    } tuples_apply{};

    template<typename... T>
    concept tuples_applicable = ::std::invocable<tuples_apply_fn, T...>;

    template<typename... T>
    concept nothrow_tuples_applicable = concepts::nothrow_invocable<tuples_apply_fn, T...>;

    template<typename... T>
    struct tuples_each_apply_fn
    {
        template<::std::invocable<T...> Fn, typename... Tuple>
            requires(tuples_applicable<functional::constructor_fn<T>, Tuple> && ...)
        constexpr decltype(auto) operator()(Fn&& fn, Tuple&&... tuple) const noexcept(
            concepts::nothrow_invocable<Fn, T...> &&
            (nothrow_tuples_applicable<functional::constructor_fn<T>, Tuple> && ...) //
        )
        {
            return ::std::invoke(
                fn, //
                tuple_apply(functional::constructor<T>, ::std::forward<Tuple>(tuple))... //
            );
        };
    };

    template<typename... T>
    inline constexpr tuples_each_apply_fn<T...> tuples_each_apply{};

    template<typename T>
    struct make_from_tuples_fn
    {
        template<typename... Tuple>
            requires tuples_applicable<functional::constructor_fn<T>, Tuple...>
        constexpr T make_from_tuple(Tuple&&... t) const
            noexcept(nothrow_tuples_applicable<functional::constructor_fn<T>, Tuple...>)
        {
            return tuples_apply(functional::constructor<T>, std::forward<Tuple>(t)...);
        }
    };

    template<typename T>
    inline constexpr make_from_tuples_fn<T> make_from_tuples{};

    inline constexpr struct tuples_cat_fn
    {
        template<typename... Tuples>
        constexpr auto operator()(Tuples&&... t)
        {
            return tuples_apply(
                []<typename... Args>(Args&&... args)
                { return ::std::tuple{::std::forward<Args>(args)...}; },
                std::forward<Tuples>(t)... //
            );
        }
    } tuple_cat{};
}

namespace std
{
    template<size_t I, typename TupleLike>
        requires requires { typename ::stdsharp::get_t<I, TupleLike>; }
    struct tuple_element<I, TupleLike> : // NOLINT(cert-dcl58-cpp)
        type_identity<::stdsharp::get_t<I, TupleLike>>
    {
    };
}