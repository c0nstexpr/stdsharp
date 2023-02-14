#pragma once

#include <algorithm>

#include "allocator_traits.h"
#include "../ranges/ranges.h"
#include "../cassert/cassert.h"

namespace stdsharp
{


    template<allocator_req Alloc, ::std::ranges::forward_range Range>
        requires requires(typename allocator_traits<Alloc>::allocated allocated) //
    {
        requires ::std::same_as<::std::ranges::range_value_t<Range>, decltype(allocated)>;
        requires ::std::ranges::output_range<Range, decltype(allocated)>;
    }
    class allocation
    {
        Alloc alloc_{};
        Range allocated_{};

        using traits = allocator_traits<Alloc>;
        using pointer = typename traits::pointer;
        using size_type = typename traits::size_type;
        using allocated_type = typename traits::allocated;

        constexpr void deallocate_all_impl() noexcept
        {
            ::std::ranges::for_each(
                allocated_,
                [alloc = &this->alloc_](const allocated_type allocated) noexcept
                {
                    if(allocated.ptr != nullptr)
                        traits::deallocate(alloc, allocated.ptr, allocated.size);
                }
            );
        }

        constexpr void allocate_from(const Range& other_allocated)
        {
            ::std::ranges::for_each(
                other_allocated,
                [this, it = allocated_.begin()](const allocated_type allocated) mutable
                {
                    *it = {
                        allocated.ptr == nullptr ? //
                            nullptr :
                            traits::allocate(alloc_, allocated.size),
                        allocated.size //
                    };
                    ++it;
                }
            );
        }

        constexpr void reallocate_from(const Range& allocated)
        {
            ::std::ranges::for_each(
                allocated,
                [this, begin = allocated_.begin()](const allocated_type allocated) mutable
                {
                    if(allocated.ptr == nullptr) *begin = {};
                    else reallocate_from(begin, allocated);
                    ++begin;
                }
            );
        }

    public:
        allocation() = default;

        template<typename... Args>
            requires ::std::constructible_from<Alloc, Args...>
        constexpr allocation(Args&&... args) noexcept(nothrow_constructible_from<Alloc, Args...>):
            alloc_(::std::forward<Args>(args)...)
        {
        }

        constexpr allocation(const allocation& other):
            alloc_(traits::select_on_container_copy_construction(other.alloc_))
        {
            allocate_from(other.allocated_);
        }

        constexpr allocation(allocation&& other) noexcept:
            alloc_(::std::move(other.alloc_)), allocated_(other.allocated_)
        {
        }

        constexpr allocation& operator=(const allocation& other)
        {
            if(this == &other) return *this;

            if constexpr(traits::propagate_on_container_copy_assignment::value)
            {
                if(alloc_ != other.alloc_) deallocate_all_impl();
                alloc_ = other.alloc_;
                allocate_from(other.allocated_);
            }
            else reallocate_from(other.allocated_);

            return *this;
        }

        constexpr allocation& operator=(allocation&& other) noexcept
        {
            if(this == &other) return *this;

            if constexpr(traits::propagate_on_container_move_assignment::value)
            {
                deallocate_all_impl();
                alloc_ = ::std::move(other.alloc_);
            }
            else if(alloc_ != other.alloc_)
            {
                reallocate_from(other.allocated_);
                return *this;
            }

            ::std::ranges::copy(other.allocated_, allocated_.begin());

            return *this;
        }

        constexpr ~allocation() noexcept { deallocate_all_impl(); }

        constexpr auto rng() const noexcept
        {
            namespace std_rng = ::std::ranges;

            return std_rng::subrange{std_rng::cbegin(allocated_), std_rng::cend(allocated_)};
        }

        constexpr void allocate(const const_iterator_t<Range> it, const size_type size)
        {
            if constexpr(is_debug)
                if(it <=) {}

            if(size <= it->size) return;

            if(it->ptr != nullptr) traits::deallocate(alloc_, it->ptr, it->size);

            const_cast<allocated_type&>(*it) = // NOLINT(*-const-cast)
                {traits::allocate(alloc_, size), size};
        }

        constexpr void deallocate(const const_iterator_t<Range> it) noexcept
        {
            if(it->ptr == nullptr) return;

            traits::deallocate(alloc_, it->ptr, it->size);
            const_cast<allocated_type&>(*it) = {}; // NOLINT(*-const-cast)
        }

        constexpr void deallocate_all() noexcept
        {
            deallocate_all_impl();
            allocated_ = {};
        }
    };
}