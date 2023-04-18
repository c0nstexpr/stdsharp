#pragma once

#include <span>

#include "allocator_traits.h"
#include "pointer_traits.h"
#include "../cassert/cassert.h"
#include "../cstdint/cstdint.h"

namespace stdsharp
{
    template<typename>
    struct allocator_aware_traits;

    template<allocator_req Alloc>
    using allocation = typename allocator_aware_traits<Alloc>::allocation;

    namespace details
    {
        struct allocation_access
        {
            template<allocator_req allocator_type>
            static constexpr allocation<allocator_type> make_allocation(
                allocator_type& alloc,
                const allocator_size_type<allocator_type> size,
                const allocator_cvp<allocator_type>& hint = nullptr
            )
            {
                if(size == 0) [[unlikely]]
                    return {};

                return {allocator_traits<allocator_type>::allocate(alloc, size, hint), size};
            }

            template<allocator_req allocator_type>
            static constexpr allocation<allocator_type> try_make_allocation(
                allocator_type& alloc,
                const allocator_size_type<allocator_type> size,
                const allocator_cvp<allocator_type>& hint = nullptr
            ) noexcept
            {
                if(size == 0) [[unlikely]]
                    return {};

                return {allocator_traits<allocator_type>::try_allocate(alloc, size, hint), size};
            }
        };
    }

    template<typename... Args>
        requires requires //
    {
        details::allocation_access::make_allocation(::std::declval<Args>()...); //
    }
    constexpr auto make_allocation(Args&&... args)
    {
        return details::allocation_access::make_allocation(::std::forward<Args>(args)...);
    }

    template<typename... Args>
        requires requires //
    {
        details::allocation_access::try_make_allocation(::std::declval<Args>()...); //
    }
    constexpr auto try_make_allocation(Args&&... args) noexcept
    {
        return details::allocation_access::try_make_allocation(::std::forward<Args>(args)...);
    }

    template<typename Alloc>
    struct allocator_aware_traits : allocator_traits<Alloc>
    {
        using traits = allocator_traits<Alloc>;

        using typename traits::value_type;
        using typename traits::pointer;
        using typename traits::const_void_pointer;
        using typename traits::size_type;
        using typename traits::difference_type;
        using typename traits::allocator_type;

        class [[nodiscard]] allocation
        {
            friend details::allocation_access;

            pointer ptr_ = nullptr;
            size_type size_ = 0;

        public:
            template<typename T>
            [[nodiscard]] constexpr ::std::span<T>
                as_span(const difference_type offset, const size_type count) const noexcept
            {
                return {pointer_cast<T>(ptr_ + offset), count};
            }

            template<typename T>
            [[nodiscard]] constexpr auto as_span(const difference_type offset = 0) const noexcept
            {
                return as_span<T>(offset, size_ * sizeof(value_type) / sizeof(T));
            }

            [[nodiscard]] constexpr auto begin() const noexcept { return ptr_; }

            [[nodiscard]] constexpr auto end() const noexcept { return ptr_ + size_; }

            [[nodiscard]] constexpr auto size() const noexcept { return size_; }

            constexpr void allocate(allocator_type& alloc, const size_type size) noexcept
            {
                if(size_ >= size) return;

                if(ptr_ != nullptr) deallocate(alloc);

                *this = make_allocation(size);
            }

            constexpr void deallocate(allocator_type& alloc) noexcept
            {
                traits::deallocate(alloc, ptr_, size_);
                ptr_ = nullptr;
                size_ = 0;
            }

            [[nodiscard]] constexpr operator bool() const noexcept { return ptr_ != nullptr; }
        };

    private:
        template<typename T, auto Impl, typename Fn = decltype(Impl)>
        struct do_as_fn
        {
            using span_t = const ::std::span<T>&;

            static constexpr bool is_invocable =
                ::std::invocable<Fn, allocator_type&, span_t, span_t>;

            static constexpr bool is_noexcept =
                nothrow_invocable<Fn, allocator_type&, span_t, span_t>;

            template<typename Rng>
            constexpr void
                operator()(allocator_type& alloc, const allocation& dest, const allocation& src)
                    const noexcept(is_noexcept)
                requires is_invocable
            {
                Impl(alloc, dest.template as_span<T>(), src.template as_span<T>());
            }
        };

        template<typename T>
        struct copy_as_impl_fn
        {
            using span_t = const ::std::span<T>&;

            // NOLINTNEXTLINE(*-swappable-parameters)
            constexpr void operator()(allocator_type& alloc, span_t dest, span_t src) const
                noexcept(nothrow_copy_constructible<T>)
                requires ::std::copy_constructible<T>
            {
                for(auto d_first = dest.begin(); const auto& v : src)
                    traits::construct(alloc, d_first++, v);
            }
        };

        template<typename T>
        struct copy_assign_as_impl_fn
        {
            using span_t = const ::std::span<T>&;

            constexpr void operator()(span_t dest, span_t src_span) const
                noexcept(nothrow_copy_assignable<T>)
                requires copy_assignable<T>
            {
                for(auto d_first = dest.begin(); const auto& v : src_span) *(d_first++) = v;
            }
        };

        template<typename T>
        struct move_as_impl_fn
        {
            using span_t = const ::std::span<T>&;

            constexpr void operator()(span_t dest, span_t src) const
                noexcept(nothrow_move_constructible<T>)
                requires ::std::move_constructible<T>
            {
                for(auto d_first = dest.begin(); const auto& v : src) *(d_first++) = ::std::move(v);
            }
        };

    public:
        template<typename T = value_type>
        using copy_as_fn = do_as_fn<T, copy_as_impl_fn<T>{}>;

        template<typename T = value_type>
        static constexpr copy_as_fn copy_as{};

        template<typename T = value_type>
        using copy_assign_as_fn = do_as_fn<T, copy_assign_as_impl_fn<T>{}>;

        template<typename T = value_type>
        static constexpr copy_assign_as_fn copy_assign_as{};

        template<typename T = value_type>
        using move_as_fn = do_as_fn<T, move_as_impl_fn<T>{}>;

        template<typename T = value_type>
        static constexpr move_as_fn move_as{};

        template<
            ::std::invocable<allocator_type&, const allocation&, const allocation&> Copy =
                copy_as_fn<>>
        static constexpr allocation
            copy_construct(allocator_type& alloc, const allocation& other, Copy&& copy = {})
        {
            const auto& res = get_allocation(alloc, other.size());
            ::std::invoke(::std::forward<Copy>(copy), res, other);
            return res;
        }

        static constexpr allocation
            move_construct(const allocator_type&, const allocation& other) noexcept
        {
            return other;
        }

        template<
            ::std::invocable<const allocation&, const allocation&> AssignFn = copy_assign_as_fn<>>
        static constexpr void copy_assign(
            const allocation& dest,
            const allocation& src,
            AssignFn&& assign = {}
        ) noexcept(nothrow_invocable<AssignFn, const allocation&, const allocation&>)
        {
            ::std::invoke(::std::forward<AssignFn>(assign), dest, src);
        }
    };
}