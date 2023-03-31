#pragma once

#include "allocator_traits.h"
#include "stdsharp/concepts/concepts.h"

namespace stdsharp
{
    template<typename Allocation, typename Alloc>
    concept allocation_req = requires(
        Allocation allocation,
        const Allocation const_allocation,
        Alloc alloc,
        const Alloc const_alloc
    ) //
    {
        requires allocator_req<Alloc>;
        requires nothrow_default_initializable<Allocation>;
        requires boolean_testable<Allocation>;

        requires nothrow_copyable<Allocation>;

        allocation.deallocate(alloc);
        allocation.destroy(alloc); // clang-format off

        { const_allocation.has_value() } -> ::std::convertible_to<bool>;

        { const_allocation.size() } -> // clang-format on
                ::std::same_as<typename allocator_traits<Alloc>::size_type>;

        allocation.move_from(alloc, const_alloc, const_allocation);
        allocation.copy_from(alloc, const_alloc, const_allocation);
    };

    template<allocator_req Alloc, allocation_req<Alloc> Allocation>
    class basic_allocator_aware
    {
    public:
        using traits = allocator_traits<Alloc>;
        using allocator_type = Alloc;
        using allocation_t = Allocation;

    private:
        using const_alloc_ref = const allocator_type&;
        using this_t = basic_allocator_aware;

        allocator_type allocator_{};
        allocation_t allocation_{};

        constexpr void move_from(this_t&& other) noexcept( //
            noexcept(allocation_.move_from(allocator_, other.allocator_, other.allocation_))
        )
            requires requires //
        {
            allocation_.move_from(allocator_, other.allocator_, other.allocation_); //
        }
        {
            if(allocator_ == other.allocator_) allocation_ = other.allocation_;
            else allocation_.move_from(allocator_, other.allocator_, other.allocation_);
        }

        constexpr void copy_from(const this_t& other) noexcept( //
            noexcept(allocation_.copy_from(allocator_, other.allocator_, other.allocation_))
        )
            requires requires //
        {
            allocation_.copy_from(allocator_, other.allocator_, other.allocation_); //
        }
        {
            allocation_.copy_from(allocator_, other.allocator_, other.allocation_);
        }

    public:
        basic_allocator_aware() = default;

        constexpr basic_allocator_aware(const Alloc& alloc) noexcept: allocator_(alloc) {}

        constexpr basic_allocator_aware(const this_t& other, const Alloc& alloc) //
            noexcept(noexcept(copy_from(other)))
            requires requires { copy_from(other); }
            : allocator_(alloc)
        {
            copy_from(other);
        }

        constexpr basic_allocator_aware(this_t&& other, const Alloc& alloc) //
            noexcept(noexcept(move_from(other)))
            requires requires { move_from(other); }
            : allocator_(alloc)
        {
            move_from(other);
        }

        constexpr basic_allocator_aware(const this_t& other) //
            noexcept(nothrow_constructible_from<this_t, decltype(other), const_alloc_ref>)
            requires ::std::constructible_from<this_t, decltype(other), const_alloc_ref>
            : this_t(other, traits::copy_construct(other.allocator_))
        {
        }

        basic_allocator_aware(this_t&& other) noexcept = default;

        // TODO: implement

        basic_allocator_aware& operator=(const basic_allocator_aware& other)
        {
            if(this == &other) return;

            if(*this) destroy();

            allocation_.copy_from(allocator_, other.allocation_);

            return *this;
        }

        basic_allocator_aware& operator=(this_t&& other) //
            noexcept(noexcept(allocation_.move_assign(allocator_, other.allocation_)))
        {
            if(this == &other) return;

            if(*this) destroy();

            if(allocator_ == other.allocator_) allocation_ = other.allocation_;
            else allocation_.move_from(allocator_, other.allocation_);

            return *this;
        }

        constexpr ~basic_allocator_aware() { deallocate(); }

        [[nodiscard]] constexpr auto& allocator() const noexcept { return allocator_; }

        [[nodiscard]] constexpr auto& allocator() noexcept { return allocator_; }

        template<typename... Args>
            requires requires { allocation_.allocate(allocator_, ::std::declval<Args>()...); }
        constexpr void allocate(Args&&... args)
        {
            deallocate();
            allocation_.allocate(allocator_, ::std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires requires { allocation_.construct(allocator_, ::std::declval<Args>()...); }
        constexpr decltype(auto) construct(Args&&... args)
        {
            if(*this) destroy();

            return allocation_.construct(allocator_, ::std::forward<Args>(args)...);
        }

        template<typename T, typename... Args>
            requires requires //
        {
            allocation_.template construct<T>(allocator_, ::std::declval<Args>()...); //
        }
        constexpr decltype(auto) construct(Args&&... args)
        {
            if(*this) destroy();

            return allocation_.template construct<T>(allocator_, ::std::forward<Args>(args)...);
        }

        constexpr void destroy() noexcept
        {
            if(!*this) return;
            allocation_.destroy(allocator_);
        }

        constexpr void deallocate() noexcept
        {
            if(!*this) return;
            allocation_.deallocate(allocator_);
            allocation_ = {};
        }

        [[nodiscard]] constexpr explicit operator bool() const noexcept { return allocation_; }

        [[nodiscard]] constexpr auto size() const noexcept { return allocation_.size(); }

        [[nodiscard]] constexpr auto has_value() const noexcept { return allocation_.has_value(); }
    };
}