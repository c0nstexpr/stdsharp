#pragma once

#include <concepts>

namespace stdsharp
{
    namespace details
    {
        template<template<typename...> typename Inner, typename... T>
        struct adl_proof
        {
            struct t_ : Inner<T...>
            {
                using Inner<T...>::Inner;
            };
        };
    }

    template<template<typename...> typename Inner, typename... T>
    using adl_proof_t = typename details::adl_proof<Inner, T...>::t_;
}