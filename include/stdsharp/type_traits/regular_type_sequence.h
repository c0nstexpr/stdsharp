#pragma once

#include "../utility/adl_proof.h"

namespace stdsharp
{
    template<typename T>
    struct basic_type_constant : std::type_identity<T>
    {
        template<typename U>
        [[nodiscard]] constexpr bool
            operator==(const basic_type_constant<U> /*unused*/) const noexcept
        {
            return std::same_as<T, U>;
        }
    };

    template<typename T>
    basic_type_constant(std::type_identity<T>) -> basic_type_constant<T>;

    template<typename T>
    using type_constant = adl_proof_t<basic_type_constant, T>;

    template<typename... T>
    struct basic_type_sequence
    {
        [[nodiscard]] static constexpr auto size() noexcept { return sizeof...(T); }
    };

    template<typename... T>
    using regular_type_sequence = adl_proof_t<basic_type_sequence, T...>;
}