#pragma once

#include <shared_mutex>

#include "mutex/mutex.h"
#include "reflection/reflection.h"
#include "utility/constructor.h"

namespace stdsharp
{
    template<non_const T, shared_lockable Lockable = std::shared_mutex>
        requires basic_lockable<Lockable>
    class synchronizer
    {
    public:
        using value_type = T;

        using lock_type = Lockable;

        synchronizer() = default;

        template<typename... Args>
            requires requires {
                requires std::constructible_from<value_type, Args...>;
                requires std::default_initializable<lock_type>;
                requires !requires {
                    [](const std::piecewise_construct_t, auto&&...) {}(std::declval<Args>()...);
                };
            }
        explicit(sizeof...(Args) == 1) constexpr synchronizer(Args&&... args) noexcept(
            nothrow_constructible_from<value_type, Args...> &&
            nothrow_default_initializable<lock_type> //
        ):
            value_(cpp_forward(args)...)
        {
        }

        synchronizer(const synchronizer&) = delete;

        synchronizer(synchronizer&&) = delete;

        synchronizer& operator=(const synchronizer&) = delete;

        synchronizer& operator=(synchronizer&&) = delete;

        ~synchronizer() = default;

    private:
        template<typename ValueTuple, typename LockableTuple, std::size_t... I, std::size_t... J>
        constexpr synchronizer(
            ValueTuple&& value_tuple,
            LockableTuple&& lockable_tuple,
            std::index_sequence<I...> /*unused*/,
            std::index_sequence<J...> /*unused*/
        ):
            value_(cpo::get_element<I>(cpp_forward(value_tuple))...),
            lockable_(cpo::get_element<J>(cpp_forward(lockable_tuple))...)
        {
        }

        using default_get_shared_lock_fn = constructor<std::shared_lock<lock_type>>;
        using default_get_unique_lock_fn = constructor<std::unique_lock<lock_type>>;

    public:
        template<typename ValueTuple, typename LockableTuple>
            requires piecewise_constructible_from<value_type, ValueTuple> &&
            piecewise_constructible_from<lock_type, LockableTuple>
        constexpr synchronizer(
            const std::piecewise_construct_t /*unused*/,
            ValueTuple&& value_tuple,
            LockableTuple&& lockable_tuple
        ):
            synchronizer(
                cpp_forward(value_tuple),
                cpp_forward(lockable_tuple),
                std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<ValueTuple>>>{},
                std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<LockableTuple>>>{}
            )

        {
        }

        template<
            typename Self,
            typename Fn,
            std::invocable<lock_type> GetLock = default_get_shared_lock_fn,
            typename SelfT = const Self>
            requires std ::invocable<Fn&, forward_cast_t<SelfT, value_type>> &&
            lockable<std::invoke_result_t<GetLock>>
        constexpr void read(this const Self&& self, Fn func, GetLock get_lock = {})
        {
            auto&& this_ = forward_cast<SelfT, synchronizer>(self);
            const auto& lock = get_lock(cpp_forward(this_).lockable_);
            invoke(func, cpp_forward(this_).value_);
        }

        template<
            typename Self,
            typename Fn,
            std::invocable<lock_type> GetLock = default_get_unique_lock_fn>
            requires std::invocable<Fn&, forward_cast_t<Self, value_type>> &&
            lockable<std::invoke_result_t<GetLock>>
        constexpr void write(this Self&& self, Fn func, GetLock get_lock = {})
        {
            auto&& this_ = forward_cast<Self, synchronizer>(self);
            const auto& lock = get_lock(cpp_forward(this_).lockable_);
            invoke(func, cpp_forward(this_).value_);
        }

        constexpr const lock_type& lockable() const noexcept { return lockable_; }

    private:
        value_type value_{};
        mutable lock_type lockable_{};
    };

    template<typename T>
    synchronizer(T&&) -> synchronizer<std::decay_t<T>>;
}

namespace stdsharp::reflection
{
    template<typename T, typename U>
    inline constexpr indexed_values function<synchronizer<T, U>>{
        member_reflect<synchronizer<T, U>, literals::ltr{"read"}>(
            [](decay_same_as<synchronizer<T, U>> auto&& v, auto&&... args) //
            noexcept(noexcept(cpp_forward(v).read(cpp_forward(args)...))) //
            -> decltype(cpp_forward(v).read(cpp_forward(args)...))
            {
                return cpp_forward(v).read(cpp_forward(args)...); //
            }
        ),
        member_reflect<synchronizer<T, U>, literals::ltr{"write"}>(
            [](decay_same_as<synchronizer<T, U>> auto&& v, auto&&... args) //
            noexcept(noexcept(cpp_forward(v).write(cpp_forward(args)...))) //
            -> decltype(cpp_forward(v).write(cpp_forward(args)...))
            {
                return cpp_forward(v).write(cpp_forward(args)...); //
            }
        )
    };
}
