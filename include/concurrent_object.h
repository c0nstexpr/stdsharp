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
        template<typename Other>
        friend void swap(concurrent_object& left, Other&& right) requires requires
        {
            left.swap(right);
        }
        {
            left.swap(right); //
        }

        template<typename OtherLockable>
        static void forward_object(const concurrent_object<T, OtherLockable>& other)
        {
            union
            {
                T obj;
            } u;
            scope([&other, &u] { u.obj = other.object_; }, ::std::shared_lock{other.lockable_});
            return ::std::move(u.obj);
        }

        template<typename OtherLockable>
        static void forward_object(concurrent_object<T, OtherLockable>&& other)
        {
            union
            {
                T obj;
            } u;
            scope(
                [&other, &u] { u.obj = ::std::move(other).object_; },
                ::std::unique_lock{other.lockable_} //
            );
            return ::std::move(u.obj);
        }

        template<typename Other>
        using forward_object_t = decltype(forward_object(::std::declval<Other>()));

    public:
        concurrent_object() = default;

        template<typename TArg, typename... LockableArg>
            requires ::std::constructible_from<T, TArg> &&
                ::std::constructible_from<Lockable, LockableArg...>
        explicit concurrent_object(TArg&& t_arg, LockableArg&&... lockable_arg) //
            noexcept(concepts::nothrow_constructible_from<T, TArg>&&
                         concepts::nothrow_constructible_from<Lockable, LockableArg...>):
            object_(::std::forward<TArg>(t_arg)),
            lockable_(::std::forward<LockableArg>(lockable_arg)...)
        {
        }

        template<typename Other, typename... LockableArg>
            requires requires
            {
                requires ::std::
                    constructible_from<concurrent_object, forward_object_t<Other>, LockableArg...>;
            } // NOLINTNEXTLINE(hicpp-explicit-conversions)
        concurrent_object(Other&& other, LockableArg&&... lockable_arg) noexcept(
            concepts::nothrow_constructible_from<
                concurrent_object,
                forward_object_t<Other>,
                LockableArg... // clang-format off
            >
        ):
            // clang-format on
            concurrent_object(
                forward_object(::std::forward<Other>(other)),
                ::std::forward<LockableArg>(lockable_arg)... //
            )
        {
        }

        concurrent_object(const concurrent_object& other) requires ::std::copy_constructible<T> &&
            ::std::default_initializable<Lockable> : object_(forward_object(other))
        {
        }

        concurrent_object(concurrent_object&& other) noexcept(false) //
            requires ::std::move_constructible<T> && ::std::default_initializable<Lockable> :
            object_(forward_object(::std::move(other)))
        {
        }

        template<typename Other>
            requires requires { requires ::std::assignable_from<T, forward_object_t<Other>>; }
        concurrent_object& operator=(Other&& other) //
            noexcept(concepts::nothrow_assignable_from<T, forward_object_t<Other>>)
        {
            object_ = forward_object(::std::forward<Other>(other));
            return *this;
        }

        concurrent_object& operator=(const concurrent_object& other) //
            requires concepts::copy_assignable<T>
        {
            if(this != &other) object_ = forward_object(other);
            return *this;
        }

        concurrent_object& operator=(concurrent_object&& other) noexcept(false) //
            requires concepts::move_assignable<T>
        {
            if(this != &other) object_ = forward_object(::std::move(other));
            return *this;
        }

        template<typename OtherLockable>
            requires ::std::swappable<T>
        void swap(concurrent_object<T, OtherLockable>& other)
        {
            ::std::ranges::swap(object_, forward_object(other));
        }

        ~concurrent_object() = default;

        template<::std::invocable<const T&> Func>
        void read(Func&& func) const&
        {
            scope(
                [&object = object_, &func] { ::std::invoke(func, object); },
                ::std::unique_lock{lockable_} //
            );
        }

        template<::std::invocable<const T&&> Func>
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
