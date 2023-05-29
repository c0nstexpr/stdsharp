#pragma once

#include <optional>
#include <shared_mutex>

#include "mutex/mutex.h"
#include "reflection/reflection.h"
#include "type_traits/indexed_traits.h"
#include "utility/value_wrapper.h"

namespace stdsharp
{
    template<typename T, shared_lockable Lockable = std::shared_mutex>
        requires basic_lockable<Lockable>
    class synchronizer
    {
    public:
        using value_type = std::optional<T>;

    private:
        template<typename OtherLockable>
            requires copy_assignable<value_type>
        static constexpr void assign_value(
            value_type& object, //
            const synchronizer<T, OtherLockable>& other //
        )
        {
            other.read([&object](const value_type& obj) { object = obj; });
        }

        template<typename OtherLockable>
            requires move_assignable<value_type>
        static constexpr void
            assign_value(value_type& object, synchronizer<T, OtherLockable>&& other)
        {
            cpp_move(other).write([&object](value_type&& obj) { object = cpp_move(obj); });
        }

        template<typename Other>
        static constexpr bool other_assignable =
            requires(value_type obj) { assign_value(obj, std::declval<Other>()); };

        static constexpr bool copy_assignable = other_assignable<const synchronizer&>;
        static constexpr bool move_assignable = other_assignable<synchronizer>;

        template<typename Other>
            requires(other_assignable<Other>)
        constexpr synchronizer(const empty_t, Other&& other)
        {
            assign_value(object_, cpp_forward(other));
        }

        template<typename Other>
            requires(other_assignable<Other>)
        constexpr synchronizer& assign_impl(Other&& other)
        {
            write([&other](value_type& obj) { assign_value(obj, cpp_forward(other)); });
            return *this;
        }

        struct read_fn
        {
            template<typename This, typename Func>
            constexpr void operator()(This&& instance, Func&& func) const
            {
                const std::shared_lock lock{instance.lockable()};
                std::invoke(cpp_forward(func), cpp_forward(instance).object_);
            }
        };

        struct write_fn
        {
            template<typename Func>
            constexpr void operator()(synchronizer&& instance, Func&& func) const
            {
                std::invoke(cpp_forward(func), cpp_move(instance).object_);
            }

            template<typename Func>
            constexpr void operator()(synchronizer& instance, Func&& func) const
            {
                const std::unique_lock lock{instance.lockable()};
                std::invoke(cpp_forward(func), instance.object_);
            }
        };

    public:
        using lock_type = Lockable;

        synchronizer() = default;

        template<typename... Args>
            requires std::constructible_from<T, Args...> && std::default_initializable<Lockable>
        explicit constexpr synchronizer(Args&&... t_arg): object_(cpp_forward(t_arg)...)
        {
        }

        template<typename... TArgs, typename... LockArgs>
            requires std::constructible_from<T, TArgs...> &&
                         std::constructible_from<Lockable, LockArgs...>
        explicit constexpr synchronizer(
            const std::piecewise_construct_t tag,
            std::tuple<TArgs...> t_arg,
            std::tuple<LockArgs...> lock_arg
        ):
            object_(cpp_forward(t_arg)), lockable_(tag, cpp_move(lock_arg))
        {
        }

        template<typename Other>
            requires(other_assignable<Other>)
        constexpr synchronizer(Other&& other): synchronizer(empty, cpp_forward(other))
        {
        }

        constexpr synchronizer(const synchronizer& other)
            requires copy_assignable
            : synchronizer(empty, other)
        {
        }

        constexpr synchronizer(synchronizer&& other) noexcept(false)
            requires move_assignable
            : synchronizer(empty, cpp_move(other))
        {
        }

        template<typename Other>
            requires(other_assignable<Other>)
        constexpr synchronizer& operator=(Other&& other)
        {
            assign_impl(cpp_forward(other));
            return *this;
        }

        constexpr synchronizer& operator=(const synchronizer& other)
            requires copy_assignable
        {
            if(this != &other) assign_impl(other);
            return *this;
        }

        constexpr synchronizer& operator=(synchronizer&& other) noexcept(false)
            requires move_assignable
        {
            if(this != &other) assign_impl(cpp_move(other));
            return *this;
        }

        ~synchronizer() = default;

        template<typename OtherLockable>
        constexpr void swap(synchronizer<T, OtherLockable>& other) //
            requires std::swappable_with<
                value_type,
                typename std::remove_reference_t<decltype(other)>::value_type // clang-format off
            > // clang-format on
        {
            write(
                [&other](value_type& obj) //
                { other.write([&obj](auto& other_obj) { std::ranges::swap(obj, other_obj); }); } //
            );
        }

        template<std::invocable<const value_type&> Func>
        constexpr void read(Func&& func) const&
        {
            read_fn{}(*this, cpp_forward(func));
        }

        template<std::invocable<const value_type> Func>
        constexpr void read(Func&& func) const&&
        {
            read_fn{}(static_cast<const synchronizer&&>(*this), cpp_forward(func));
        }

        template<std::invocable<value_type&> Func>
        constexpr void write(Func&& func) &
        {
            write_fn{}(*this, cpp_forward(func));
        }

        template<std::invocable<value_type> Func>
        constexpr void write(Func&& func) &&
        {
            write_fn{}(cpp_move(*this), cpp_forward(func));
        }

        constexpr const auto& lockable() const noexcept { return lockable_; }

    private:
        std::optional<T> object_{};
        mutable Lockable lockable_{};

        constexpr auto& lockable() noexcept { return lockable_; }
    };

    template<typename T>
    synchronizer(T&&) -> synchronizer<std::decay_t<T>>;

    namespace reflection
    {
        template<typename T, typename U>
        inline constexpr auto function<synchronizer<T, U>> =
            member<synchronizer<T, U>>::template func_reflect<"read"_ltr, "write"_ltr>(
                [](auto&& v, auto&&... args) //
                noexcept( //
                    noexcept(cast_fwd<synchronizer<T, U>>(cpp_forward(v)).read(cpp_forward(args)...)
                    )
                ) -> //
                decltype( //
                    cast_fwd<synchronizer<T, U>>(cpp_forward(v)).read(cpp_forward(args)...)
                ) //
                { return cast_fwd<synchronizer<T, U>>(cpp_forward(v)).read(cpp_forward(args)...); },
                [](auto&& v, auto&&... args) //
                noexcept( //
                    noexcept( //
                        cast_fwd<synchronizer<T, U>>(cpp_forward(v)).write(cpp_forward(args)...)
                    )
                ) -> //
                decltype( //
                    cast_fwd<synchronizer<T, U>>(cpp_forward(v)).write(cpp_forward(args)...)
                ) //
                { return cast_fwd<synchronizer<T, U>>(cpp_forward(v)).write(cpp_forward(args)...); }
            );
    }
}