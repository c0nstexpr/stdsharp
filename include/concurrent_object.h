// Created by BlurringShadow at 2021-03-12-下午 1:56

#pragma once

#include <shared_mutex>

#include "mutex/mutex.h"
#include "reflection/reflection.h"
#include "scope.h"

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
        void read(Func&& func) const&
        {
            scope(
                [&object = object_, &func] { ::std::invoke(func, object); },
                ::std::unique_lock{lockable_} //
            );
        }

        template<::std::invocable<const T&> Func>
        void read(Func&& func) const&&
        {
            scope(
                [&object = object_, &func] { ::std::invoke(func, static_cast<const T&&>(object)); },
                ::std::unique_lock{lockable_} //
            );
        }

        template<auto Name>
            requires(Name == "read"sv)
        constexpr auto operator()(const reflection::member_t<Name>) const& noexcept
        {
            return [this]<typename... Args>
                requires requires { this->read(::std::declval<Args>()...); }
            (Args && ... args) //
                noexcept(noexcept(this->read(::std::declval<Args>()...)))
            {
                return this->read(::std::forward<Args>(args)...);
            };
        }

        template<auto Name>
            requires(Name == "read"sv)
        constexpr auto operator()(const reflection::member_t<Name>) const&& noexcept
        {
            return [this]<typename... Args>
                requires requires { this->read(::std::declval<Args>()...); }
            (Args && ... args) //
                noexcept(noexcept(this->read(::std::declval<Args>()...)))
            {
                return static_cast<const concurrent_object&&>(*this).read(
                    ::std::forward<Args>(args)... //
                );
            };
        }

        template<::std::invocable<T&> Func>
        void write(Func&& func) &
        {
            scope(
                [&object = object_, &func] { ::std::invoke(func, object); },
                ::std::unique_lock{lockable_} //
            );
        }

        template<::std::invocable<T> Func>
        void write(Func&& func) &&
        {
            scope(
                [&object = object_, &func] { ::std::invoke(func, ::std::move(object)); },
                ::std::unique_lock{lockable_} //
            );
        }

        template<auto Name>
            requires(Name == "write"sv)
        constexpr auto operator()(const reflection::member_t<Name>) & noexcept
        {
            return [this]<typename... Args>
                requires requires { this->write(::std::declval<Args>()...); }
            (Args && ... args) //
                noexcept(noexcept(this->write(::std::declval<Args>()...)))
            {
                return this->write(::std::forward<Args>(args)...);
            };
        }

        template<auto Name>
            requires(Name == "write"sv)
        constexpr auto operator()(const reflection::member_t<Name>) && noexcept
        {
            return [this]<typename... Args>
                requires requires { ::std::move(*this).write(::std::declval<Args>()...); }
            (Args && ... args) //
                noexcept(noexcept(::std::move(*this).write(::std::declval<Args>()...)))
            {
                return ::std::move(*this).write(::std::forward<Args>(args)...);
            };
        }

    private:
        T object_;

        mutable Lockable lockable_;
    };

    template<typename T, typename Lockable = ::std::shared_mutex>
    concurrent_object(T&&, Lockable&&) -> concurrent_object<T, Lockable>;
}
