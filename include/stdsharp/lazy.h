#pragma once

#include <variant>
#include <memory>

#include "concepts/concepts.h"
#include "type_traits/core_traits.h"

namespace stdsharp
{
    template<std::invocable Fn>
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

        static constexpr void generate_value(auto&& this_) //
            noexcept(nothrow_invocable<Fn> && nothrow_constructible_from<value_type, std::invoke_result_t<Fn>>)
        {
            if(this_.has_value()) return;

            auto&& v = invoke(cpp_move(this_.fn_));
            std::ranges::destroy_at(&this_.fn_);
            std::ranges::construct_at(&this_.value_, cpp_move(v));
            this_.has_value_ = true;
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

#define STDSHARP_LAZY_VALUE_GETTER(ref)                                            \
    constexpr decltype(auto) get()                                                 \
        ref noexcept(noexcept(generate_value(static_cast<lazy_value ref>(*this)))) \
        requires requires { generate_value(static_cast<lazy_value ref>(*this)); }  \
    {                                                                              \
        generate_value(static_cast<lazy_value ref>(*this));                        \
        return static_cast<value_type ref>(value_);                                \
    }                                                                              \
    constexpr decltype(auto) cget()                                                \
        ref noexcept(noexcept(static_cast<lazy_value ref>(*this).get()))           \
        requires requires { static_cast<lazy_value ref>(*this).get(); }            \
    {                                                                              \
        return std::as_const(static_cast<lazy_value ref>(*this).get());            \
    }

        STDSHARP_LAZY_VALUE_GETTER(&)
        STDSHARP_LAZY_VALUE_GETTER(&&)

#undef STDSHARP_LAZY_VALUE_GETTER
    };

    template<typename Fn>
    lazy_value(Fn&&) -> lazy_value<std::decay_t<Fn>>;
}
