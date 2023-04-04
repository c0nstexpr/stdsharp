#pragma once

#include <numeric>

#include "allocator_traits.h"
#include "../cassert/cassert.h"
#include "../containers/actions.h"

namespace stdsharp
{
    template<
        allocator_req Alloc,
        sequence_container Allocations =
            ::std::vector<typename allocator_traits<Alloc>::allocation> // clang-format off
        > // clang-format on
        requires requires(
            Allocations allocations,
            typename Allocations::value_type value,
            typename allocator_traits<Alloc>::allocation allocation
        ) { requires ::std::same_as<decltype(value), decltype(allocation)>; }
    class basic_allocator_aware
    {
    public:
        using traits = allocator_traits<Alloc>;
        using allocator_type = Alloc;
        using allocations_type = Allocations;
        using allocation = typename allocations_type::value_type;
        using size_type = typename traits::size_type;

    private:
        using iter = typename allocations_type::iterator;
        using const_iter = typename allocations_type::const_iterator;
        using const_alloc_ref = const allocator_type&;
        using this_t = basic_allocator_aware;

        allocator_type allocator_{};
        allocations_type allocations_{};

    public:
        basic_allocator_aware() = default;

        constexpr basic_allocator_aware(const Alloc& alloc) noexcept: allocator_(alloc) {}

        constexpr basic_allocator_aware(const Alloc& alloc) //
            noexcept(nothrow_constructible_from<allocations_type, allocator_type>)
            requires ::std::constructible_from<allocations_type, allocator_type>
            : allocator_(alloc), allocations_(make_obj_uses_allocator<allocations_type>(alloc))
        {
        }

        // TODO: implement

        constexpr basic_allocator_aware(const this_t& other, const Alloc& alloc) //
            noexcept(noexcept(copy_from(other)))
            requires requires { copy_from(other); }
            : allocator_(alloc)
        {
        }

        constexpr basic_allocator_aware(this_t&& other, const Alloc& alloc) //
            noexcept(noexcept(move_from(other)))
            requires requires { move_from(other); }
            : allocator_(alloc)
        {
        }

        constexpr basic_allocator_aware(const this_t& other) //
            noexcept(nothrow_constructible_from<this_t, decltype(other), const_alloc_ref>)
            requires ::std::constructible_from<this_t, decltype(other), const_alloc_ref>
            : this_t(other, traits::copy_construct(other.allocator_))
        {
        }

        basic_allocator_aware(this_t&& other) noexcept = default;

        basic_allocator_aware& operator=(const basic_allocator_aware& other)
        {
            if(this == &other) return;

            destroy();

            allocations_.copy_from(allocator_, other.allocations_);

            return *this;
        }

        basic_allocator_aware& operator=(this_t&& other) //
            noexcept(noexcept(allocations_.move_assign(allocator_, other.allocations_)))
        {
            if(this == &other) return;

            destroy();

            if(allocator_ == other.allocator_) allocations_ = other.allocations_;
            else allocations_.move_from(allocator_, other.allocations_);

            return *this;
        }

        constexpr ~basic_allocator_aware() { deallocate(); }

        template<::std::invocable<allocations_type&, allocation> Func = actions::emplace_back_fn>
        [[nodiscard]] constexpr const auto& allocate(const size_type size, Func&& func = {})
        {
            ::std::invoke(func, allocations_, traits::get_allocation(allocator_, size));
            return ::ranges::back(allocations_);
        }

        [[nodiscard]] constexpr const auto& reallocate(const const_iter iter, const size_type size)
        {
            if constexpr(is_debug)
                if(::std::ranges::distance(iter, allocations_.cbegin()) >= allocations_.size())
                    throw std::out_of_range{"iterator out of range"};

            auto& value = const_cast<allocation&>(*iter); // NOLINT(*-const-cast)
            value.deallocate(allocator_);
            value = traits::get_allocation(allocator_, size);
        }

        constexpr void deallocate(const const_iter iter) noexcept
        {
            iter->deallocate(allocator_);
            actions::cpo::erase(allocations_, iter);
        }

        constexpr void deallocate(const const_iter begin, const const_iter end) noexcept
        {
            for(auto it = begin; it != end; ++it) it->deallocate(allocator_);

            actions::cpo::erase(begin, end);
        }

        constexpr void deallocate() noexcept
        {
            deallocate(allocations_.cbegin(), allocations_.cend());
            allocations_.clear();
        }

        [[nodiscard]] constexpr auto& allocator() const noexcept { return allocator_; }

        [[nodiscard]] constexpr auto& allocator() noexcept { return allocator_; }

        [[nodiscard]] constexpr explicit operator bool() const noexcept
        {
            return ::std::ranges::any_of(allocations_);
        }

        [[nodiscard]] constexpr auto size() const noexcept
        {
            return ::std::accumulate(
                allocations_.cbegin(),
                allocations_.cend(),
                [](const allocation& v) noexcept { return v.size; }
            );
        }

        [[nodiscard]] constexpr auto has_value() const noexcept { return allocations_.has_value(); }

        [[nodiscard]] constexpr auto& allocations() const noexcept { return allocations_; }
    };
}