#pragma once

#include <shared_mutex>

#include "mutex/mutex.h"
#include "reflection/reflection.h"

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
        explicit(sizeof...(Args) == 1) constexpr synchronizer(Args&&... args) //
            noexcept(nothrow_constructible_from<value_type, Args...> && nothrow_default_initializable<lock_type>):
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

#define STDSHARP_SYN_MEM(volatile_, ref)                                                       \
    template<typename... Args>                                                                 \
    constexpr void read(                                                                       \
        std::invocable<const volatile_ value_type ref> auto&& func,                            \
        Args&&... args                                                                         \
    ) const volatile_ ref                                                                      \
        requires std::constructible_from<std::shared_lock<lock_type>, lock_type&, Args...>     \
    {                                                                                          \
        const std::shared_lock lock{lockable_, cpp_forward(args)...};                          \
        invoke(cpp_forward(func), cpp_forward(*this).value_);                                  \
    }                                                                                          \
                                                                                               \
    template<typename... Args>                                                                 \
    constexpr void write(std::invocable<volatile_ value_type ref> auto&& func, Args&&... args) \
        volatile_ ref                                                                          \
        requires std::constructible_from<std::unique_lock<lock_type>, lock_type&, Args...>     \
    {                                                                                          \
        const std::unique_lock lock{lockable_, cpp_forward(args)...};                          \
        invoke(cpp_forward(func), cpp_forward(*this).value_);                                  \
    }

        STDSHARP_SYN_MEM(, &)
        STDSHARP_SYN_MEM(, &&)
        STDSHARP_SYN_MEM(volatile, &)
        STDSHARP_SYN_MEM(volatile, &&)

#undef STDSHARP_SYN_MEM

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
