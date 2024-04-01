#pragma once

#include <type_traits>
#include <tuple>

#include "../utility/adl_proof.h"
#include "../macros.h"

namespace stdsharp::cpo::inline cpo_impl
{
    void get(auto&&) = delete;

    template<std::size_t I>
    struct get_element_fn
    {
        [[nodiscard]] constexpr decltype(auto) operator()(auto&& t) const
            noexcept(noexcept(cpp_forward(t).template get<I>()))
            requires requires { cpp_forward(t).template get<I>(); }
        {
            return cpp_forward(t).template get<I>();
        }

        [[nodiscard]] constexpr decltype(auto) operator()(auto&& t) const
            noexcept(noexcept(get<I>(cpp_forward(t))))
            requires requires {
                get<I>(cpp_forward(t));
                requires !requires { cpp_forward(t).template get<I>(); };
            }
        {
            return get<I>(cpp_forward(t));
        }
    };

    template<std::size_t I>
    inline constexpr get_element_fn<I> get_element;
}

namespace stdsharp
{
    template<std::size_t I, typename T>
    using get_element_t = decltype(cpo::get_element<I>(std::declval<T>()));

    template<typename...>
    struct basic_type_sequence;

    template<auto...>
    struct regular_value_sequence;
}

namespace std
{
    template<typename... T>
    struct tuple_size<::stdsharp::basic_type_sequence<T...>>
    {
        static constexpr auto value = ::stdsharp::basic_type_sequence<T...>::size();
    };

    template<::stdsharp::adl_proofed_for<::stdsharp::basic_type_sequence> T>
    struct tuple_size<T> : tuple_size<typename T::basic_type_sequence>
    {
    };

    template<auto... V>
    struct tuple_size<::stdsharp::regular_value_sequence<V...>>
    {
        static constexpr auto value = ::stdsharp::regular_value_sequence<V...>::size();
    };
}