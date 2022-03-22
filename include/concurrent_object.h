// Created by BlurringShadow at 2021-03-12-下午 1:56

#pragma once

#include <shared_mutex>

#include "reflection/reflection.h"
#include "functional/operations.h"

namespace stdsharp
{
    template<typename T>
    class concurrent_object
    {
    public:
        template<typename... Args>
            requires ::std::constructible_from<T, Args...>
        constexpr explicit concurrent_object(Args&&... args) //
            noexcept(concepts::nothrow_constructible_from<T, Args...>):
            object_(::std::forward<Args>(args)...)
        {
        }

        constexpr auto& raw() noexcept { return object_; }

        template<auto Name>
            requires(Name == "raw"sv)
        constexpr auto operator()(const reflection::member_t<Name>) noexcept
        {
            return [this]() { return this->raw(); };
        }

        constexpr auto& raw() const noexcept { return object_; }

        template<auto Name>
            requires(Name == "raw"sv)
        constexpr auto operator()(const reflection::member_t<Name>) const noexcept
        {
            return [this]() { return this->raw(); };
        }

        template<::std::invocable<const T&> Func>
        void read(Func&& func) const
        {
            ::std::shared_lock _(mutex_);
            func(object_);
        }

        template<auto Name>
            requires(Name == "read"sv)
        constexpr auto operator()(const reflection::member_t<Name>) const noexcept
        {
            return [this]() { return this->read(); };
        }

        template<::std::invocable<T&> Func>
        void write(Func&& func)
        {
            ::std::unique_lock _{mutex_};
            func(object_);
        }

        template<auto Name>
            requires(Name == "write"sv)
        constexpr auto operator()(const reflection::member_t<Name>) noexcept
        {
            return [this]() { return this->write(); };
        }

    private:
        T object_;

        mutable ::std::shared_mutex mutex_;
    };
}
