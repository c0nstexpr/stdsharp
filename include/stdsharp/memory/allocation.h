#pragma once

#include <algorithm>

#include "../cassert/cassert.h"
#include "../containers/concepts.h"

namespace stdsharp
{
    template<allocator_req Alloc, container Range, typename Derived = void>
        requires requires(typename allocator_traits<Alloc>::allocated allocated) //
    {
        requires ::std::same_as<typename Range::value_type, decltype(allocated)>;
        requires ::std::ranges::output_range<Range, decltype(allocated)>;
    }
    class allocation
    {
        using traits = allocator_traits<Alloc>;
        using allocator_type = Alloc;
        using range_type = Range;

    public:
        using allocated = typename traits::allocated;

    private:
        allocator_type alloc_{};
        range_type allocated_{};

        using pointer = typename traits::pointer;
        using size_type = typename traits::size_type;

        constexpr void reallocate_impl(allocated& v, const size_type size)
        {
            if(size <= v.size) return;

            if(v.ptr != nullptr) deallocate_impl(v);

            v = {traits::allocate(alloc_, size), size};

            if constexpr(requires { to_derived().on_allocate(v); }) to_derived().on_allocate(v);
        }

        constexpr void allocate_from(const Range& other)
        {
            for(auto it = allocated_.begin(); const allocated allocated : other)
            {
                *it = {
                    allocated.ptr == nullptr ? //
                        nullptr :
                        traits::allocate(alloc_, allocated.size),
                    allocated.size //
                };
                ++it;
            }
        }

        constexpr void reallocate_from(const Range& rng)
        {
            for(auto it = allocated_.begin(); const allocated& v : rng)
            {
                if(v.ptr != nullptr) reallocate_impl(it, v.size);
                ++it;
            }
        }

        constexpr void verify_rng_iter(const const_iterator_t<Range> it) const
        {
            precondition<::std::invalid_argument>(
                [it, &allocated = allocated_]
                {
                    const auto begin = allocated.cbegin();
                    const auto end = allocated.cend();

                    if(::std::is_constant_evaluated())
                    {
                        for(auto allocated_it = begin; allocated_it != end; ++allocated_it)
                            if(allocated_it == it) return true;
                        return false;
                    }

                    return it >= begin && it <= end;
                },
                "iterator not compatible with range"
            );
        }

        constexpr auto& to_derived() noexcept
            requires class_<Derived>
        {
            return static_cast<Derived&>(*this);
        }

        constexpr auto& to_derived() const noexcept
            requires class_<Derived>
        {
            return static_cast<const Derived&>(*this);
        }

        constexpr void deallocate_impl(allocated& allocated)
        {
            if(allocated.ptr == nullptr) return;

            if constexpr(noexcept(to_derived().on_deallocate(allocated)))
                to_derived().on_deallocate(allocated);

            traits::deallocate(alloc_, allocated.ptr, allocated.size);
            allocated = {};
        }

        constexpr void deallocate_impl(const const_iterator_t<Range> it)
        {
            deallocate_impl(const_cast<allocated&>(*it)); // NOLINT(*-const-cast)
        }

        constexpr void
            deallocate_impl(const const_iterator_t<Range> begin, const_iterator_t<Range> end)
        {
            for(; begin != end; ++begin) deallocate_impl(*begin);
        }

    public:
        allocation() = default;

        template<typename... AllocArgs, typename... RngArgs>
            requires ::std::constructible_from<Alloc, AllocArgs...> &&
                         ::std::constructible_from<Range, RngArgs...>
        constexpr allocation(AllocArgs&&... alloc_args, const empty_t, RngArgs&&... rng_args) //
            noexcept( //
                nothrow_constructible_from<Alloc, AllocArgs...>&&
                    nothrow_constructible_from<Range, RngArgs...>
            ):
            alloc_(::std::forward<AllocArgs>(alloc_args)...),
            allocated_(::std::forward<RngArgs>(rng_args)...)
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
                if(alloc_ != other.alloc_) release();
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
                release();
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

        constexpr ~allocation() noexcept { release(); }

        constexpr auto rng() const noexcept
        {
            namespace std_rng = ::std::ranges;
            return std_rng::subrange{std_rng::cbegin(allocated_), std_rng::cend(allocated_)};
        }

        constexpr void allocate(const const_iterator_t<Range> it, const size_type size)
        {
            verify_rng_iter(it);

            precondition<::std::invalid_argument>(
                [it, end = allocated_.cend()] { return it != end; },
                "input iterator cannot be the end iterator"
            );

            reallocate_impl(const_cast<allocated&>(*it), size); // NOLINT(*-const-cast)
        }

        constexpr void deallocate(const const_iterator_t<Range> it)
        {
            verify_rng_iter(it);
            deallocate_impl(it);
        }

        constexpr void deallocate(const const_iterator_t<Range> begin, const_iterator_t<Range> end)
        {
            verify_rng_iter(begin);
            verify_rng_iter(end);
            deallocate_impl(begin, end);
        }

        constexpr void release() noexcept { deallocate_impl(allocated_.begin(), allocated_.end()); }
    };
}