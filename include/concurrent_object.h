// Created by BlurringShadow at 2021-03-12-下午 1:56

#pragma once

#include <optional>
#include <shared_mutex>

#include "mutex/mutex.h"
#include "reflection/reflection.h"
#include "scope.h"
#include "type_traits/core_traits.h"

namespace stdsharp
{
    template<typename T, shared_lockable Lockable = ::std::shared_mutex>
        requires basic_lockable<Lockable>
    class concurrent_object
    {
        using optional_t = ::std::optional<T>;

        template<typename Other>
            requires requires(concurrent_object& left)
            {
                ::std::declval<concurrent_object&>().swap(::std::declval<Other>());
            }
        friend void swap(concurrent_object& left, Other&& right)
        {
            left.swap(::std::forward<Other>(right)); //
        }

        template<typename OtherLockable>
            requires concepts::copy_assignable<optional_t>
        static void assign_value(
            optional_t& object, //
            const concurrent_object<T, OtherLockable>& other //
        )
        {
            other.read([&object](optional_t& obj) { object = obj; });
        }

        template<typename OtherLockable>
            requires concepts::move_assignable<optional_t>
        static void assign_value(
            optional_t& object, //
            concurrent_object<T, OtherLockable>&& other //
        )
        {
            ::std::move(other).write([&object](optional_t&& obj) { object = ::std::move(obj); });
        }

        template<typename Other>
        static constexpr auto other_assignable = requires(optional_t obj)
        {
            assign_value(obj, ::std::declval<Other>());
        };

        template<typename Other, typename... LockableArg>
            requires requires
            {
                ::std::constructible_from<concurrent_object, LockableArg...>&&
                    other_assignable<Other>;
            } // NOLINTNEXTLINE(hicpp-explicit-conversions)
        concurrent_object(const type_traits::empty_t, Other&& other, LockableArg&&... lockable_arg):
            concurrent_object(::std::forward<LockableArg>(lockable_arg)...)
        {
            assign_value(object_, ::std::forward<Other>(other));
        }

        template<typename Other>
            requires(other_assignable<Other>)
        concurrent_object& assign_impl(Other&& other)
        {
            write([&other](optional_t& obj) { assign_value(obj, ::std::forward<Other>(other)); });
            return *this;
        }

    public:
        using value_type = T;
        using lock_type = Lockable;

        concurrent_object() = default;

        template<typename... LockableArg>
            requires ::std::constructible_from<Lockable, LockableArg...>
        explicit concurrent_object(LockableArg&&... lockable_arg) //
            noexcept(concepts::nothrow_constructible_from<Lockable, LockableArg...>):
            lockable_(::std::forward<LockableArg>(lockable_arg)...)
        {
        }

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
                requires ::std::constructible_from<
                    concurrent_object,
                    type_traits::empty_t,
                    Other,
                    LockableArg... // clang-format off
                >; // clang-format on
            } // NOLINTNEXTLINE(hicpp-explicit-conversions)
        concurrent_object(Other&& other, LockableArg&&... lockable_arg):
            concurrent_object(
                type_traits::empty,
                ::std::forward<Other>(other),
                ::std::forward<LockableArg>(lockable_arg)... //
            )
        {
        }

        concurrent_object(const concurrent_object& other) //
            requires ::std::constructible_from<concurrent_object, const concurrent_object&> :
            concurrent_object(other)
        {
        }

        concurrent_object(concurrent_object&& other) noexcept(false) //
            requires ::std::constructible_from<concurrent_object, concurrent_object> :
            concurrent_object(::std::move(other))
        {
        }

        template<typename Other>
            requires requires
            {
                ::std::declval<concurrent_object>().assign_impl(::std::declval<Other>());
            }
        concurrent_object& operator=(Other&& other) //
        {
            if(this != ::std::addressof(other)) assign_impl(::std::forward<Other>(other));
            return *this;
        }

        concurrent_object& operator=(const concurrent_object& other) requires requires
        {
            ::std::declval<concurrent_object>().assign_impl(other);
        }
        {
            if(this != &other) assign_impl(other);
            return *this;
        }

        concurrent_object& operator=(concurrent_object&& other) noexcept(false) requires requires
        {
            ::std::declval<concurrent_object>().assign_impl(::std::move(other));
        }
        {
            if(this != &other) assign_impl(::std::move(other));
            return *this;
        }

        ~concurrent_object() = default;

        template<typename OtherLockable>
            requires ::std::swappable<optional_t>
        void swap(concurrent_object<T, OtherLockable>& other)
        {
            ::std::ranges::swap(object_, forward_object(other));
        }

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
            ::std::invoke(func, static_cast<const T&&>(object_));
        }

        template<auto Name>
            requires(Name == "read"sv)
        constexpr auto operator()(const reflection::member_t<Name>) const& noexcept
        {
            return [this]<typename... Args>
                requires requires { this->read(::std::declval<Args>()...); }
            (Args && ... args) //
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
            {
                return static_cast<const concurrent_object&&>(*this) //
                    .read(::std::forward<Args>(args)...);
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
            ::std::invoke(func, ::std::move(object_));
        }

        template<auto Name>
            requires(Name == "write"sv)
        constexpr auto operator()(const reflection::member_t<Name>) & noexcept
        {
            return [this]<typename... Args>
                requires requires { this->write(::std::declval<Args>()...); }
            (Args && ... args) //
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
            {
                return ::std::move(*this).write(::std::forward<Args>(args)...);
            };
        }

        constexpr const auto& lockable() const noexcept { return lockable_; }

    private:
        ::std::optional<T> object_;

        mutable Lockable lockable_;
    };

    template<typename T, typename Lockable = ::std::shared_mutex>
    concurrent_object(T&&, Lockable&&) -> concurrent_object<T, Lockable>;
}
