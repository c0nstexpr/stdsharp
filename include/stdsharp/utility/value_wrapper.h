#pragma once

#include <utility>

#include "../concepts/concepts.h"
#include "../compilation_config_in.h"

namespace stdsharp
{
    template<typename T>
    struct tuplewise_ctor
    {
    private:
        template<size_t... I>
        constexpr tuplewise_ctor(auto&& args, const std::index_sequence<I...>):
            T(std::get<I>(cpp_move(args))...)
        {
        }

    public:
        tuplewise_ctor() = default;

        template<typename... Args, typename Tuple = std::tuple<Args...>>
            requires std::is_constructible_v<T, Args...> &&
            (!std::is_constructible_v<T, const std::piecewise_construct_t&, Tuple>)
        constexpr tuplewise_ctor(const std::piecewise_construct_t, Tuple args) //
            noexcept(nothrow_constructible_from<T, Args...>):
            tuplewise_ctor(cpp_move(args), std::index_sequence_for<Args...>{})
        {
        }
    };

    namespace details
    {
        template<typename T>
        struct value_wrapper
        {
            T v;

            value_wrapper() = default;

            template<typename... U>
                requires std::constructible_from<T, U...>
            constexpr value_wrapper(U&&... u) noexcept(nothrow_constructible_from<T, U...>):
                v(cpp_forward(u)...)
            {
            }
        };

        template<empty_type T>
        struct value_wrapper<T>
        {
            STDSHARP_NO_UNIQUE_ADDRESS T v{};

            template<typename... U>
                requires std::constructible_from<T, U...>
            constexpr value_wrapper(U&&... u) noexcept(nothrow_constructible_from<T, U...>):
                v(cpp_forward(u)...)
            {
            }
        };
    }

    template<typename T>
    struct value_wrapper : details::value_wrapper<T>, tuplewise_ctor<value_wrapper<T>>
    {
        using value_type = T;
        using details::value_wrapper<T>::value_wrapper;
        using tuplewise_ctor<value_wrapper<T>>::tuplewise_ctor;
        using details::value_wrapper<T>::v;

#define STDSHARP_OPERATOR(const_, ref)                                                        \
    constexpr const_ T ref get() const_ ref noexcept { return static_cast<const_ T ref>(v); } \
                                                                                              \
    constexpr explicit operator const_ T ref() const_ ref noexcept { return get(); }

        STDSHARP_OPERATOR(, &)
        STDSHARP_OPERATOR(const, &)
        STDSHARP_OPERATOR(, &&)
        STDSHARP_OPERATOR(const, &&)

#undef STDSHARP_OPERATOR
    };
}

#include "../compilation_config_out.h"