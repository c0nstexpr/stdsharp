#pragma once

#include <type_traits>

namespace stdsharp
{
    template<template<typename...> typename Inner, typename... T>
    struct adl_proof_traits
    {
        struct proofed_t : Inner<T...>
        {
            using Inner<T...>::Inner;
            using adl_traits = adl_proof_traits<Inner, T...>;
        };

        template<
            template<typename...>
            typename Compared,
            template<typename, typename> typename Predicate = std::is_same>
        static constexpr auto same_as = Predicate<proofed_t, Compared<T...>>::value;
    };

    template<template<typename...> typename Inner, typename... T>
    using adl_proof_t = typename adl_proof_traits<Inner, T...>::proofed_t;
}