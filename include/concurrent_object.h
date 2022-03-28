// Created by BlurringShadow at 2021-03-12-下午 1:56

#pragma once

#include <shared_mutex>

#include "mutex/mutex.h"
#include "reflection/reflection.h"

namespace stdsharp
{
    template<typename T, shared_lockable Lockable = ::std::shared_mutex>
        requires basic_lockable<Lockable>
    class concurrent_object
    {
    public:
        concurrent_object() = default;

        template<typename TArg, typename... LockableArg>
            requires ::std::constructible_from<T, TArg> &&
                ::std::constructible_from<Lockable, LockableArg...>
        constexpr explicit concurrent_object(TArg&& t_arg, LockableArg&&... lockable_arg) //
            noexcept(concepts::nothrow_constructible_from<T, TArg>&&
                         concepts::nothrow_constructible_from<Lockable, LockableArg...>):
            object_(::std::forward<TArg>(t_arg)),
            lockable_(::std::forward<LockableArg>(lockable_arg)...)
        {
        }

        template<::std::invocable<const T&> Func>
        void read(Func&& func) const
        {
            ::std::shared_lock _(lockable_);
            ::std::invoke(func, object_);
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
            ::std::unique_lock _{lockable_};
            ::std::invoke(func, object_);
        }

        template<auto Name>
            requires(Name == "write"sv)
        constexpr auto operator()(const reflection::member_t<Name>) noexcept
        {
            return [this]() { return this->write(); };
        }

    private:
        T object_;

        mutable Lockable lockable_;
    };

    template<typename T, typename Lockable = ::std::shared_mutex>
    concurrent_object(T&&, Lockable&&) -> concurrent_object<T, Lockable>;
}
