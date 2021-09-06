// Created by BlurringShadow at 2021-03-12-下午 1:56

#pragma once

#include "type_traits.h"

#include <shared_mutex>

namespace stdsharp::utility
{
    template<typename T>
    class concurrent_object
    {
    public:
        template<typename... Args>
            requires ::std::constructible_from<T, Args...>
        constexpr explicit concurrent_object(Args&&... args) //
            noexcept(::stdsharp::utility::nothrow_constructible_from<T, Args...>):
            object_(::std::forward<Args>(args)...)
        {
        }

        constexpr auto& raw() noexcept { return object_; }

        constexpr auto& raw() const noexcept { return object_; }

        template<::std::invocable<const T&> Func>
        void read(Func&& func) const
        {
            ::std::shared_lock _(mutex_);
            func(object_);
        }

        template<std::invocable<T&> Func>
        void write(Func&& func)
        {
            std::unique_lock _(mutex_);
            func(object_);
        }

    private:
        T object_;

        mutable std::shared_mutex mutex_;
    };
}
