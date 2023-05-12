#pragma once

namespace stdsharp::details
{
    template<typename T>
    T remove_ref(T&&);

    template<typename T>
    T remove_ref(T&);

    template<bool>
    using var_template = void;
}

#define cpp_forward(v) static_cast<decltype(v)>(v)
#define cpp_move(v) static_cast<decltype(stdsharp::details::remove_ref(v))&&>(v)
#define cpp_is_constexpr(v) requires { typename var_template<(v, true)>; }