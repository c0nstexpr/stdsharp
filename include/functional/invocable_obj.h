//
// Created by BlurringShadow on 2021-9-17.
//

#pragma once
#include <range/v3/functional/overload.hpp>

#include "concepts/concepts.h"

namespace stdsharp::functional
{
    inline constexpr struct nodiscard_tag_t
    {
    } nodiscard_tag;

    template<typename T>
    struct is_nodiscard_func_obj : ::std::bool_constant<::std::derived_from<T, nodiscard_tag_t>>
    {
    };

    template<typename T>
    concept nodiscard_func_obj = is_nodiscard_func_obj<T>::value;

    template<typename Invocable, typename = void>
    class invocable_obj : public ::ranges::overloaded<Invocable>
    {
        using base = ::ranges::overloaded<Invocable>;

    public:
        using base::base;
        using invocable_t = Invocable;
    };

    template<typename Invocable>
    class invocable_obj<Invocable, nodiscard_tag_t> : invocable_obj<Invocable>, nodiscard_tag_t
    {
        using base = invocable_obj<Invocable>;

    public:
        using typename invocable_obj::base::invocable_t;

        template<typename... T>
            requires ::std::constructible_from<Invocable, T...>
        constexpr explicit invocable_obj(const nodiscard_tag_t, T&&... t) //
            noexcept(concepts::nothrow_constructible_from<Invocable, T...>):
            invocable_obj::base(::std::forward<T>(t)...)
        {
        }

        template<typename... Args>
            requires ::std::invocable<const Invocable&, Args...>
        [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args) const& //
            noexcept(concepts::nothrow_invocable<const Invocable, Args...>)
        {
            return base::operator()(::std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires ::std::invocable<Invocable&, Args...>
        [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args) & //
            noexcept(concepts::nothrow_invocable<Invocable&, Args...>)
        {
            return base::operator()(::std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires ::std::invocable<const Invocable, Args...>
        [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args) const&& //
            noexcept(concepts::nothrow_invocable<const Invocable, Args...>)
        {
            return static_cast<const base&&>(*this)(::std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires ::std::invocable<Invocable, Args...>
        [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args) && //
            noexcept(concepts::nothrow_invocable<Invocable, Args...>)
        {
            return static_cast<base&&>(*this)(::std::forward<Args>(args)...);
        }
    };

    template<typename Invocable>
    invocable_obj(Invocable&&) -> invocable_obj<::std::decay_t<Invocable>>;

    template<typename Invocable>
    invocable_obj(nodiscard_tag_t, Invocable&&)
        -> invocable_obj<::std::decay_t<Invocable>, nodiscard_tag_t>;

    inline constexpr invocable_obj make_invocable_ref(
        nodiscard_tag,
        ::ranges::overload(
            []<typename Invocable>(Invocable& invocable) noexcept
            {
                return invocable_obj<Invocable&>{invocable}; //
            },
            []<typename Invocable>(nodiscard_tag_t, Invocable& invocable) noexcept //
            {
                return invocable_obj<Invocable&>{nodiscard_tag, invocable};
            } // clang-format off
        ) // clang-format on
    );
}
