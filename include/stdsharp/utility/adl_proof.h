// Created by BlurringShadow at 2021-02-27-下午 10:24

#pragma once

namespace stdsharp
{
    namespace details
    {
        template<template<typename...> typename Inner, typename... T>
        struct adl_proof
        {
            struct t_ : Inner<T...>
            {
            };
        };
    }

    template<template<typename...> typename Inner, typename... T>
    using adl_proof_t = typename details::adl_proof<Inner, T...>::t_;
}