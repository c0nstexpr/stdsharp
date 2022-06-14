// Created by BlurringShadow at 2021-02-27-下午 10:24

#pragma once

#include <utility>

#include "pack_get.h"
#include "value_wrapper.h"

namespace stdsharp
{
    inline constexpr struct
    {
    private:
        template<typename T>
        struct auto_cast_operator
        {
            T&& t;

            template<typename U>
                requires concepts::explicitly_convertible<T, U>
            [[nodiscard]] constexpr operator U() const&& //
                noexcept(concepts::nothrow_explicitly_convertible<T, U>)
            {
                return static_cast<U>(::std::forward<T>(t));
            }
        };

    public:
        template<typename T>
        [[nodiscard]] constexpr auto operator()(T&& t) const noexcept
        {
            return auto_cast_operator<T>{::std::forward<T>(t)}; //
        }
    } auto_cast{};

    template<typename T>
    struct forward_like_fn
    {
    private:
        template<typename U>
        using copy_const_t =
            ::std::conditional_t<concepts::const_<::std::remove_reference_t<T>>, const U, U>;

    public:
        template<typename U>
        [[nodiscard]] constexpr type_traits::
            ref_align_t<T&&, copy_const_t<::std::remove_reference_t<U>>>
            operator()(U&& u) const noexcept
        {
            return auto_cast(u);
        }
    };

    template<typename T>
    inline constexpr forward_like_fn<T> forward_like{};

    template<typename T, typename U>
    using forward_like_t = decltype(forward_like<T>(::std::declval<U>()));
}