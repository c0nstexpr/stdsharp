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

        using func = func_pointer<no_exception, Ret, Args...>;

        using not_null = gsl::not_null<func>;

        template<typename Closure>
            requires requires //
        {
            requires empty_type<Closure>;
            requires !(is_well_formed(req)) || invocable_r<const Closure&, Ret, Args...>;
            requires cpp_is_constexpr(Closure{});
            requires !no_exception || nothrow_invocable_r<const Closure&, Ret, Args...>;
        }
        [[nodiscard]] static constexpr auto encapsulate() noexcept
        {
            if constexpr(!(is_well_formed(req))) return;
            else if constexpr(explicitly_convertible<Closure, func>)
                return static_cast<func>(Closure{});
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
            constexpr dispatcher(const Closure /*unused*/) noexcept:
                not_null(encapsulate<Closure>())
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
            constexpr dispatcher& operator=(const Closure /*unused*/) noexcept
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

            constexpr dispatcher(const auto& /*unused*/) noexcept {}

            constexpr dispatcher(const func /*unused*/) noexcept {}

            dispatcher(nullptr_t) = delete;
            dispatcher(pointer auto) = delete;

            constexpr dispatcher& operator=(const auto& /*unused*/) noexcept { return *this; }

            constexpr dispatcher& operator=(const func /*unused*/) noexcept { return *this; }

            dispatcher& operator=(nullptr_t) = delete;
            dispatcher& operator=(pointer auto) = delete;

            constexpr bool operator==(const dispatcher /*unused*/) const noexcept { return true; }
        };
    };
}

namespace stdsharp
{
    template<expr_req ExprReq, typename Ret, typename... Args>
    using dispatcher =
        typename details::dispatcher_traits<ExprReq, Ret, Args...>::template dispatcher<>;
}