#pragma once

#include "../concepts/type.h"
#include "../macros.h"
#include "../type_traits/type.h"

#include "../compilation_config_in.h"

namespace stdsharp
{

    template<typename From, typename To>
    using forward_cast_t = apply_qualifiers<
        To,
        const_<std::remove_reference_t<From>>,
        volatile_<std::remove_reference_t<From>>,
        ref_qualifier_v<From&&>>;

    template<typename From, typename To, typename... Rest>
    struct forward_cast_fn
    {
    private:
        using current_fn = forward_cast_fn<From, To>;
        using current_cast_t = typename current_fn::cast_t;
        using next_fn = forward_cast_fn<current_cast_t, Rest...>;

    public:
        [[nodiscard]] constexpr decltype(auto) operator()(auto&& from) const noexcept
            requires std::invocable<current_fn, From> && std::invocable<next_fn, current_cast_t>
        {
            return next_fn{}(current_fn{}(static_cast<From&&>(from)));
        }
    };

    template<typename From, typename To>
        requires requires(std::remove_cvref_t<From> f, std::remove_cvref_t<To> t) {
            requires not_same_as<decltype(f), decltype(t)>;
            requires !(base_of<decltype(f), decltype(t)> || base_of<decltype(t), decltype(f)>);
        }
    struct forward_cast_fn<From, To>
    {
    };

    template<typename From, typename To>
    struct forward_cast_fn<From, To>
    {
        using cast_t = forward_cast_t<From, To>;

        // c-style cast allow us cast to inaccessible base
        [[nodiscard]] constexpr decltype(auto) operator()(std::remove_reference_t<From>&& from) //
            const noexcept
        {
            return (cast_t)from; // NOLINT
        }

        [[nodiscard]] constexpr decltype(auto) operator()(std::remove_reference_t<From>& from) //
            const noexcept
        {
            return (cast_t)from; // NOLINT
        }
    };

    template<typename From, typename To, typename... Rest>
    inline constexpr forward_cast_fn<From, To, Rest...> forward_cast{};
}

#include "../compilation_config_out.h"