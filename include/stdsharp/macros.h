#pragma once

namespace stdsharp::details
{
    template<typename T>
    constexpr T remove_ref(T&&);

    template<typename T>
    constexpr T remove_ref(T&);
}

#define cpp_forward(v) static_cast<decltype(v)>(v)
#define cpp_move(v) static_cast<decltype(stdsharp::details::remove_ref(v))&&>(v)