#pragma once

#include "../type_traits/core_traits.h"
#include "../concepts/concepts.h"

#include "../compilation_config_in.h"

namespace stdsharp
{
    template<typename From, typename To>
    using forward_cast_t = cv_ref_align_t<From&&, To>;
}

namespace stdsharp
{
    template<typename...>
    struct forward_cast_fn;

    template<typename From, typename To>
    struct forward_cast_fn<From, To>
    {
        using from_t = std::remove_cvref_t<From>;
        using to_t = std::remove_cvref_t<To>;

        [[nodiscard]] constexpr decltype(auto) operator()(From&& from) const noexcept
            requires std::derived_from<from_t, to_t> && not_same_as<from_t, to_t>
        { // c-style cast allow us cast derived to inaccessible base
            return (forward_cast_t<From, To>)cpp_forward(from); // NOLINT
        }

        [[nodiscard]] constexpr decltype(auto) operator()(From&& from) const noexcept
            requires std::same_as<from_t, to_t>
        {
            return static_cast<forward_cast_t<From, To>>(cpp_forward(from));
        }
    };

    template<typename From, typename To, typename... Rest>
    struct forward_cast_fn<From, To, Rest...>
    {
    private:
        using cur_fn = forward_cast_fn<From, To>;
        using next_fn = forward_cast_fn<To, Rest...>;

    public:
        [[nodiscard]] constexpr decltype(auto) operator()(From&& from) const noexcept
            requires requires { next_fn{}(cur_fn{}(from)); }
        {
            return next_fn{}(cur_fn{}(from));
        }
    };

    template<typename... T>
    inline constexpr forward_cast_fn<T...> forward_cast{};
}

#include "../compilation_config_out.h"