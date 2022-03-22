#pragma once

#include <mutex>
#include <concepts>

namespace stdsharp
{
    using namespace ::std::literals;

    namespace details
    {

    }

    template<typename T>
    concept basic_lockable = requires(T t)
    {
        t.lock();
        requires noexcept(t.unlock());
    };

    template<typename T>
    concept lockable = basic_lockable<T> && requires(T t)
    { // clang-format off
        { t.try_lock() } -> ::std::same_as<bool>; // clang-format on
    };

    template<typename T>
    concept timed_lockable = lockable<T> && requires(T t)
    { // clang-format off
        { t.try_lock_for(1s) } -> ::std::same_as<bool>;
        { t.try_lock_until(std::chrono::system_clock::now()) } -> ::std::same_as<bool>; // clang-format on
    };

    template<typename T>
    concept mutex =
        lockable<T> && !::std::movable<T> && ::std::default_initializable<T> && requires(T t)
    { // clang-format off
        { t.lock() } -> ::std::same_as<void>;
        { t.unlock() } -> ::std::same_as<void>; // clang-format on
    };
}