#pragma once

#include <tuple>

#include "allocator_traits.h"
#include "pointer_traits.h"
#include "../scope.h"

// NOLINTBEGIN(*-reinterpret-cast, *-pointer-arithmetic)
namespace stdsharp
{
    template<::std::size_t I>
    class aggregate_bad_alloc : public ::std::bad_alloc
    {
        ::std::array<::std::exception_ptr, I> exceptions_;

    public:
        template<typename... Args>
            requires ::std::constructible_from<decltype(exceptions_), Args...>
        constexpr aggregate_bad_alloc(Args&&... args) noexcept:
            exceptions_{::std::forward<Args>(args)...}
        {
        }

        [[nodiscard]] constexpr const auto& exceptions() const noexcept { return exceptions_; }
    };

    namespace details
    {
        template<typename Allocator, typename... Allocators>
        concept allocator_composable = requires(
            allocator_traits<Allocator> first_traits,
            allocator_traits<Allocators>... traits
        ) //
        {
            requires all_same<
                typename decltype(first_traits)::value_type,
                typename decltype(traits)::value_type...>;

            pointer_traits<typename decltype(first_traits)::const_void_pointer>::to_pointer({}),
                (pointer_traits<typename decltype(traits)::const_void_pointer>::to_pointer({}),
                 ...);
        };

        template<::std::size_t I, typename... Allocators>
        constexpr void dispatch_deallocate( // NOLINT(*-exception-escape)
            ::std::tuple<Allocators...>& allocators,
            ::std::tuple_element_t<0, ::std::tuple<Allocators...>>* const ptr,
            const ::std::size_t count
        ) noexcept
        {
            using alloc_traits =
                allocator_traits<::std::tuple_element_t<I, ::std::tuple<Allocators...>>>;

            alloc_traits::deallocate(
                get<I>(allocators),
                pointer_traits<typename alloc_traits::pointer>::to_pointer(ptr),
                auto_cast(count)
            );
        }

        template<typename... Allocators, ::std::size_t... I>
        constexpr auto fallback_try_allocate_by(
            ::std::tuple<Allocators...>& allocators,
            const ::std::size_t count,
            const void* const hint,
            const ::std::index_sequence<I...> //
        ) noexcept
        {
            using value_type =
                ::std::tuple_element_t<0, ::std::remove_reference_t<decltype(allocators)>>;

            using deallocate_fn =
                void (*)(::std::tuple<Allocators...>&, value_type*, ::std::size_t);

            static constexpr ::std::array<deallocate_fn, sizeof...(Allocators)> deallocate_f{
                &dispatch_deallocate<I, Allocators...>... //
            };

            value_type* ptr = nullptr;
            ::std::size_t alloc_index = 0;

            empty = ( //
                ( //
                    alloc_index = I,
                    ptr = pointer_traits<typename allocator_traits<Allocators>::pointer>:: //
                    to_address( //
                        allocator_traits<Allocators>::try_allocate(
                            get<I>(allocators),
                            auto_cast(count),
                            pointer_traits<typename allocator_traits<
                                Allocators>::const_void_pointer>::to_pointer(hint)
                        )
                    ),
                    ptr != nullptr
                ) ||
                ...
            );

            return ::std::pair{ptr, deallocate_f[alloc_index]}; // NOLINT(*-constant-array-index)
        }

        template<typename... Allocators, ::std::size_t... I>
        constexpr auto fallback_allocate_by(
            ::std::tuple<Allocators...>& allocators,
            const ::std::size_t count,
            const void* const hint,
            const ::std::index_sequence<I...> //
        )
        {
            using value_type =
                ::std::tuple_element_t<0, ::std::remove_reference_t<decltype(allocators)>>;

            using deallocate_fn =
                void (*)(::std::tuple<Allocators...>&, value_type*, ::std::size_t);

            static constexpr ::std::array<deallocate_fn, sizeof...(Allocators)> deallocate_f{
                &dispatch_deallocate<I, Allocators...>... //
            };

            value_type* ptr = nullptr;
            ::std::size_t alloc_index = 0;
            ::std::array<::std::exception_ptr, sizeof...(Allocators)> exceptions;

            empty = ( //
                ( //
                    alloc_index = I,
                    [&ptr, &allocators, count, hint, &exceptions]
                    {
                        using alloc_traits = allocator_traits<Allocators>;

                        try
                        {
                            ptr = pointer_traits<typename alloc_traits::pointer>:: //
                                to_address( //
                                    alloc_traits::allocate(
                                        get<I>(allocators),
                                        auto_cast(count),
                                        pointer_traits<typename alloc_traits::const_void_pointer>::
                                            to_pointer(hint)
                                    )
                                );
                        }
                        catch(...)
                        {
                            exceptions[I] = ::std::current_exception();
                        }
                    }(),
                    ptr != nullptr
                ) ||
                ...
            );

            if(ptr == nullptr) throw aggregate_bad_alloc<sizeof...(I)>{::std::move(exceptions)};

            auto deallocator = [&allocators, ptr, count, alloc_index]() noexcept
            {
                deallocate_f[alloc_index](allocators, ptr, count); //
            };

            struct guard : scope::scoped<scope::exit_fn_policy::on_exit, decltype(deallocator)>
            {
            };

            return ::std::pair{ptr, deallocate_f[alloc_index]}; // NOLINT(*-constant-array-index)
        }
    }

    template<typename Allocator, typename... Allocators>
        requires details::allocator_composable<Allocator, Allocators...>
    [[nodiscard]] constexpr auto fallback_allocate_by(
        ::std::tuple<Allocator, Allocators>&... allocators,
        const ::std::size_t count,
        const void* const hint
    )
    {
        return details::fallback_allocate_by(
            allocators...,
            count,
            hint,
            ::std::make_index_sequence<1 + sizeof...(Allocators)>{}
        );
    }

    template<typename Allocator, typename... Allocators>
        requires details::allocator_composable<Allocator, Allocators...>
    [[nodiscard]] constexpr auto fallback_try_allocate_by(
        ::std::tuple<Allocator, Allocators>&... allocators,
        const ::std::size_t count,
        const void* const hint
    ) noexcept
    {
        return details::fallback_try_allocate_by(
            allocators...,
            count,
            hint,
            ::std::make_index_sequence<1 + sizeof...(Allocators)>{}
        );
    }
}

// NOLINTEND(*-reinterpret-cast, *-pointer-arithmetic)