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
            template<typename allocator_type>
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

            template<typename allocator_type>
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

    template<allocator_req allocator_type>
    constexpr allocation<allocator_type> make_allocation(
        allocator_type& alloc,
        const allocator_size_type<allocator_type> size,
        const allocator_cvp<allocator_type>& hint = nullptr
    )
    {
        return details::allocation_access::make_allocation(alloc, size, hint);
    }

    template<allocator_req allocator_type>
    constexpr allocation<allocator_type> try_make_allocation(
        allocator_type& alloc,
        const allocator_size_type<allocator_type> size,
        const allocator_cvp<allocator_type>& hint = nullptr
    ) noexcept
    {
        return details::allocation_access::try_make_allocation(alloc, size, hint);
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
        using typename traits::propagate_on_container_copy_assignment;
        using typename traits::propagate_on_container_move_assignment;
        using typename traits::propagate_on_container_swap;
        using typename traits::is_always_equal;

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
                if constexpr(is_debug)
                    precondition<::std::out_of_range>(
                        [condition = (offset * sizeof(value_type) + count * sizeof(T)) <=
                             (size_ * sizeof(value_type))] { return condition; },
                        "pointer out of range"
                    );

                return {pointer_cast<T>(ptr_ + offset), count};
            }

            template<typename T>
            [[nodiscard]] constexpr auto as_span(const difference_type offset = 0) const noexcept
            {
                if constexpr(is_debug)
                    precondition<::std::out_of_range>(
                        [condition = offset <= size_] { return condition; },
                        "pointer out of range"
                    );

                return ::std::span<T>{
                    pointer_cast<T>(ptr_ + offset),
                    pointer_cast<T>(ptr_ + size_) //
                };
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
        template<typename T, auto Impl>
        struct use_alloc_do_as_fn
        {
            using span_t = const ::std::span<T>&;
            using fn = decltype(Impl);

            static constexpr bool is_invocable =
                ::std::invocable<fn, allocator_type&, span_t, span_t>;

            static constexpr bool is_noexcept =
                nothrow_invocable<fn, allocator_type&, span_t, span_t>;

            constexpr void
                operator()(allocator_type& alloc, const allocation& dest, const allocation& src)
                    const noexcept(is_noexcept)
                requires is_invocable
            {
                Impl(alloc, dest.template as_span<T>(), src.template as_span<T>());
            }

            constexpr void operator()(
                allocator_type& alloc,
                const allocation& dest,
                const allocation& src,
                const size_type count
            ) const noexcept(is_noexcept)
                requires is_invocable
            {
                Impl(alloc, dest.template as_span<T>(count), src.template as_span<T>(count));
            }
        };

        template<typename T, auto Impl, typename Fn = decltype(Impl)>
        struct do_as_fn
        {
            using span_t = const ::std::span<T>&;

            static constexpr bool is_invocable = ::std::invocable<Fn, span_t, span_t>;

            static constexpr bool is_noexcept = nothrow_invocable<Fn, span_t, span_t>;

            constexpr void operator()(const allocation& dest, const allocation& src) const
                noexcept(is_noexcept)
                requires is_invocable
            {
                Impl(dest.template as_span<T>(), src.template as_span<T>());
            }

            constexpr void
                operator()(const allocation& dest, const allocation& src, const size_type count)
                    const noexcept(is_noexcept)
                requires is_invocable
            {
                Impl(dest.template as_span<T>(count), src.template as_span<T>(count));
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
        struct move_as_impl_fn
        {
            using span_t = const ::std::span<T>&;

            // NOLINTNEXTLINE(*-swappable-parameters)
            constexpr void operator()(allocator_type& alloc, span_t dest, span_t src) const
                noexcept(nothrow_move_constructible<T>)
                requires ::std::move_constructible<T>
            {
                for(auto d_first = dest.begin(); const auto& v : src)
                    traits::construct(alloc, d_first++, ::std::move(v));
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
        struct move_assign_as_impl_fn
        {
            using span_t = const ::std::span<T>&;

            constexpr void operator()(span_t dest, span_t src) const
                noexcept(nothrow_move_constructible<T>)
                requires ::std::move_constructible<T>
            {
                for(auto d_first = dest.begin(); auto&& v : src) *(d_first++) = ::std::move(v);
            }
        };

        template<typename T>
        struct swap_as_impl_fn
        {
            using span_t = const ::std::span<T>&;

            constexpr void operator()(span_t dest, span_t src) const noexcept(nothrow_swappable<T>)
                requires ::std::swappable<T>
            {
                for(auto d_first = dest.begin(); auto& v : src)
                    ::std::ranges::swap(*(d_first++), v);
            }
        };

    public:
        template<typename T = value_type>
        struct delete_as_fn
        {
        private:
            constexpr void
                operator()(allocator_type& alloc, const ::std::span<T>& dest) const noexcept
            {
                for(auto& v : dest) traits::destroy(alloc, &v);
            }

        public:
            constexpr void operator()(allocator_type& alloc, const allocation& dest) const noexcept
            {
                (*this)(alloc, dest.template as_span<T>());
            }

            constexpr void
                operator()(allocator_type& alloc, const allocation& dest, const size_type count)
                    const noexcept
            {
                (*this)(alloc, dest.template as_span<T>(count));
            }
        };

        template<typename T = value_type>
        static constexpr delete_as_fn<T> delete_as{};

        template<typename T = value_type>
        using copy_as_fn = use_alloc_do_as_fn<T, copy_as_impl_fn<T>{}>;

        template<typename T = value_type>
        static constexpr copy_as_fn<T> copy_as{};

        template<typename T = value_type>
        using move_as_fn = use_alloc_do_as_fn<T, move_as_impl_fn<T>{}>;

        template<typename T = value_type>
        static constexpr move_as_fn<T> move_as{};

        template<typename T = value_type>
        using copy_assign_as_fn = do_as_fn<T, copy_assign_as_impl_fn<T>{}>;

        template<typename T = value_type>
        static constexpr copy_assign_as_fn<T> copy_assign_as{};

        template<typename T = value_type>
        using move_assign_as_fn = do_as_fn<T, move_assign_as_impl_fn<T>{}>;

        template<typename T = value_type>
        static constexpr move_assign_as_fn<T> move_assign_as{};

        template<typename T = value_type>
        using swap_as_fn = do_as_fn<T, swap_as_impl_fn<T>{}>;

        template<typename T = value_type>
        static constexpr swap_as_fn<T> swap_as{};

        using allocation_pair = ::std::pair<allocator_type&, allocation>;

        template<
            ::std::invocable<allocator_type&, const allocation&, const allocation&> Copy =
                copy_as_fn<>>
        static constexpr allocation
            copy_construct(allocator_type& alloc, const allocation& other, Copy&& copy = {})
        {
            const auto& res = make_allocation(alloc, other.size());
            ::std::invoke(::std::forward<Copy>(copy), res, other);
            return res;
        }

        static constexpr allocation move_construct(const allocation& other) noexcept
        {
            return other;
        }

        template<
            ::std::invocable<const allocation&, const allocation&> CopyFn =
                invocables<copy_assign_as_fn<>, delete_as_fn<>, copy_as_fn<>>>
            requires ::std::invocable<CopyFn, allocator_type&, const allocation&> &&
            ::std::invocable<CopyFn, allocator_type&, const allocation&, const allocation&>
        static constexpr void
            copy_assign(allocation_pair& dest, const allocation_pair& src, CopyFn copy = {})
        {
            if(dest.second.size() >= src.second.size())
            {
                ::std::invoke(copy, dest.second, src);
                return;
            }

            ::std::invoke(copy, dest.first, dest.second);
            dest.second.deallocate(dest.first);
            dest.second = copy_construct(dest.first, src, ::std::move(copy));
        }

        template<
            ::std::invocable<const allocation&, const allocation&> CopyFn =
                invocables<copy_assign_as_fn<>, delete_as_fn<>, copy_as_fn<>>>
            requires ::std::invocable<CopyFn, allocator_type&, const allocation&> &&
            ::std::invocable<CopyFn, allocator_type&, const allocation&, const allocation&> &&
            propagate_on_container_copy_assignment::value
        static constexpr void
            copy_assign(allocation_pair& dest, const allocation_pair& src, CopyFn copy = {})
        {
            if((is_always_equal::value || dest.first == src.first) &&
               dest.second.size() >= src.second.size())
            {
                dest.first = src.first;
                ::std::invoke(copy, dest.second, src.second);
                return;
            }

            ::std::invoke(copy, dest.first, dest.second);
            dest.second.deallocate(dest.first);
            dest.first = src.first;
            dest.second = copy_construct(dest.first, src.second, ::std::move(copy));
        }

        template<
            ::std::invocable<const allocation&, const allocation&> MoveFn =
                invocables<move_assign_as_fn<>, delete_as_fn<>, move_as_fn<>>>
            requires ::std::invocable<MoveFn, allocator_type&, const allocation&> &&
            ::std::invocable<MoveFn, allocator_type&, const allocation&, const allocation&>
        static constexpr void
            move_assign(allocation_pair& dest, allocation_pair&& src, MoveFn move = {})
        {
            const auto delete_fn = [&move, &dest] { ::std::invoke(move, dest.first, dest.second); };

            if(dest.first == src.first)
            {
                delete_fn();
                dest.second.deallocate(dest.first);
                dest.second = src.second;
                return;
            }

            if(dest.second.size() >= src.second.size())
            {
                ::std::invoke(move, dest.second, src.second);
                return;
            }

            delete_fn();
            dest.second.allocate(dest.first, src.size());
            ::std::invoke(::std::move(move), dest.first, dest.second, src.second);
        }

        template<::std::invocable<allocator_type&, const allocation&> MoveFn = delete_as_fn<>>
            requires propagate_on_container_move_assignment::value
        static constexpr void
            move_assign(allocation_pair& dest, allocation_pair&& src, MoveFn&& move = {}) noexcept
        {
            if(!(is_always_equal::value || dest.first == src.first))
            {
                ::std::invoke(::std::forward<MoveFn>(move), dest.first, dest.second);
                dest.second.deallocate(dest.first);
            }

            dest.first = ::std::move(src.first);
            dest.second = src.second;
        }

        template<
            ::std::invocable<const allocation&, const allocation&> SwapFn =
                invocables<move_assign_as_fn<>, move_as_fn<>>>
            requires ::std::invocable<SwapFn, allocator_type&, const allocation&, const allocation&>
        static constexpr void swap(allocation_pair& lhs, allocation_pair& rhs, SwapFn swap = {})
        {
            if(is_always_equal::value || lhs.first == rhs.first)
            {
                ::std::swap(lhs.second, rhs.second);
                return;
            }

            if(lhs.second.size() == rhs.second.size())
            {
                ::std::invoke(swap, lhs.second, rhs.second);
                return;
            }

            const auto move_construct = [&swap](allocation_pair& src)
            {
                const auto allocation = make_allocation(src.first, src.second.size());
                ::std::invoke(swap, src.first, allocation, src.second);
                return allocation;
            };
            const auto lhs_tmp = move_construct({lhs.first, rhs.second});
            const auto rhs_tmp = move_construct({rhs.first, lhs.second});

            lhs = lhs_tmp;
            rhs = rhs_tmp;
        }

        static constexpr void swap(allocation_pair& lhs, allocation_pair& rhs, const auto& = 0)
            requires propagate_on_container_swap::value
        {
            ::std::swap(lhs.second, rhs.second);
            ::std::ranges::swap(lhs.first, rhs.first);
        }
    };
}