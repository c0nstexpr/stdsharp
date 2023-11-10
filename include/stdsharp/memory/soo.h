#include "composed_allocator.h"
#include "static_allocator.h"
#include "box.h"

namespace stdsharp
{
    namespace details
    {
        template<typename Alloc>
        concept soo_alloc =
            allocator_req<Alloc> && std::same_as<typename Alloc::value_type, all_aligned>;
    }

    template<
        std::size_t Size = 1,
        details::soo_alloc Allocator = std::allocator<all_aligned> // clang-format off
    > // clang-format on
    using soo_allocator = composed_allocator<static_allocator_for<all_aligned, Size>, Allocator>;

    template<
        typename T,
        std::size_t Size = 1,
        details::soo_alloc Allocator = std::allocator<all_aligned>>
    using soo_box_for = box_for<T, soo_allocator<Size, Allocator>>;

    template<std::size_t Size = 1, details::soo_alloc Allocator = std::allocator<all_aligned>>
    using trivial_soo_box = trivial_box<soo_allocator<Size, Allocator>>;

    template<std::size_t Size = 1, details::soo_alloc Allocator = std::allocator<all_aligned>>
    using normal_soo_box = normal_box<soo_allocator<Size, Allocator>>;

    template<std::size_t Size = 1, details::soo_alloc Allocator = std::allocator<all_aligned>>
    using unique_soo_box = unique_box<soo_allocator<Size, Allocator>>;
}