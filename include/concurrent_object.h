// Created by BlurringShadow at 2021-03-12-下午 1:56

#pragma once

#include <shared_mutex>

#include "concurrent_object.h"
#include "mutex/mutex.h"
#include "tuple/tuple.h"
#include "reflection/reflection.h"
#include "functional/operations.h"

namespace stdsharp
{
    template<typename T, shared_lockable Lockable = ::std::shared_mutex>
    class concurrent_object
    {
        template<typename TTuple, typename LockableTuple, ::std::size_t... I, ::std::size_t... J>
            requires ::std::constructible_from<T, get_t<I, TTuple>...> &&
                ::std::constructible_from<Lockable, get_t<J, LockableTuple>...>
        constexpr concurrent_object(
            TTuple&& t_tuple,
            LockableTuple&& lockable_tuple,
            const type_traits::regular_type_sequence<
                ::std::index_sequence<I...>,
                ::std::index_sequence<J...> // clang-format off
            >
        ) noexcept(
            concepts::nothrow_constructible_from<T, get_t<I, TTuple>...>&&
                concepts::nothrow_constructible_from<Lockable, get_t<J, LockableTuple>...> //
        ): object_(get<I>(t_tuple)...), mutex_(get<J>(lockable_tuple)...) // clang-format on
        {
        }

        template<typename TTuple, typename LockableTuple>
        using ctor_param_seq = type_traits::regular_type_sequence<
            ::std::make_index_sequence<::std::tuple_size_v<::std::decay_t<TTuple>>>,
            ::std::make_index_sequence<
                ::std::tuple_size_v<::std::decay_t<LockableTuple>> // clang-format off
            >
        >; // clang-format on

    public:
        template<typename... Args>
            requires ::std::constructible_from<T, Args...> && ::std::default_initializable<Lockable>
        constexpr explicit concurrent_object(Args&&... args) //
            noexcept(concepts::nothrow_constructible_from<T, Args...>):
            object_(::std::forward<Args>(args)...)
        {
        }

        template<typename TTuple, typename LockableTuple>
            requires ::std::constructible_from<
                concurrent_object,
                TTuple,
                LockableTuple,
                ctor_param_seq<TTuple, LockableTuple> // clang-format off
            > // clang-format on
        constexpr concurrent_object(
            const ::std::piecewise_construct_t,
            TTuple&& t_tuple,
            LockableTuple&& lockable_tuple // clang-format off
        ) noexcept( // clang-format on
            concepts::nothrow_constructible_from<
                concurrent_object,
                TTuple,
                LockableTuple,
                ctor_param_seq<TTuple, LockableTuple> // clang-format off
            >
        ): concurrent_object( // clang-format on
                ::std::forward<TTuple>(t_tuple),
                ::std::forward<LockableTuple>(lockable_tuple),
                ctor_param_seq<TTuple, LockableTuple>{} //
            )
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

        mutable Lockable mutex_;
    };
}
