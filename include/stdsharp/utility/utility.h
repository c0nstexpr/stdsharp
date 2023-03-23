
#pragma once

#include <utility>

#include "adl_proof.h"
#include "auto_cast.h"
#include "cast_to.h"
#include "constructor.h"
#include "invocable.h"
#include "value_wrapper.h"
#include "implementation_reference.h"

namespace stdsharp
{
    template<typename T>
    struct forward_like_fn
    {
    private:
        template<typename U>
        using copy_const_t = ::std::conditional_t<const_<::std::remove_reference_t<T>>, const U, U>;

    public:
        template<typename U>
        [[nodiscard]] constexpr ref_align_t<T&&, copy_const_t<::std::remove_reference_t<U>>>
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