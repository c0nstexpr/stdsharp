#pragma once

#include "../concepts/type.h"
#include "../macros.h"
#include "../type_traits/type.h"

#include "../compilation_config_in.h"

namespace stdsharp
{
    template<typename From, typename To, typename... Rest>
    struct forward_cast_fn
    {
    private:
        template<typename, typename, typename...>
        friend class forward_cast_fn;

        using current_fn = forward_cast_fn<From, To>;

        template<typename... Fn>
        using combined_fn = typename forward_cast_fn<typename current_fn::cast_t, Rest...>::
            template combined_fn<Fn..., current_fn>;

        using no_ref_from_t = std::remove_reference_t<From>;

    public:
        [[nodiscard]] constexpr auto operator()(auto&& from) const noexcept -> //
            decltype(combined_fn<>{}(static_cast<From&&>(from)))
        {
            return combined_fn<>{}(static_cast<From&&>(from));
        }
    };

    template<typename From, typename To>
    using forward_cast_t = forward_cast_fn<From, To>::cast_t;

    template<typename From, typename To>
    struct forward_cast_fn<From, To>
    {
    private:
        template<typename, typename, typename...>
        friend class forward_cast_fn;

        template<typename... Fn>
        struct combined_fn
        {
            constexpr auto operator()(auto&& from) const noexcept
                -> decltype((cpp_forward(from) | ... | Fn{}) | forward_cast_fn<From, To>{})
            {
                return (cpp_forward(from) | ... | Fn{}) | forward_cast_fn<From, To>{};
            }
        };

        using no_ref_from_t = std::remove_reference_t<From>;

    public:
        using from_t = std::remove_cv_t<no_ref_from_t>;
        using to_t = std::remove_cvref_t<To>;
        using cast_t = apply_qualifiers<
            To,
            const_<no_ref_from_t>,
            volatile_<no_ref_from_t>,
            ref_qualifier_v<From&&>>;

        template<typename T>
            requires(base_of<to_t, from_t> || base_of<from_t, to_t>) &&
            std::same_as<std::remove_reference_t<T>, no_ref_from_t>
        [[nodiscard]] constexpr decltype(auto) operator()(T&& from) const noexcept
        {
            // c-style cast allow us cast to inaccessible base
            if constexpr(not_same_as<from_t, to_t>) return (cast_t)from; // NOLINT
            else return static_cast<cast_t>(from);
        }

        template<typename T>
            requires std::invocable<forward_cast_fn, T>
        [[nodiscard]] friend constexpr decltype(auto) operator|(
            T&& from,
            const forward_cast_fn& self //
        ) noexcept
        {
            return self(cpp_forward(from));
        }
    };

    template<typename From, typename To, typename... Rest>
    inline constexpr forward_cast_fn<From, To, Rest...> forward_cast{};
}

#include "../compilation_config_out.h"