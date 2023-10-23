#pragma once

#include "../allocator_traits.h"
#include "../pointer_traits.h"

namespace stdsharp
{
    template<allocator_req Alloc>
    class [[nodiscard]] allocation
    {
    public:
        using allocator_type = Alloc;
        using pointer = allocator_pointer<allocator_type>;
        using size_type = allocator_size_type<allocator_type>;
        using value_type = allocator_value_type<allocator_type>;
        using const_pointer = allocator_const_pointer<allocator_type>;

    private:
        pointer ptr_ = nullptr;
        size_type size_ = 0;

    public:
        allocation() = default;

        constexpr allocation(const pointer ptr, const size_type size) noexcept:
            ptr_(ptr), size_(size)
        {
        }

        [[nodiscard]] constexpr auto begin() const noexcept { return ptr_; }

        [[nodiscard]] constexpr auto end() const noexcept { return ptr_ + size_; }

        [[nodiscard]] constexpr const_pointer cbegin() const noexcept { return begin(); }

        [[nodiscard]] constexpr const_pointer cend() const noexcept { return end(); }

        template<typename T = value_type>
        [[nodiscard]] constexpr decltype(auto) data() const noexcept
        {
            if constexpr(std::same_as<T, value_type>) return begin();
            else return pointer_cast<T>(begin());
        }

        template<typename T = value_type>
        [[nodiscard]] constexpr decltype(auto) cdata() const noexcept
        {
            if constexpr(std::same_as<T, value_type>) return cbegin();
            else return pointer_cast<T>(cbegin());
        }

        template<typename T = value_type>
        [[nodiscard]] constexpr T& get() const noexcept
        {
            Expects(!empty());
            return *data<T>();
        }

        template<typename T = value_type>
        [[nodiscard]] constexpr const T& cget() const noexcept
        {
            Expects(!empty());
            return *cdata<T>();
        }

        [[nodiscard]] constexpr auto size() const noexcept { return size_; }

        [[nodiscard]] constexpr auto empty() const noexcept { return ptr_ == nullptr; }
    };

    template<allocator_req Alloc>
    class [[nodiscard]] callocation : allocation<Alloc>
    {
        using allocation = allocation<Alloc>;
        using typename allocation::value_type;
        using allocation::allocation;
        using allocation::cbegin;
        using allocation::cend;
        using allocation::size;
        using allocation::empty;

        [[nodiscard]] constexpr auto begin() const noexcept { return allocation::cbegin(); }

        [[nodiscard]] constexpr auto end() const noexcept { return allocation::cend(); }

        template<typename T = value_type>
        [[nodiscard]] constexpr auto data() const noexcept
        {
            return allocation::template cdata<T>();
        }

        template<typename T = value_type>
        [[nodiscard]] constexpr T& get() const noexcept
        {
            return allocation::template cget<T>();
        }
    };

    template<typename Alloc, typename Rng>
    concept callocations_view = std::ranges::input_range<Rng> && //
        std::ranges::view<Rng> && //
        nothrow_convertible_to<std::ranges::range_value_t<Rng>, callocation<Alloc>>;

    template<typename Rng, typename Alloc>
    concept allocations_view = callocations_view<Alloc, Rng> &&
        nothrow_convertible_to<std::ranges::range_reference_t<Rng>, allocation<Alloc>&> &&
        std::ranges::output_range<Rng, allocation<Alloc>>;

    template<allocator_req Allocator, allocations_view<Allocator> Allocations>
    struct target_allocations
    {
        std::reference_wrapper<Allocator> allocator;
        Allocations allocations;
    };

    template<typename T, typename U>
    target_allocations(T&, U) -> target_allocations<T, U>;

    template<allocator_req Allocator, callocations_view<Allocator> Allocations>
    struct const_source_allocations
    {
        std::reference_wrapper<const Allocator> allocator;
        Allocations allocations;
    };

    template<typename T, typename U>
    const_source_allocations(T&, U) -> const_source_allocations<T, U>;

    template<allocator_req Allocator, allocations_view<Allocator> Allocations>
    struct source_allocations
    {
        std::reference_wrapper<Allocator> allocator;
        Allocations allocations;

        constexpr auto to_const() const noexcept
        {
            return const_source_allocations{
                allocator.get(), // TODO: replace with std::views::as_const
                std::ranges::subrange{
                    std::ranges::cbegin(allocations),
                    std::ranges::cend(allocations)
                }
            };
        }
    };

    template<typename T, typename U>
    source_allocations(T&, U) -> source_allocations<T, U>;

    template<allocator_req Allocator, std::invocable<Allocator> DeferAllocations>
        requires requires(std::invoke_result_t<DeferAllocations>, allocation<Allocator> result) {
            requires std::ranges::input_range<decltype(result)> &&
                nothrow_convertible_to<
                         std::ranges::range_reference_t<decltype(result)>,
                         allocation<Allocator>>;
        }
    struct ctor_input_allocation
    {
        Allocator allocator;
        DeferAllocations deferred_allocations;
    };

    template<typename T, typename U>
    ctor_input_allocation(T, U) -> ctor_input_allocation<T, U>;

    template<typename T, typename UDefer>
    ctor_input_allocation(T, UDefer) -> ctor_input_allocation<T, std::invoke_result<UDefer, T&>>;
}