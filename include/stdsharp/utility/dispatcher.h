#pragma once

#include <gsl/pointers>

#include "../type_traits/core_traits.h"
#include "../concepts/concepts.h"

namespace stdsharp::details
{
    template<expr_req ExprReq, typename Ret, typename... Args>
    struct dispatcher_traits
    {
        static constexpr auto req = ExprReq;
        static constexpr bool no_exception = is_noexcept(req);

        template<bool Noexcept = no_exception>
        using func_sig = Ret (*)(Args...) noexcept(Noexcept);

        using func = func_sig<>;

        using not_null = gsl::not_null<func>;

        template<invocable_r<Ret, Args...> Closure>
            requires requires //
        {
            requires empty_type<Closure>;
            requires cpp_is_constexpr(Closure{});
            requires !no_exception || nothrow_invocable_r<Closure, Ret, Args...>;
        }
        [[nodiscard]] static constexpr auto encapsulate() noexcept
        {
            if constexpr(explicitly_convertible<Closure, func>) return static_cast<func>(Closure{});
            else
                return +[](Args... args) noexcept(no_exception) -> Ret
                {
                    constexpr Closure c{};
                    if constexpr(std::same_as<void, Ret>) c(cpp_forward(args)...);
                    else return c(cpp_forward(args)...);
                };
        }

        [[nodiscard]] static constexpr auto get_default() noexcept
            requires(no_exception ? nothrow_default_initializable<Ret> : std::default_initializable<Ret>)
        {
            return [](const Args&...) noexcept(no_exception) { return Ret{}; };
        }

        [[nodiscard]] static constexpr auto get_default() noexcept
            requires std::same_as<void, Ret>
        {
            return [](const Args&...) noexcept {};
        }

        template<expr_req = req>
        struct dispatcher : public not_null
        {
        public:
            using not_null::not_null;
            using not_null::operator=;

            static constexpr auto requirement = ExprReq;

            template<typename Closure>
                requires requires {
                    encapsulate<Closure>();
                    requires !std::convertible_to<Closure, func>;
                }
            constexpr dispatcher(const Closure) noexcept: not_null(encapsulate<Closure>())
            {
            }

            constexpr dispatcher() noexcept
                requires requires { get_default(); }
                : dispatcher(get_default())
            {
            }

            template<typename Closure>
                requires requires {
                    encapsulate<Closure>();
                    requires !std::convertible_to<Closure, func>;
                }
            constexpr dispatcher& operator=(const Closure) noexcept
            {
                return (*this = encapsulate<Closure>());
            }

            constexpr bool operator==(const dispatcher other) const noexcept
            {
                return this->get() == other.get();
            }

            template<typename... U>
                requires std::invocable<func, U...>
            constexpr Ret operator()(U&&... args) const noexcept(no_exception)
            {
                return (*(this->get()))(cpp_forward(args)...);
            }
        };

        template<>
        struct dispatcher<expr_req::ill_formed>
        {
            static constexpr auto requirement = expr_req::ill_formed;

            dispatcher() = default;

            template<typename Closure>
                requires requires { encapsulate<Closure>(); }
            constexpr dispatcher(const Closure) noexcept
            {
            }

            template<typename Closure>
                requires requires { encapsulate<Closure>(); }
            constexpr dispatcher& operator=(const Closure) noexcept
            {
                return *this;
            }

            constexpr bool operator==(const dispatcher) const noexcept { return true; }
        };
    };
}

namespace stdsharp
{
    template<expr_req ExprReq, typename Ret, typename... Args>
    using dispatcher =
        typename details::dispatcher_traits<ExprReq, Ret, Args...>::template dispatcher<>;
}