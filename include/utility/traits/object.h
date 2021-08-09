// Created by BlurringShadow at 2021-03-05-下午 11:53

#pragma once

#include "type_sequence.h"

namespace blurringshadow::utility::traits
{
    struct unique_object
    {
        unique_object() noexcept = default;
        unique_object(const unique_object&) = delete;
        unique_object(unique_object&&) noexcept = default;
        unique_object& operator=(const unique_object&) = delete;
        unique_object& operator=(unique_object&&) noexcept = default;
        ~unique_object() = default;
    };

    template<typename T>
    struct private_object
    {
        friend T;

    private:
        private_object() = default;
    };
}
