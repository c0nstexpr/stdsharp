// Created by BlurringShadow at 2021-02-27-下午 10:24

#pragma once

#include <utility>
#include <array>
#include <vector>
#include <string>
#include <string_view>

#include "type_traits/core_traits.h"

namespace stdsharp
{
    using namespace ::std::literals;

    namespace details
    {
        template<typename T>
        struct auto_cast
        {
            T&& t;

            template<typename U>
                requires requires
                {
                    static_cast<U>(t);
                } // NOLINTNEXTLINE(hicpp-explicit-conversions)
            [[nodiscard]] constexpr operator U() const&& noexcept(noexcept(static_cast<U>(t)))
            {
                return static_cast<U>(t);
            }

            template<typename U>
            [[nodiscard]] constexpr U operator()() const&& noexcept(noexcept(static_cast<U>(t)))
            {
                return static_cast<U>(*this);
            }
        };
    }

    inline constexpr struct
    {
        template<typename T>
        [[nodiscard]] constexpr auto operator()(T&& t) const noexcept
        {
            return details::auto_cast<T>{::std::forward<T>(t)}; //
        }
    } auto_cast{};

    namespace details
    {
        template<typename T, typename U>
        struct forward_like_fn
        {
        private:
            struct deduce_helper
            {
                U u;
            };

        public:
            [[nodiscard]] constexpr auto operator()(U&& x) const noexcept
                -> decltype(::std::declval<type_traits::const_ref_align_t<T&&, deduce_helper>>().m)
            {
                return auto_cast(x);
            }
        };

        template<typename T>
        struct forward_like_fn<T, void>
        {
            template<typename U>
            [[nodiscard]] constexpr auto operator()(U&& x) const noexcept
                -> type_traits::const_ref_align_t<T&&, ::std::remove_reference_t<U>>
            {
                return auto_cast(x);
            }
        };
    }

    inline constexpr struct
    {
        template<typename T>
            requires ::std::is_enum_v<T>
        [[nodiscard]] constexpr auto operator()(const T v) const noexcept
        {
            return static_cast<::std::underlying_type_t<T>>(v);
        }
    } to_underlying{};

    template<typename T, typename U = void>
    inline constexpr details::forward_like_fn<T, U> forward_like{};

    template<typename T, typename U = void>
    using forward_like_t = decltype( //
        forward_like<
            T,
            ::std::conditional_t<::std::same_as<U, void>, void, U> // clang-format off
        >(::std::declval<U>()) // clang-format on
    );
}