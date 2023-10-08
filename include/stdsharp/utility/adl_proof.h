#pragma once

#include <concepts>
#include <type_traits>

#include "../namespace_alias.h"

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
            get_adl_traits(const std::type_identity<proofed_t>) noexcept
        {
            return {};
        }
    };

    template<template<typename...> typename Inner, typename... T>
    using adl_proof_t = typename adl_proof_traits<Inner, T...>::proofed_t;
}

namespace stdsharp::details
{
    template<
        template<typename...>
        typename Proofed,
        template<template<typename...> typename, typename...>
        typename Traits,
        typename... T // clang-format off
    > // clang-format on
    consteval std::same_as<adl_proof_traits<Proofed, T...>> auto
        adl_proofed_traits(const Traits<Proofed, T...> t)
    {
        return t;
    }

    template<typename T, template<typename...> typename Proofed>
    inline constexpr auto adl_proofed_for = requires(
        decltype(details::adl_proofed_traits<Proofed>(get_adl_traits(std::type_identity<T>{}))) v
    ) { requires std::same_as<T, typename decltype(v)::proofed_t>; };
}

namespace stdsharp
{
    template<typename T, template<typename...> typename Proofed>
    concept adl_proofed_for = requires { requires details::adl_proofed_for<T, Proofed>; };
}