#pragma once

#include "allocator_traits.h"
#include "pointer_traits.h"
#include "../tuple/tuple.h"
#include "../scope.h"

namespace stdsharp::scope
{
    namespace details
    {
        template<
            typename Deallocator,
            typename Base = scoped<
                exit_fn_policy::on_exit,
                ref_invocable_t<Deallocator> // clang-format off
            >
        > // clang-format on
        struct raii_memory : Deallocator, Base
        {
            template<typename... T>
            constexpr raii_memory(T&&... t) noexcept:
                Deallocator(::std::forward<T>(t)...), Base(*this)
            {
            }
        };
    }

    template<typename Alloc>
        requires requires //
    {
        pointer_traits<typename allocator_traits<Alloc>::pointer>::to_pointer({}); //
    }
    [[nodiscard]] constexpr auto make_scoped_memory(
        Alloc& alloc,
        typename allocator_traits<Alloc>::value_type* const ptr,
        const typename allocator_traits<Alloc>::size_type count
    ) noexcept
    {
        using alloc_traits = allocator_traits<Alloc>;

        class deallocator
        {
        public:
            constexpr deallocator(Alloc& alloc, decltype(ptr) ptr, decltype(count) count) noexcept:
                alloc_(alloc), ptr_(ptr), count_(count)
            {
            }

            constexpr void operator()() const noexcept // NOLINT(*-exception-escape)
            {
                alloc_traits::deallocate(
                    alloc_,
                    pointer_traits<typename alloc_traits::pointer>::to_pointer(ptr_),
                    count_
                );
            }

            [[nodiscard]] constexpr auto ptr() const noexcept { return ptr_; }

            [[nodiscard]] constexpr auto count() const noexcept { return count_; }

            [[nodiscard]] constexpr auto& allocator() const noexcept { return alloc_.get(); }

        private:
            ::std::reference_wrapper<Alloc> alloc_;
            typename Alloc::value_type* ptr_;
            typename alloc_traits::size_type count_;
        };

        return details::raii_memory<deallocator>{alloc, ptr, count};
    }

    template<typename Alloc>
    [[nodiscard]] constexpr auto allocate(
        Alloc& alloc,
        const typename allocator_traits<Alloc>::size_type count,
        const typename allocator_traits<Alloc>::const_void_pointer hint = nullptr
    )
    {
        return make_scoped_memory(
            alloc,
            allocator_traits<Alloc>::allocate(alloc, count, hint),
            count
        );
    }

    template<typename Alloc>
    [[nodiscard]] constexpr auto try_allocate(
        Alloc& alloc,
        const typename allocator_traits<Alloc>::size_type count,
        const typename allocator_traits<Alloc>::const_void_pointer hint = nullptr
    ) noexcept
    {
        return make_scoped_memory(
            alloc,
            allocator_traits<Alloc>::try_allocate(alloc, count, hint),
            count
        );
    }

    template<typename Alloc>
    using memory_t = decltype(allocate(::std::declval<Alloc&>(), 0, nullptr));
}