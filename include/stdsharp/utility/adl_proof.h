#pragma once

#include "../namespace_alias.h"

#include <concepts>
#include <type_traits>

namespace stdsharp
{
    template<template<typename...> typename Inner, typename... T>
    struct adl_proof_traits
    {
        struct proofed_t : Inner<T...>
        {
            using Inner<T...>::Inner;
        };

    private:
        friend consteval adl_proof_traits get_traits(const proofed_t&);
    };

    template<template<typename...> typename Inner, typename... T>
    using adl_proof_t = typename adl_proof_traits<Inner, T...>::proofed_t;
}

namespace stdsharp::details
{
    template<typename T, template<typename...> typename Proofed>
    struct adl_proofed_for
    {
        template<typename>
        struct traits;

        template<template<typename...> typename Inner, typename... U>
        struct traits<adl_proof_traits<Inner, U...>>
        {
            static constexpr auto v =
                std::same_as<typename adl_proof_traits<Inner, U...>::proofed_t, T> &&
                std::same_as<T, Proofed<U...>>;
        };
    };
}

namespace stdsharp
{
    template<typename T, template<typename...> typename Proofed>
    concept adl_proofed_for = requires(T t) {
        requires details::adl_proofed_for<T, Proofed>::template traits<
            decltype(get_traits(t))>::v;
    };
}