// Created by BlurringShadow at 2021-03-11-下午 4:30

#pragma once

#include "functional/functional.h"

namespace stdsharp::utility::property
{
    template<typename SetterFn>
    class setter : public ::stdsharp::functional::invocable_obj<SetterFn>
    {
    public:
        using base = ::stdsharp::functional::invocable_obj<SetterFn>;

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
            noexcept(::stdsharp::concepts::nothrow_invocable<const base, T>)
        {
            static_cast<const base&&> (*this)(::std::forward<T>(t));
            return *this;
        }

        template<typename T>
            requires ::std::invocable<SetterFn, T>
        constexpr auto& operator=(T&& t) && //
            noexcept(::stdsharp::concepts::nothrow_invocable<base, T>)
        {
            static_cast<base&&> (*this)(::std::forward<T>(t));
            return *this;
        }
    };

    template<typename T>
    setter(T&& t) -> setter<::std::remove_cvref_t<T>>;

    inline constexpr ::stdsharp::functional::invocable_obj value_setter{
        ::stdsharp::functional::nodiscard_tag,
        [](auto& t) noexcept
        {
            return ::stdsharp::utility::property::setter{
                ::stdsharp::functional::bind_ref_front(::stdsharp::functional::assign_v, t) //
            };
        } //
    };
}
