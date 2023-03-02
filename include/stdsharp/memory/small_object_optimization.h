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
    }

    template<
        ::std::size_t Size,
        details::soo_allocator Allocator = ::std::allocator<generic_storage> // clang-format off
    > // clang-format on
    struct soo_traits
    {
        using static_allocator = static_allocator<generic_storage, Size * sizeof(generic_storage)>;
        using static_resource = typename static_allocator::resource_type;

        using allocator_type = composed_allocator<static_allocator, Allocator>;
        using allocator_traits = allocator_traits<allocator_type>;
    };

    namespace details
    {
        template<
            ::std::size_t Size,
            typename Allocator,
            template<typename>
            typename Allocation,
            typename SooTraits = soo_traits<Size, Allocator> // clang-format off
        > // clang-format on
        class static_cached : Allocation<typename SooTraits::allocator_type>
        {
            using static_rsc = typename SooTraits::static_resource;
            using m_base = Allocation<SooTraits>;

            static_rsc rsc_;

        public:
            constexpr static_cached() noexcept(nothrow_default_initializable<Allocator>):
                m_base(static_rsc{rsc_})
            {
            }

            template<typename... Args>
                requires ::std::constructible_from<Allocator, Args...>
            constexpr static_cached(Args&&... args) //
                noexcept(nothrow_constructible_from<Allocator, Args...>):
                m_base(static_rsc{rsc_}, Allocator{::std::forward<Args>(args)...})
            {
            }

            constexpr static_cached(static_cached&& other) noexcept:
                m_base(static_rsc{rsc_}, static_cast<m_base&&>(other))
            {
            }

            constexpr static_cached(const static_cached& other)
                requires ::std::copy_constructible<m_base>
                : m_base(static_rsc{rsc_}, static_cast<const m_base&>(other))
            {
            }

            constexpr static_cached& operator=(static_cached&& other) //
                noexcept(nothrow_move_assignable<m_base>)
            {
                static_cast<m_base&>(*this) = static_cast<m_base&&>(other);
                return *this;
            }

            constexpr static_cached& operator=(const static_cached& other)
                requires copy_assignable<m_base>
            {
                static_cast<m_base&>(*this) = static_cast<const m_base&>(other);
                return *this;
            }

            ~static_cached() = default;

            using m_base::get;
            using m_base::has_value;
            using m_base::is_same_type;
            using m_base::operator bool;
            using m_base::reset;
            using m_base::reserved;
            using m_base::size;
            using m_base::type;
            using m_base::get_allocator;
            using m_base::emplace;
        };
    }

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