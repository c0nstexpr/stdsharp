// Created by BlurringShadow at 2021-03-11-下午 4:42

#pragma once

#include "utility/functional.h"

namespace blurringshadow::utility::property
{
    template<::std::invocable GetterFn>
    class getter : public ::blurringshadow::utility::nodiscard_invocable_obj<GetterFn>
    {
    public:
        using base = ::blurringshadow::utility::nodiscard_invocable_obj<GetterFn>;

        using base::base;

        using value_type = ::std::invoke_result_t<GetterFn>;
    };

    template<typename T>
    getter(T&& t) -> getter<::std::remove_cvref_t<T>>;

    inline constexpr ::blurringshadow::utility::nodiscard_invocable_obj value_getter{
        [](auto& t) noexcept
        {
            return ::blurringshadow::utility::property::getter{
                ::blurringshadow::utility::bind_ref_front(identity_v, t) //
            };
        } //
    };
}
