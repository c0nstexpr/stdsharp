#pragma once

#include <concepts>
#include <type_traits>

#include "../namespace_alias.h"

namespace stdsharp
{
    template<typename T>
    struct adl_accessor
    {
    private:
        template<template<typename...> typename Proofed>
        struct same_as_impl
        {
            template<template<typename...> typename Inner, typename... U>
            static consteval auto value(const std::type_identity<Inner<U...>>)
            {
                return std::same_as<T, Proofed<U...>>;
            }
        };

    public:
        template<template<typename...> typename Proofed>
        static constexpr auto same_as = requires {
            requires same_as_impl<Proofed>::value(std::type_identity<typename T::inner>{});
        };
    };

    template<template<typename...> typename Inner, typename... T>
    struct adl_proof_traits
    {
        struct proofed_t : Inner<T...>
        {
            friend adl_accessor<proofed_t>;

            using Inner<T...>::Inner;

        private:
            using inner = Inner<T...>;
        };
    };

    template<template<typename...> typename Inner, typename... T>
    using adl_proof_t = typename adl_proof_traits<Inner, T...>::proofed_t;

    template<typename T, template<typename...> typename Proofed>
    concept adl_proofed_for = adl_accessor<T>::template same_as<Proofed>;
}