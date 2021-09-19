// Created by BlurringShadow at 2021-03-11-下午 4:42

#pragma once

#include "functional/functional.h"

namespace stdsharp::utility::property
{
    template<::std::invocable GetterFn>
    class getter : public ::stdsharp::functional::invocable_obj<GetterFn>
    {
    public:
        using base = ::stdsharp::functional::invocable_obj<GetterFn>;

        using base::base;

        using value_type = ::std::invoke_result_t<GetterFn>;
    };

    template<typename T>
    getter(T&& t) -> getter<::std::remove_cvref_t<T>>;

    inline constexpr ::stdsharp::functional::invocable_obj value_getter{
        ::stdsharp::functional::nodiscard_tag,
        [](auto& t) noexcept
        {
            return ::stdsharp::utility::property::getter{
                ::stdsharp::functional::bind_ref_front(::stdsharp::functional::identity_v, t) //
            };
        } //
    };
}
