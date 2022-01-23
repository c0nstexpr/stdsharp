//
// Created by BlurringShadow on 2021-9-27.
//

#pragma once
#include <ranges>
#include <range/v3/range.hpp>
#include <range/v3/view.hpp>

#include "functional/decompose.h"
#include "type_traits/type_traits.h"

namespace stdsharp
{
    namespace ranges
    {
        template<typename T>
        using const_iterator_t = decltype(::std::ranges::cbegin(::std::declval<T&>()));

        template<typename T>
        using range_const_reference_t =
            type_traits::add_const_lvalue_ref_t<::std::ranges::range_value_t<T>>;

        template<auto>
        struct rng_as_iters_fn;

        template<>
        struct rng_as_iters_fn<functional::decompose_size> : type_traits::index_constant<2>
        {
        };

        template<>
        struct rng_as_iters_fn<::std::size_t{0}> : functional::nodiscard_tag_t
        {
            template<::std::ranges::range Rng>
            [[nodiscard]] constexpr auto operator()(Rng&& rng) const
                noexcept(noexcept(::std::ranges::begin(rng)))
            {
                return ::std::ranges::begin(::std::forward<Rng>(rng)); //
            }
        };

        template<>
        struct rng_as_iters_fn<::std::size_t{1}> : functional::nodiscard_tag_t
        {
            template<::std::ranges::range Rng>
            [[nodiscard]] constexpr auto operator()(Rng&& rng) const
                noexcept(noexcept(::std::ranges::end(rng)))
            {
                return ::std::ranges::end(::std::forward<Rng>(rng)); //
            }
        };

        template<::std::size_t I>
        inline constexpr functional::cpo_fn<rng_as_iters_fn<I>> rng_as_iters;
    }

    namespace functional
    {
        template<>
        struct decompose_size_t<ranges::rng_as_iters> : type_traits::index_constant<2>
        {
        };

#define BS_STD_RANGES_NODISCARD_OBJ(obj_name)                                         \
    template<>                                                                        \
    struct is_nodiscard_func_obj<::std::decay_t<decltype(::std::ranges::obj_name)>> : \
        ::std::bool_constant<true>                                                    \
    {                                                                                 \
    };

#define BS_STD_RANGES_RANGE_OBJ(obj_name)    \
    BS_STD_RANGES_NODISCARD_OBJ(obj_name)    \
    BS_STD_RANGES_NODISCARD_OBJ(c##obj_name) \
    BS_STD_RANGES_NODISCARD_OBJ(cr##obj_name)

        BS_STD_RANGES_RANGE_OBJ(begin)
        BS_STD_RANGES_RANGE_OBJ(end)

#undef BS_STD_RANGES_RANGE_OBJ

        BS_STD_RANGES_NODISCARD_OBJ(size)
        BS_STD_RANGES_NODISCARD_OBJ(ssize)
        BS_STD_RANGES_NODISCARD_OBJ(empty)
        BS_STD_RANGES_NODISCARD_OBJ(data)
        BS_STD_RANGES_NODISCARD_OBJ(cdata)

#undef BS_STD_RANGES_NODISCARD_OBJ

#define BS_RANGES_NODISCARD_OBJ(obj_name)                                        \
    template<>                                                                   \
    struct is_nodiscard_func_obj<::std::decay_t<decltype(::ranges::obj_name)>> : \
        ::std::bool_constant<true>                                               \
    {                                                                            \
    };

#define BS_RANGES_RANGE_OBJ(obj_name)    \
    BS_RANGES_NODISCARD_OBJ(obj_name)    \
    BS_RANGES_NODISCARD_OBJ(c##obj_name) \
    BS_RANGES_NODISCARD_OBJ(cr##obj_name)

        BS_RANGES_RANGE_OBJ(begin)
        BS_RANGES_RANGE_OBJ(end)

#undef BS_RANGES_RANGE_OBJ

        BS_RANGES_NODISCARD_OBJ(at)
        BS_RANGES_NODISCARD_OBJ(size)
        BS_RANGES_NODISCARD_OBJ(front)
        BS_RANGES_NODISCARD_OBJ(back)
        BS_RANGES_NODISCARD_OBJ(index)
        BS_RANGES_NODISCARD_OBJ(empty)
        BS_RANGES_NODISCARD_OBJ(data)
        BS_RANGES_NODISCARD_OBJ(cdata)

#undef BS_RANGES_NODISCARD_OBJ
    }
}
