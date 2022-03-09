// Created by BlurringShadow at 2021-02-27-下午 10:24

#pragma once

#include <utility>

#include "utility/pack_get.h"
#include "utility/value_wrapper.h"

namespace stdsharp
{
    using namespace ::std::literals;

    inline constexpr struct
    {
    private:
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

    public:
        template<typename T>
        [[nodiscard]] constexpr auto operator()(T&& t) const noexcept
        {
            return auto_cast<T>{::std::forward<T>(t)}; //
        }
    } auto_cast{};

    inline constexpr struct
    {
        template<concepts::enumeration T>
        [[nodiscard]] constexpr auto operator()(const T v) const noexcept
        {
            return static_cast<::std::underlying_type_t<T>>(v);
        }
    } to_underlying{};

    template<typename T>
    struct forward_like_fn
    {
        template<typename U>
        [[nodiscard]] constexpr type_traits::const_ref_align_t<T&&, ::std::remove_cvref_t<U>>
            operator()(U&& x) const noexcept
        {
            return auto_cast(x);
        }
    };

    template<typename T>
    inline constexpr forward_like_fn<T> forward_like{};

    template<typename T, typename U>
    using forward_like_t = decltype(forward_like<T>(::std::declval<U>()));
}