// Created by BlurringShadow at 2021-02-27-下午 10:24

#pragma once

#include <random>

#include "../details/prologue.h"

namespace stdsharp
{
    inline constexpr struct
    {
        [[nodiscard]] auto& operator()() const
        {
            static thread_local ::std::random_device random_device;
            return random_device;
        }
    } get_random_device{};
}

#include "../details/epilogue.h"