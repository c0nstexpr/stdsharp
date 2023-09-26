#pragma once

#include "../type_traits/core_traits.h"
#include "../concepts/concepts.h"

namespace stdsharp
{
    namespace details
    {
        template<expr_req ExprReq, typename Ret, typename... Args>
        using implement_dispatcher_func = Ret(Args...) noexcept(is_noexcept(ExprReq));
    }

    template<expr_req ExprReq, typename Ret, typename... Args>
    struct implement_dispatcher :
        std::reference_wrapper<details::implement_dispatcher_func<ExprReq, Ret, Args...>>
    {
    private:
        using func = ::meta::_t<implement_dispatcher>;
        using m_base = std::reference_wrapper<func>;

        template<invocable_r<Ret, Args...> Closure>
            requires requires //
        {
            requires cpp_is_constexpr(Closure{});
            requires(ExprReq != expr_req::no_exception) ||
                nothrow_invocable_r<Closure, Ret, Args...>;
        }
        static constexpr auto encapsulate() noexcept
        {
            return +[](Args... args) noexcept(is_noexcept(ExprReq)) -> Ret
            {
                constexpr Closure c{};
                if constexpr(std::same_as<void, Ret>) c(cpp_forward(args)...);
                else return c(cpp_forward(args)...);
            };
        }

        [[nodiscard]] static constexpr auto get_default() noexcept
            requires requires {
                requires std::default_initializable<Ret>;
                requires(ExprReq < expr_req::no_exception) || noexcept(Ret{});
            }
        {
            return [](const Args&...) noexcept(is_noexcept(ExprReq)) { return Ret{}; };
        }

        [[nodiscard]] static constexpr auto get_default() noexcept
            requires std::same_as<void, Ret>
        {
            return empty;
        }

    public:
        using m_base::m_base;

        static constexpr auto requirement = ExprReq;

        template<typename Closure>
            requires requires { encapsulate<Closure>(); }
        constexpr implement_dispatcher(const Closure) noexcept: m_base(*encapsulate<Closure>())
        {
        }

        constexpr implement_dispatcher() noexcept
            requires requires { get_default(); }
            : implement_dispatcher(get_default())
        {
        }

        constexpr implement_dispatcher& operator=(const func f) noexcept
        {
            this->get() = f;
            return *this;
        }

        template<typename Closure>
            requires requires { encapsulate<Closure>(); }
        constexpr implement_dispatcher& operator=(const Closure) noexcept
        {
            this->get() = *encapsulate<Closure>();
            return *this;
        }
    };

    template<typename Ret, typename... Arg>
    struct implement_dispatcher<expr_req::ill_formed, Ret, Arg...> : empty_t
    {
        static constexpr auto requirement = expr_req::ill_formed;
    };
}