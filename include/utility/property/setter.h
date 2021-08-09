// Created by BlurringShadow at 2021-03-11-下午 4:30

#pragma once

#include "utility/functional.h"

namespace blurringshadow::utility::property
{
    template<typename SetterFn>
    class setter : public ::blurringshadow::utility::invocable_obj<SetterFn>
    {
    public:
        using base = ::blurringshadow::utility::invocable_obj<SetterFn>;

        using base::base;

        template<typename T>
        constexpr auto& operator=(T&& t) const
            noexcept(noexcept(base::operator()(::std::forward<T>(t))))
        {
            base::operator()(::std::forward<T>(t));
            return *this;
        }

        template<typename T>
        constexpr auto& operator=(T&& t) noexcept(noexcept(base::operator()(::std::forward<T>(t))))
        {
            base::operator()(std::forward<T>(t));
            return *this;
        }
    };

    template<typename T>
    setter(T&& t) -> setter<::std::remove_cvref_t<T>>;

    inline constexpr auto value_setter = [](auto& t) noexcept
    {
        return ::blurringshadow::utility::property::setter{
            ::blurringshadow::utility::bind_ref_front(assign_v, t) //
        };
    };
}
