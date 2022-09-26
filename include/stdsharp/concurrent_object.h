// Created by BlurringShadow at 2021-03-12-下午 1:56

#pragma once

#include <optional>
#include <shared_mutex>

#include "mutex/mutex.h"
#include "reflection/reflection.h"
#include "scope.h"
#include "functional/bind.h"

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
            requires requires(concurrent_object& left) //
        {
            ::std::declval<concurrent_object&>().swap(::std::declval<Other>()); //
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
        static constexpr bool other_assignable =
            requires(value_type obj) { assign_value(obj, ::std::declval<Other>()); };

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

        struct read_fn
        {
            template<typename This, typename Func>
            constexpr void operator()(This&& instance, Func&& func) const
            {
                const ::std::shared_lock lock{instance.lockable_};
                ::std::invoke(::std::forward<Func>(func), ::std::forward<This>(instance).object_);
            }
        };

        struct write_fn
        {
            template<typename Func>
            constexpr void operator()(concurrent_object&& instance, Func&& func) const
            {
                ::std::invoke(::std::forward<Func>(func), ::std::move(instance).object_);
            }

            template<typename Func>
            constexpr void operator()(concurrent_object& instance, Func&& func) const
            {
                const ::std::unique_lock lock{instance.lockable_};
                ::std::invoke(::std::forward<Func>(func), instance.object_);
            }
        };

    public:
        using lock_type = Lockable;

        concurrent_object() = default;

        template<typename... TArg>
            requires ::std::constructible_from<T, TArg...>
        explicit concurrent_object(TArg&&... t_arg): object_(::std::forward<TArg>(t_arg)...)
        {
        }

        template<typename Other>
            requires(other_assignable<Other>)
        concurrent_object(Other&& other):
            concurrent_object(type_traits::empty, ::std::forward<Other>(other))
        {
        }

        concurrent_object(const concurrent_object& other)
            requires copy_assignable
            : concurrent_object(type_traits::empty, other)
        {
        }

        concurrent_object(concurrent_object&& other) noexcept(false)
            requires move_assignable
            : concurrent_object(type_traits::empty, ::std::move(other))
        {
        }

        template<typename Other>
            requires(other_assignable<Other>)
        concurrent_object& operator=(Other&& other)
        {
            assign_impl(::std::forward<Other>(other));
            return *this;
        }

        concurrent_object& operator=(const concurrent_object& other)
            requires copy_assignable
        {
            if(this != &other) assign_impl(other);
            return *this;
        }

        concurrent_object& operator=(concurrent_object&& other) noexcept(false)
            requires move_assignable
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
                [&other](value_type& obj) //
                {
                    other.write([&obj](auto& other_obj) { ::std::ranges::swap(obj, other_obj); });
                } //
            );
        }

        template<::std::invocable<const value_type&> Func>
        void read(Func&& func) const&
        {
            read_fn{}(*this, ::std::forward<Func>(func));
        }

        template<::std::invocable<const value_type> Func>
        void read(Func&& func) const&&
        {
            read_fn{}(static_cast<const concurrent_object&&>(*this), ::std::forward<Func>(func));
        }

        template<::std::invocable<value_type&> Func>
        void write(Func&& func) &
        {
            write_fn{}(*this, ::std::forward<Func>(func));
        }

        template<::std::invocable<value_type> Func>
        void write(Func&& func) &&
        {
            write_fn{}(::std::move(*this), ::std::forward<Func>(func));
        }

        constexpr const auto& lockable() const noexcept { return lockable_; }

    private:
        ::std::optional<T> object_{};
        mutable Lockable lockable_{};

        template<auto Name, concepts::decay_same_as<concurrent_object> This>
            requires requires { requires Name == "write"sv; }
        [[nodiscard]] friend constexpr auto get_member(This&& instance) noexcept
        {
            return functional::bind(write_fn{}, ::std::forward<This>(instance));
        }

        template<auto Name, concepts::decay_same_as<concurrent_object> This>
            requires requires { requires Name == "read"sv; }
        [[nodiscard]] friend constexpr auto get_member(This&& instance) noexcept
        {
            return functional::bind(read_fn{}, ::std::forward<This>(instance));
        }
    };

    namespace reflection
    {
        template<typename T, typename U>
        struct get_members_t<concurrent_object<T, U>>
        {
            [[nodiscard]] constexpr auto operator()() const noexcept
            {
                return ::std::array{
                    member_info{"read", member_category::function},
                    member_info{"write", member_category::function} //
                };
            }
        };
    }

    template<typename T>
    concurrent_object(T&&) -> concurrent_object<::std::decay_t<T>>;
}
