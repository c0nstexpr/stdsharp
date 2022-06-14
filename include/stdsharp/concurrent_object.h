// Created by BlurringShadow at 2021-03-12-下午 1:56

#pragma once

#include <optional>
#include <shared_mutex>

#include "mutex/mutex.h"
#include "reflection/reflection.h"
#include "scope.h"

namespace stdsharp
{
    template<typename T, concepts::shared_lockable Lockable = ::std::shared_mutex>
        requires ::std::default_initializable<Lockable> && concepts::basic_lockable<Lockable>
    class concurrent_object
    {
    public:
        using value_type = ::std::optional<T>;

    private:
        using empty_t = type_traits::empty_t;

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
            requires concepts::copy_assignable<value_type>
        static void assign_value(
            value_type& object, //
            const concurrent_object<T, OtherLockable>& other //
        )
        {
            other.read([&object](const value_type& obj) { object = obj; });
        }

        template<typename OtherLockable>
            requires concepts::move_assignable<value_type>
        static void assign_value(value_type& object, concurrent_object<T, OtherLockable>&& other)
        {
            ::std::move(other).write([&object](value_type&& obj) { object = ::std::move(obj); });
        }

        template<typename Other>
        static constexpr bool other_assignable = requires(value_type obj)
        {
            assign_value(obj, ::std::declval<Other>());
        };

        static constexpr bool copy_assignable = other_assignable<const concurrent_object&>;
        static constexpr bool move_assignable = other_assignable<concurrent_object>;

        template<typename Other>
            requires(other_assignable<Other>)
        concurrent_object(const empty_t, Other&& other)
        {
            assign_value(object_, ::std::forward<Other>(other));
        }

        template<typename Other>
            requires(other_assignable<Other>)
        concurrent_object& assign_impl(Other&& other)
        {
            write([&other](value_type& obj) { assign_value(obj, ::std::forward<Other>(other)); });
            return *this;
        }

    public:
        using lock_type = Lockable;

        concurrent_object() = default;

        template<typename... TArg>
            requires ::std::constructible_from<T, TArg...>
        explicit concurrent_object(TArg&&... t_arg): object_(::std::forward<TArg>(t_arg)...) {}

        template<typename Other>
            requires(other_assignable<Other>)
        concurrent_object(Other&& other):
            concurrent_object(type_traits::empty, ::std::forward<Other>(other))
        {
        }

        concurrent_object(const concurrent_object& other) requires copy_assignable :
            concurrent_object(type_traits::empty, other)
        {
        }

        concurrent_object(concurrent_object&& other) noexcept(false) requires move_assignable :
            concurrent_object(type_traits::empty, ::std::move(other))
        {
        }

        template<typename Other>
            requires(other_assignable<Other>)
        concurrent_object& operator=(Other&& other)
        {
            assign_impl(::std::forward<Other>(other));
            return *this;
        }

        concurrent_object& operator=(const concurrent_object& other) requires copy_assignable
        {
            if(this != &other) assign_impl(other);
            return *this;
        }

        concurrent_object& operator=(concurrent_object&& other) //
            noexcept(false) requires move_assignable
        {
            if(this != &other) assign_impl(::std::move(other));
            return *this;
        }

        ~concurrent_object() = default;

        template<typename OtherLockable>
        void swap(concurrent_object<T, OtherLockable>& other) //
            requires ::std::swappable_with<
                value_type,
                typename ::std::remove_reference_t<decltype(other)>::value_type // clang-format off
            > // clang-format on
        {
            write(
                [&other](value_type& obj)
                {
                    other.write(
                        [&obj](auto& other_obj)
                        {
                            ::std::ranges::swap(obj, other_obj); //
                        } //
                    );
                } //
            );
        }

        template<::std::invocable<const value_type&> Func>
        void read(Func&& func) const&
        {
            ::std::shared_lock lock{lockable_};
            ::std::invoke(func, object_);
        }

        template<::std::invocable<const value_type> Func>
        void read(Func&& func) const&&
        {
            ::std::invoke(func, static_cast<const value_type&&>(object_));
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

        template<::std::invocable<value_type&> Func>
        void write(Func&& func) &
        {
            ::std::unique_lock lock{lockable_};
            ::std::invoke(func, object_);
        }

        template<::std::invocable<value_type> Func>
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
        ::std::optional<T> object_{};
        mutable Lockable lockable_{};
    };

    template<typename T, typename Lockable = ::std::shared_mutex>
    concurrent_object(T&&, Lockable&&) -> concurrent_object<T, Lockable>;
}
