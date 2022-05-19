#pragma once

#include <mutex>
#include <concepts>

using namespace ::std::literals;

namespace stdsharp::concepts
{
    template<typename T>
    concept basic_lockable = requires(T t)
    {
        t.lock();
        t.unlock();
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
    concept shared_lockable = requires(T t)
    {
        t.lock_shared();
        t.unlock_shared(); // clang-format off
        { t.try_lock_shared() } -> ::std::same_as<bool>; // clang-format on
    };

    template<typename T>
    concept shared_timed_lockable = shared_lockable<T> && requires(T t)
    { // clang-format off
        { t.try_lock_shared_for(1s) } -> ::std::same_as<bool>;
        { t.try_lock_shared_until(std::chrono::system_clock::now()) } -> ::std::same_as<bool>; // clang-format on
    };

    template<typename T>
    concept mutex =
        lockable<T> && !::std::movable<T> && ::std::default_initializable<T> && requires(T t)
    { // clang-format off
        { t.lock() } -> ::std::same_as<void>;
        { t.unlock() } -> ::std::same_as<void>; // clang-format on
    };

    template<typename T>
    concept timed_mutex = timed_lockable<T> && mutex<T>;

    template<typename T>
    concept shared_mutex = mutex<T> && shared_lockable<T> && requires(T t)
    {
        t.try_unlock_shared(); // clang-format off
        { t.lock_shared() } -> ::std::same_as<void>;
        { t.unlock_shared() } -> ::std::same_as<void>; // clang-format on
    };

    template<typename T>
    concept shared_timed_mutex = timed_mutex<T> && shared_mutex<T> && shared_timed_lockable<T>;
}