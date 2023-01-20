#pragma once

#include "allocator_traits.h"
#include "pointer_traits.h"
#include "../cstdint/cstdint.h"
#include "../cmath/cmath.h"
#include <__tuple>

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

    namespace details
    {
        template<typename T, typename Meta>
        union byte_like
        {
            ::std::array<T, ceil_reminder(sizeof(Meta), sizeof(T))> t;
            Meta index;
        };

        template<typename T, typename Base, typename... Allocators>
        class composed_allocator : Base
        {
            using data = byte_like<T>;

            using deallocate_fn = void (*)(::std::tuple<Allocators...>&, data*, ::std::size_t);

            template<::std::size_t I>
            using alloc_t = typename Base::template type<I>;

            template<::std::size_t I>
            using alloc_traits = allocator_traits<alloc_t<I>>;

            template<::std::size_t I>
            using alloc_pointer = typename alloc_traits<I>::pointer;

            template<::std::size_t I>
            using alloc_const_pointer = typename alloc_traits<I>::const_pointer;

            template<::std::size_t I>
            using alloc_const_void_pointer = typename alloc_traits<I>::const_void_pointer;

            template<::std::size_t I>
            using alloc_size_type = typename alloc_traits<I>::size_type;

        public:
            using value_type = T;

            using propagate_on_container_copy_assignment = ::std::disjunction<
                typename allocator_traits<Allocators>::propagate_on_container_copy_assignment...>;

            using propagate_on_container_move_assignment = ::std::disjunction<
                typename allocator_traits<Allocators>::propagate_on_container_move_assignment...>;

            using propagate_on_container_swap = ::std::disjunction<
                typename allocator_traits<Allocators>::propagate_on_container_swap...>;

            using is_always_equal =
                ::std::conjunction<typename allocator_traits<Allocators>::is_always_equal...>;

            template<typename U>
            struct rebind
            {
                using other = composed_allocator<
                    U,
                    typename allocator_traits<Allocators>::template rebind_alloc<U>...>;
            };

            [[nodiscard]] constexpr const Base& get_allocators() const noexcept { return *this; }

        private:
            static constexpr data*
                set_alloc_meta(data* const ptr, const ::std::size_t alloc_index) noexcept
            {
                ptr->index = alloc_index;
                return ptr + 1;
            }

            static constexpr ::std::pair<data*, ::std::size_t> get_alloc_meta(data* ptr)
            {
                ptr -= 1;
                return {ptr, ptr->index};
            }

            template<::std::size_t... I>
            constexpr void deallocate_impl(
                const ::std::index_sequence<I...>,
                const ::std::size_t alloc_index,
                data* const ptr,
                const ::std::size_t count
            ) noexcept
            {
                using deallocate_fn = void (*)(composed_allocator&, data*, ::std::size_t);

                static constexpr ::std::array<deallocate_fn, sizeof...(I)> deallocate_f = {
                    +[]( // NOLINT(*-exception-escape)
                         composed_allocator& instance,
                         data* const ptr,
                         const ::std::size_t count
                     ) noexcept
                    {
                        alloc_traits<I>::deallocate(
                            get<I>(instance),
                            pointer_traits<alloc_pointer<I>>::to_pointer(ptr),
                            auto_cast(count)
                        );
                    }... //
                };

                deallocate_f[alloc_index](*this, ptr, count); // NOLINT(*-constant-array-index)
            }

            template<::std::size_t... I>
            [[nodiscard]] constexpr auto
                max_size_impl(const ::std::index_sequence<I...>) const noexcept
            {
                return ::std::max( //
                    ::std::initializer_list{
                        static_cast<::std::size_t>(alloc_traits<I>::max_size(get<I>(*this)))... //
                    }
                );
            }

            template<::std::size_t... I>
            constexpr bool
                equal_impl(const ::std::index_sequence<I...>, const composed_allocator& other)
                    const noexcept
            {
                if constexpr(is_always_equal::value) return true;
                else return ((get<I>(*this) == get<I>(other)) && ...);
            }

            friend constexpr bool
                operator==(const composed_allocator& left, const composed_allocator& right) noexcept
            {
                return left.equal_impl(::std::index_sequence_for<Allocators...>{}, right);
            };

            static constexpr auto get_data_count(const ::std::size_t t_count) noexcept
            {
                return ceil_reminder(t_count * sizeof(T) + sizeof(::std::size_t), sizeof(data));
            }

            static constexpr auto get_value_ptr(data* ptr) noexcept { return ptr->t.data(); }

            static /* TODO: constexpr */ auto get_data_ptr(T* ptr) noexcept
            {
                // conversion from pointer to void to any pointer-to-object type is not
                // constexpr
                return
                    // ::std::is_constant_evaluated() ?
                    //     static_cast<data*>(static_cast<void*>(ptr)) :
                    reinterpret_cast<data*>(ptr);
            }

        public:
            using Base::Base; // clang-format on

            constexpr void swap(composed_allocator& other) noexcept
            {
                ::std::ranges::swap(static_cast<Base&>(*this), static_cast<Base&>(other));
            }

            [[nodiscard]] T* allocate(const ::std::size_t count, const void* const hint = nullptr)
            {
                const auto [ptr, alloc_index] =
                    fallback_allocate_by(*this, get_data_count(count), hint);
                return get_value_ptr(set_alloc_meta(ptr, alloc_index));
            }

            [[nodiscard]] constexpr T*
                try_allocate(const ::std::size_t count, const void* const hint = nullptr) noexcept
            {
                return get_value_ptr(raw_allocate_impl<true>(
                    ::std::index_sequence_for<Allocators...>{},
                    get_data_count(count),
                    hint
                ));
            }

            constexpr void deallocate(T* const ptr, const ::std::size_t count) noexcept
            {
                const auto [src_ptr, index] = get_alloc_meta(get_data_ptr(ptr));

                deallocate_impl(::std::index_sequence_for<Allocators...>{}, index, src_ptr, count);
            }

            [[nodiscard]] constexpr auto max_size() const noexcept
            {
                return max_size_impl(::std::index_sequence_for<Allocators...>{});
            }
        };
    }

    template<typename T, typename... Allocators>
        requires requires(
                     allocator_traits<Allocators>... traits,
                     typename decltype(traits)::template rebind_alloc<byte>... byte_alloc,
                     allocator_traits<decltype(byte_alloc)>... byte_traits
                 ) //
    {
        requires all_same<T, typename decltype(traits)::value_type...>;

        (pointer_traits<typename decltype(byte_traits)::const_void_pointer>::to_pointer({}), ...);
    }
    using composed_allocator = details::composed_allocator<
        T,
        indexed_values<typename allocator_traits<Allocators>:: // clang-format off
        template rebind_alloc<details::byte_like<T>>...>,
        Allocators...
    >; // clang-format on
}

// NOLINTEND(*-reinterpret-cast, *-pointer-arithmetic)