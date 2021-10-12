//
// Created by BlurringShadow on 2021-9-27.
//
#pragma once
#include <ranges>
#include <range/v3/range.hpp>
#include <range/v3/view.hpp>

#include "functional/cpo.h"
#include "functional/functional.h"

namespace stdsharp
{
    namespace ranges
    {
        template<typename T>
        using const_iterator_t = decltype(::std::ranges::cbegin(std::declval<T&>()));

        template<typename T>
        using range_const_reference_t =
            ::stdsharp::type_traits::add_const_lvalue_ref_t<::std::ranges::range_value_t<T>>;

        inline constexpr struct rng_as_iters_fn
        {
        private:
            template<::std::ranges::range Rng>
            struct impl
            {
                static constexpr auto size = 2;

                Rng rng{};

                constexpr auto operator()(const ::stdsharp::type_traits::index_constant<0>) const
                    noexcept(noexcept(::std::ranges::begin(rng)))
                {
                    return ::std::ranges::begin(rng);
                }

                constexpr auto operator()(const ::stdsharp::type_traits::index_constant<0>) //
                    noexcept(noexcept(::std::ranges::begin(rng)))
                {
                    return ::std::ranges::begin(rng);
                }

                constexpr auto operator()(const ::stdsharp::type_traits::index_constant<1>) const
                    noexcept(noexcept(::std::ranges::end(rng)))
                {
                    return ::std::ranges::end(rng);
                }

                constexpr auto operator()(const ::stdsharp::type_traits::index_constant<1>) //
                    noexcept(noexcept(::std::ranges::end(rng)))
                {
                    return ::std::ranges::end(rng);
                }
            };

            template<typename Rng>
            impl(Rng&&) -> impl<::stdsharp::type_traits::coerce_t<Rng>>;

        public:
            template<typename T>
                requires requires
                {
                    ::stdsharp::ranges::rng_as_iters_fn::impl{::std::declval<T>()};
                }
            constexpr auto operator()(T&& t) const noexcept( //
                noexcept( //
                    ::stdsharp::ranges:: // clang-format off
                        rng_as_iters_fn::impl{::std::declval<T>()}  
                ) // clang-format on
            )
            {
                return ::stdsharp::ranges::rng_as_iters_fn::impl{::std::forward<T>(t)}; //
            }
        } rng_as_iters{};
    }

    namespace functional
    {
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
