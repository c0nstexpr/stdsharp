#include "composed_allocator.h"
#include "static_allocator.h"
#include "object_allocation.h"
#include "stdsharp/memory/static_memory_resource.h"

namespace stdsharp
{
    namespace details
    {
        template<typename Allocator>
        concept soo_allocator = allocator_req<Allocator> &&
            ::std::same_as<typename Allocator::value_type, generic_storage>;

        template<::std::size_t Size>
        using generic_static_allocator =
            static_allocator<generic_storage, Size * sizeof(generic_storage)>;
    }

    template<
        ::std::size_t Size = 1,
        details::soo_allocator Allocator = ::std::allocator<generic_storage>,
        special_mem_req Req = special_mem_req::normal // clang-format off
    > // clang-format on
    struct basic_static_cached :
        basic_object_allocation<
            Req,
            composed_allocator<details::generic_static_allocator<Size>, Allocator>>
    {
        using base = basic_object_allocation<Req, typename basic_static_cached::allocator_type>;

        using base::base;
    };

    template<
        ::std::size_t Size = 1,
        typename Allocator = ::std::allocator<generic_storage> // clang-format off
    > // clang-format on
    using static_cached = details::static_cached<Size, Allocator, object_allocation>;

    template<
        ::std::size_t Size = 1,
        typename Allocator = ::std::allocator<generic_storage> // clang-format off
    > // clang-format on
    using unique_static_cached = details::static_cached<Size, Allocator, unique_object_allocation>;
}