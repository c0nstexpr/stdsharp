#pragma once

#include "../concepts/type.h"
#include "../macros.h"
#include "../type_traits/type.h"

#include "../compilation_config_in.h"

namespace stdsharp::details
{
    template<typename... Pairs>
    struct forward_cast_pairs
    {
        template<typename... OtherPairs>
        static forward_cast_pairs<Pairs..., OtherPairs...>
            append(forward_cast_pairs<OtherPairs...>);

        template<typename Pair>
        using append_t = decltype(append(Pair{}));

        static constexpr auto invoke(auto&& from) noexcept //
            -> decltype((cpp_forward(from) | ... | Pairs{}))
        {
            return (cpp_forward(from) | ... | Pairs{});
        }
    };
}

namespace stdsharp
{
    template<typename...>
    struct forward_cast_fn;

    template<typename From, typename To>
    using forward_cast_t = forward_cast_fn<From, To>::cast_t;

    template<typename From, typename To>
    struct forward_cast_fn<From, To>
    {
    private:
        template<typename...>
        friend class forward_cast_fn;

        using pairs = details::forward_cast_pairs<forward_cast_fn>;

        using no_ref_from_t = std::remove_reference_t<From>;

    public:
        using from_t = std::remove_cvref_t<From>;
        using to_t = std::remove_cvref_t<To>;
        using cast_t = apply_qualifiers<
            To,
            const_<no_ref_from_t>,
            volatile_<no_ref_from_t>,
            ref_qualifier_v<From&&>>;

        template<typename T>
            requires std::same_as<std::remove_cvref_t<T>, from_t> && base_of<to_t, from_t> &&
            not_same_as<from_t, to_t>
        [[nodiscard]] constexpr decltype(auto) operator()(T&& from) const noexcept
        { // c-style cast allow us cast to inaccessible base
            return (cast_t)from; // NOLINT
        }

        template<typename T>
            requires std::same_as<std::remove_cvref_t<T>, from_t>
        [[nodiscard]] constexpr decltype(auto) operator()(T&& from) const noexcept
        {
            return static_cast<cast_t>(cpp_forward(from));
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
    struct forward_cast_fn<From, To, Rest...>
    {
    private:
        template<typename...>
        friend class forward_cast_fn;

        using current_t = forward_cast_fn<From, To>;

        using pairs = details::forward_cast_pairs<current_t>:: //
            template append_t<typename forward_cast_fn<typename current_t::cast_t, Rest...>::pairs>;

    public:
        [[nodiscard]] constexpr auto operator()(auto&& from) const noexcept -> //
            decltype(pairs::invoke(cpp_forward(from)))
        {
            return pairs::invoke(cpp_forward(from));
        }
    };

    template<typename... T>
    inline constexpr forward_cast_fn<T...> forward_cast{};
}

#include "../compilation_config_out.h"