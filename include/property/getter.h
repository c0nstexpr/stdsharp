// Created by BlurringShadow at 2021-03-11-下午 4:42

#pragma once

#include "functional/functional.h"

namespace stdsharp::property
{
    template<::std::invocable GetterFn>
    class getter : public functional::invocable_obj<GetterFn>
    {
    public:
        using base = functional::invocable_obj<GetterFn>;

        using base::base;

        using value_type = ::std::invoke_result_t<GetterFn>;
    };

    template<typename T>
    getter(T&& t) -> getter<::std::remove_cvref_t<T>>;

    inline constexpr functional::invocable_obj value_getter{
        functional::nodiscard_tag,
        [](auto& t) noexcept
        {
            return getter{functional::bind_ref_front(functional::identity_v, t)}; //
        } //
    };

    template<typename GetterFn>
    using getter_value_t = ::std::remove_cvref_t<::std::invoke_result_t<GetterFn>>;

    template<typename GetterFn>
    using getter_reference_t = getter_value_t<GetterFn>&;

    template<typename GetterFn>
    using getter_const_reference_t = const getter_value_t<GetterFn>&;
}
