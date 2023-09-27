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
            using not_null =
                std::conditional_t<ExprReq == expr_req::ill_formed, empty_t, gsl::not_null<func>>;

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
                using not_null::not_null;
                using not_null::operator func;

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

                constexpr dispatcher& operator=(const func f) noexcept
                {
                    this->get() = f;
                    return *this;
                }

                template<typename Closure>
                    requires requires { encapsulate<Closure>(); }
                constexpr dispatcher& operator=(const Closure) noexcept
                {
                    this->get() = *encapsulate<Closure>();
                    return *this;
                }
            };
        };
    }

    template<expr_req ExprReq, typename Ret, typename... Args>
    using dispatcher = typename details::dispatcher_traits<ExprReq, Ret, Args...>::dispatcher;
}