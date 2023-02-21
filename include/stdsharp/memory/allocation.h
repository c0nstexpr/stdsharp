#pragma once

#include <algorithm>

#include "../cassert/cassert.h"
#include "../containers/concepts.h"

namespace stdsharp
{
    template<allocator_req Alloc, typename Derived = void>
    class allocation
    {
        using traits = allocator_traits<Alloc>;
        using allocator_type = Alloc;

        allocator_type alloc_{};

#define STDSHARP_TO_DERIVED(const_, ref_)               \
    constexpr auto& to_derived() const_ ref_ noexcept   \
        requires class_<Derived>                        \
    {                                                   \
        return static_cast<const_ Derived ref_>(*this); \
    }

        STDSHARP_TO_DERIVED(const, &)
        STDSHARP_TO_DERIVED(const, &&)
        STDSHARP_TO_DERIVED(, &)
        STDSHARP_TO_DERIVED(, &&)

#undef STDSHARP_TO_DERIVED

    public:
        allocation() = default;

        template<typename... Args>
            requires ::std::constructible_from<Alloc, Args...>
        constexpr allocation(Args&&... args) //
            noexcept(nothrow_constructible_from<Alloc, Args...>):
            alloc_(::std::forward<Args>(args)...)
        {
        }

        constexpr allocation(const allocation& other) noexcept:
            alloc_(traits::select_on_container_copy_construction(other.alloc_))
        {
        }

        allocation(allocation&&) noexcept = default;

        constexpr allocation& operator=(const allocation& other) noexcept( //
            noexcept(
                to_derived().before_copy_assign(other.to_derived()),
                to_derived().after_copy_assign(other.to_derived())
            )
        )
            requires traits::propagate_on_container_copy_assignment::value
        {
            if(this == &other) return *this;


            to_derived().before_copy_assign(other.to_derived());
            alloc_ = other.alloc_;
            to_derived().after_copy_assign(other.to_derived());

            return *this;
        }

        constexpr allocation& operator=(const allocation& other) //
            noexcept(noexcept(to_derived().copy_assign(other.to_derived())))
        {
            if(this == &other) return *this;

            to_derived().copy_assign(other.to_derived());

            return *this;
        }

        constexpr allocation& operator=(allocation&& other) noexcept( //
            noexcept(
                to_derived().before_move_assign(::std::move(other).to_derived()),
                to_derived().after_move_assign(::std::move(other).to_derived())
            )
        )
            requires traits::propagate_on_container_move_assignment::value
        {
            if(this == &other) return *this;

            to_derived().before_move_assign(::std::move(other).to_derived());
            alloc_ = ::std::move(other.alloc_);
            to_derived().after_move_assign(::std::move(other).to_derived());

            return *this;
        }

        constexpr allocation& operator=(allocation&& other) //
            noexcept(noexcept(to_derived().move_assign(::std::move(other).to_derived())))
        {
            if(this == &other) return *this;

            to_derived().move_assign(::std::move(other).to_derived());

            return *this;
        }

        ~allocation() = default;

        constexpr decltype(auto) get_allocator() const noexcept { return alloc_; }

        constexpr decltype(auto) get_allocator() noexcept { return alloc_; }
    };
}