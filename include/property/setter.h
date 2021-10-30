// Created by BlurringShadow at 2021-03-11-下午 4:30

#pragma once

#include "functional/functional.h"

namespace stdsharp::property
{
    template<typename SetterFn>
    class setter : public functional::invocable_obj<SetterFn>
    {
    public:
        using base = functional::invocable_obj<SetterFn>;

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
            base::operator()(::std::forward<T>(t));
            return *this;
        }

        template<typename T>
            requires ::std::invocable<const SetterFn, T>
        constexpr auto& operator=(T&& t) const&& //
            noexcept(concepts::nothrow_invocable<const base, T>)
        {
            static_cast<const base&&> (*this)(::std::forward<T>(t));
            return *this;
        }

        template<typename T>
            requires ::std::invocable<SetterFn, T>
        constexpr auto& operator=(T&& t) && //
            noexcept(concepts::nothrow_invocable<base, T>)
        {
            static_cast<base&&> (*this)(::std::forward<T>(t));
            return *this;
        }
    };

    template<typename T>
    setter(T&& t) -> setter<::std::remove_cvref_t<T>>;

    inline constexpr functional::invocable_obj value_setter{
        functional::nodiscard_tag,
        [](auto& t) noexcept
        {
            return setter{functional::bind_ref_front(functional::assign_v, t)}; //
        } //
    };

    template<typename SetterT, typename ValueType>
    concept settable = ( //
        !::std::move_constructible<ValueType> ||
        ( //
            ::std::copy_constructible<ValueType> ? //
                ::std::invocable<SetterT, ValueType> && //
                    ::std::invocable<SetterT, ValueType&> &&
                    ::std::invocable<SetterT, const ValueType> &&
                    ::std::invocable<SetterT, const ValueType&> :
                ::std::invocable<SetterT, ValueType&&> // clang-format off
        ) // clang-format on
    );
}
