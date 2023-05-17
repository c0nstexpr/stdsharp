#pragma once

namespace stdsharp::details
{
    template<typename T>
    consteval T remove_ref(T&&);

    template<typename T>
    consteval T remove_ref(T&);

    template<bool>
    using var_template = void;
}

#define cpp_forward(...) static_cast<decltype(__VA_ARGS__)>(__VA_ARGS__)
#define cpp_move(...) \
    static_cast<decltype(stdsharp::details::remove_ref(__VA_ARGS__))&&>(__VA_ARGS__)
#define cpp_is_constexpr(...) requires { typename details::var_template<(__VA_ARGS__, true)>; }

#define stdsharp_func_member_reflect(name, ...) \
    ::stdsharp::reflection::member<                                                                              \
        __VA_ARGS__,                                                                     \
        decltype(\
            [](decay_same_as<__VA_ARGS__> auto&& v, auto&&... args) \
            noexcept(noexcept(cpp_forward(v).first(cpp_forward(args)...)))\
             -> decltype(cpp_forward(v).first(cpp_forward(args)...))                        \
            { return cpp_forward(v).first(cpp_forward(args)...); }\
        ), #name##_ltr, ::stdsharp::reflection::mem_type::function>
