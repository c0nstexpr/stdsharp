#include "composed_allocator.h"
#include "static_allocator.h"
#include "object_allocation.h"

namespace stdsharp
{
    namespace details
    {
        template<typename Alloc>
        concept soo_alloc =
            allocator_req<Alloc> && ::std::same_as<typename Alloc::value_type, all_aligned>;

        template<
            ::std::size_t Size,
            typename Allocator,
            typename Base = composed_allocator<
                static_allocator_for<all_aligned, Size>,
                Allocator> // clang-format off
        > // clang-format on
        struct soo_allocator : Base
        {
            using Base::Base;

            template<typename T>
            struct rebind
            {
                using other = soo_allocator<
                    Size,
                    typename allocator_traits<Allocator>::template rebind_alloc<T>,
                    typename allocator_traits<Base>::template rebind_alloc<T> // clang-format off
                >; // clang-format on
            };

            template<typename... Args>
                requires ::std::constructible_from<Allocator, Args...>
            constexpr soo_allocator(
                static_memory_resource<Size>& resource,
                Args&&... args
            ) noexcept(nothrow_constructible_from<Allocator, Args...>):
                Base(
                    static_allocator_for<all_aligned, Size>{resource},
                    Allocator{::std::forward<Args>(args)...}
                )
            {
            }

            constexpr soo_allocator() noexcept(nothrow_constructible_from<Allocator>)
                requires ::std::constructible_from<Allocator>
                : soo_allocator(get_static_memory_resource<Size>())
            {
            }
        };
    }

    template<
        ::std::size_t Size = 1,
        details::soo_alloc Allocator = ::std::allocator<all_aligned> // clang-format off
    > // clang-format on
    using soo_allocator = details::soo_allocator<Size, Allocator>;

    template<
        typename T,
        ::std::size_t Size = 1,
        details::soo_alloc Allocator = ::std::allocator<all_aligned>>
    using soo_allocation_for = obj_allocation_for<T, soo_allocator<Size, Allocator>>;

    template<::std::size_t Size = 1, details::soo_alloc Allocator = ::std::allocator<all_aligned>>
    using trivial_soo_allocation = trivial_obj_allocation<soo_allocator<Size, Allocator>>;

    template<::std::size_t Size = 1, details::soo_alloc Allocator = ::std::allocator<all_aligned>>
    using normal_soo_allocation = normal_obj_allocation<soo_allocator<Size, Allocator>>;

    template<::std::size_t Size = 1, details::soo_alloc Allocator = ::std::allocator<all_aligned>>
    using unique_soo_allocation = unique_obj_allocation<soo_allocator<Size, Allocator>>;
}