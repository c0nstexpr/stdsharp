#pragma once

#include "../type_traits/core_traits.h"
#include "../concepts/concepts.h"

namespace stdsharp
{
    namespace details
    {
        template<expr_req ExprReq, typename Ret, typename... Args>
        using implement_dispatcher_func = Ret(Args...) noexcept(ExprReq == expr_req::no_exception);
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
            return +[](Args... args) noexcept(ExprReq == expr_req::no_exception) -> Ret
            {
                constexpr Closure c{};
                return c(cpp_forward(args)...);
            };
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
            requires std::same_as<void, Ret>
            : implement_dispatcher([](const Args&...) noexcept {})
        {
        }

        constexpr implement_dispatcher() noexcept
            requires requires //
        {
            requires std::default_initializable<Ret>;
            requires(ExprReq != expr_req::no_exception) || noexcept(Ret{});
        }
            :
            implement_dispatcher( //
                [](Args...) noexcept(ExprReq == expr_req::no_exception) { return Ret{}; }
            )
        {
        }

        constexpr implement_dispatcher& operator=(func f) noexcept
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