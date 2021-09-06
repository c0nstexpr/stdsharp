// Created by BlurringShadow at 2021-03-11-下午 4:30

#pragma once

#include "utility/functional.h"

namespace std_sharp::utility::property
{
    template<typename SetterFn>
    class setter : public ::std_sharp::utility::invocable_obj<SetterFn>
    {
    public:
        using base = ::std_sharp::utility::invocable_obj<SetterFn>;

        using base::base;

        template<typename T>
            requires ::std::invocable<const SetterFn, T>
        constexpr auto& operator=(T&& t) const& //
            noexcept(noexcept(base::operator()(::std::forward<T>(t))))
        {
            base::operator()(::std::forward<T>(t));
            return *this;
        }

        template<typename T>
            requires ::std::invocable<SetterFn, T>
        constexpr auto& operator=(T&& t) & //
            noexcept(noexcept(base::operator()(::std::forward<T>(t))))
        {
            base::operator()(std::forward<T>(t));
            return *this;
        }

        template<typename T>
            requires ::std::invocable<const SetterFn, T>
        constexpr auto& operator=(T&& t) const&& //
            noexcept(::std_sharp::utility::nothrow_invocable<const base, T>)
        {
            static_cast<const base&&> (*this)(::std::forward<T>(t));
            return *this;
        }

        template<typename T>
            requires ::std::invocable<SetterFn, T>
        constexpr auto& operator=(T&& t) && //
            noexcept(::std_sharp::utility::nothrow_invocable<base, T>)
        {
            static_cast<base&&> (*this)(::std::forward<T>(t));
            return *this;
        }
    };

    template<typename T>
    setter(T&& t) -> setter<::std::remove_cvref_t<T>>;

    inline constexpr ::std_sharp::utility::nodiscard_invocable_obj value_setter{
        [](auto& t) noexcept
        {
            return ::std_sharp::utility::property::setter{
                ::std_sharp::utility::bind_ref_front(assign_v, t) //
            };
        } //
    };
}
