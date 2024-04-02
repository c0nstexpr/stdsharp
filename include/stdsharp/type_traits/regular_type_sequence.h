#pragma once

#include "../utility/adl_proof.h"

namespace stdsharp
{
    template<typename... T>
    struct basic_type_sequence
    {
        [[nodiscard]] static constexpr auto size() noexcept { return sizeof...(T); }
    };

    template<typename... T>
    using regular_type_sequence = adl_proof_t<basic_type_sequence, T...>;
}