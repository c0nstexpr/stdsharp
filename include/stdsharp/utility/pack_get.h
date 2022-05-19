#pragma once

#include <range/v3/functional/overload.hpp>

#include "../type_traits/core_traits.h"

namespace stdsharp
{
    template<::std::size_t N>
    struct pack_get_fn
    {
    private:
        template<::std::size_t... I, typename... Args>
        static constexpr decltype(auto) impl(
            const ::std::index_sequence<I...>,
            Args&&... args // clang-format off
        ) noexcept // clang-format on
        {
            return ::ranges::overload(
                [&args](const type_traits::index_constant<I>) -> Args&&
                {
                    return ::std::forward<Args>(args); //
                }... // clang-format off
            )(type_traits::index_constant<N>{}); // clang-format on
        }

    public:
        template<typename... Args>
            requires(N < sizeof...(Args))
        [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args) const noexcept
        {
            return impl(::std::index_sequence_for<Args...>{}, ::std::forward<Args>(args)...);
        }
    };

    template<::std::size_t N>
    inline constexpr pack_get_fn<N> pack_get{};

    template<::std::size_t N, typename... T>
    using pack_get_t = ::std::invoke_result_t<pack_get_fn<N>, T...>;
}