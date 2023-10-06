#pragma once

#include <gsl/pointers>

#include "../type_traits/core_traits.h"
#include "../concepts/concepts.h"

namespace stdsharp
{
    namespace details
    {
        template<expr_req ExprReq, typename Ret, typename... Args>
        struct dispatcher_traits
        {
            using func = Ret (*)(Args...) noexcept(is_noexcept(ExprReq));
            using not_null = gsl::not_null<func>;

            class dispatcher : public not_null
            {
                template<invocable_r<Ret, Args...> Closure>
                    requires requires //
                {
                    requires cpp_is_constexpr(Closure{});
                    requires(ExprReq != expr_req::no_exception) ||
                        nothrow_invocable_r<Closure, Ret, Args...>;
                }
                static constexpr auto encapsulate() noexcept
                {
                    if constexpr(explicitly_convertible<Closure, func>)
                        return static_cast<func>(Closure{});
                    else
                        return +[](Args... args) noexcept(is_noexcept(ExprReq)) -> Ret
                        {
                            constexpr Closure c{};
                            if constexpr(std::same_as<void, Ret>) c(cpp_forward(args)...);
                            else return c(cpp_forward(args)...);
                        };
                }

                [[nodiscard]] static constexpr auto get_default() noexcept
                    requires(is_noexcept(ExprReq) ? noexcept(Ret{}) : requires { Ret{}; })
                {
                    return [](const Args&...) noexcept(is_noexcept(ExprReq)) { return Ret{}; };
                }

                [[nodiscard]] static constexpr auto get_default() noexcept
                    requires std::same_as<void, Ret>
                {
                    return [](const Args&...) noexcept {};
                }

            public:
                using not_null::not_null;
                using not_null::operator func;
                using not_null::operator=;

                static constexpr auto requirement = ExprReq;

                template<typename Closure>
                    requires requires { encapsulate<Closure>(); }
                constexpr dispatcher(const Closure) noexcept: not_null(encapsulate<Closure>())
                {
                }

                constexpr dispatcher() noexcept
                    requires requires { get_default(); }
                    : dispatcher(get_default())
                {
                }

                template<typename Closure>
                    requires requires { encapsulate<Closure>(); }
                constexpr dispatcher& operator=(const Closure) noexcept
                {
                    return (*this = encapsulate<Closure>());
                }

                constexpr bool operator==(const dispatcher other) const noexcept
                {
                    return this->get() == other.get();
                }
            };
        };

        template<typename Ret, typename... Args>
        struct dispatcher_traits<expr_req::ill_formed, Ret, Args...>
        {
            struct dispatcher : empty_t
            {
                static constexpr auto requirement = expr_req::ill_formed;

                constexpr operator nullptr_t() const noexcept { return nullptr; }
            };
        };
    }

    template<expr_req ExprReq, typename Ret, typename... Args>
    using dispatcher = typename details::dispatcher_traits<ExprReq, Ret, Args...>::dispatcher;
}