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
        friend consteval adl_proof_traits
            get_traits(const std::type_identity<proofed_t> /*unused*/) noexcept
        {
            return {};
        }
    };

    template<template<typename...> typename Inner, typename... T>
    using adl_proof_t = typename adl_proof_traits<Inner, T...>::proofed_t;
}

namespace stdsharp::details
{
    template<typename T, template<typename...> typename Proofed>
    struct adl_proofed_for
    {
        template<typename = decltype(get_traits(std::type_identity<T>{}))>
        struct traits;

        template<template<typename...> typename Inner, typename... U>
        struct traits<adl_proof_traits<Inner, U...>>
        {
            static constexpr auto v =
                std::same_as<typename adl_proof_traits<Inner, U...>::proofed_t, T>;
        };
    };
}

namespace stdsharp
{
    template<typename T, template<typename...> typename Proofed>
    concept adl_proofed_for =
        requires { requires details::adl_proofed_for<T, Proofed>::template traits<>::v; };
}