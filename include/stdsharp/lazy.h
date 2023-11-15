#pragma once

#include <variant>
#include <memory>

#include "concepts/concepts.h"
#include "type_traits/core_traits.h"

namespace stdsharp
{
    template<std::invocable Fn>
        requires std::move_constructible<std::invoke_result_t<Fn>>
    class lazy_value
    {
    public:
        using value_type = std::invoke_result_t<Fn>;

    private:
        bool has_value_ = false;

        union
        {
            Fn fn_{};
            value_type value_;
        };

        constexpr void generate_value() //
            noexcept(nothrow_invocable<Fn> && nothrow_move_constructible<value_type>)
        {
            if(has_value()) return;

            auto&& v = invoke(cpp_move(fn_));
            std::ranges::destroy_at(&fn_);
            std::ranges::construct_at(&value_, cpp_move(v));
            has_value_ = true;
        }

        constexpr void generate_value() volatile noexcept(
            nothrow_invocable<volatile Fn> &&
            nothrow_constructible_from<value_type, std::invoke_result_t<volatile Fn>> //
        )
            requires std::invocable<volatile Fn, value_type>
        {
            if(has_value()) return;

            auto&& v = invoke(cpp_move(fn_));
            std::ranges::destroy_at(&fn_);
            std::ranges::construct_at(&value_, cpp_move(v));
            has_value_ = true;
        }

    public:
        lazy_value() = default;

        template<typename... Args>
            requires std::constructible_from<Fn, Args...>
        constexpr explicit(sizeof...(Args) == 1) lazy_value(Args&&... args) //
            noexcept(nothrow_constructible_from<Fn, Args...>):
            fn_(cpp_forward(args)...)
        {
        }

        [[nodiscard]] constexpr bool has_value() const noexcept { return has_value_; }

#define STDSHARP_LAZY_VALUE_GETTER(volatile_, ref)                                                 \
    constexpr decltype(auto) get(                                                                  \
    ) volatile_ ref noexcept(noexcept(static_cast<volatile_ lazy_value&>(*this).generate_value())) \
    {                                                                                              \
        static_cast<volatile_ lazy_value&>(*this).generate_value();                                \
        return static_cast<volatile_ value_type ref>(value_);                                      \
    }                                                                                              \
    constexpr decltype(auto) cget(                                                                 \
    ) volatile_ ref noexcept(noexcept(static_cast<volatile_ lazy_value&>(*this).generate_value())) \
    {                                                                                              \
        static_cast<volatile_ lazy_value&>(*this).generate_value();                                \
        return static_cast<const volatile_ value_type ref>(value_);                                \
    }

        STDSHARP_LAZY_VALUE_GETTER(, &)
        STDSHARP_LAZY_VALUE_GETTER(, &&)
        STDSHARP_LAZY_VALUE_GETTER(volatile, &)
        STDSHARP_LAZY_VALUE_GETTER(volatile, &&)

#undef STDSHARP_LAZY_VALUE_GETTER
    };

    template<typename Fn>
    lazy_value(Fn&&) -> lazy_value<std::decay_t<Fn>>;
}
