
#pragma once

#include <random>

namespace stdsharp
{
    inline constexpr struct
    {
        [[nodiscard]] auto& operator()() const
        {
            thread_local ::std::random_device random_device;
            return random_device;
        }
    } get_random_device{};
}