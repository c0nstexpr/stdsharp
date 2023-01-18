#pragma once

#include "allocator_traits.h"
#include "pointer_traits.h"
#include "../default_operator.h"
#include "../functional/operations.h"
#include "../type_traits/function.h"
#include "../cstdint/cstdint.h"
#include <__type_traits/is_constant_evaluated.h>

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
        template<typename T, typename Base, typename... Allocators>
        class composed_allocator : Base
        {
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
            template<::std::size_t I>
            static constexpr byte* set_alloc_meta(const alloc_pointer<I>& ptr) noexcept
            {
                byte* const raw_ptr = pointer_traits<alloc_pointer<I>>::to_address(ptr);

                *reinterpret_cast<::std::size_t*>(raw_ptr) = I;

                return raw_ptr + sizeof(::std::size_t);
            }

            template<::std::size_t I>
            constexpr byte* raw_allocate_at(
                const alloc_size_type<I> count,
                const alloc_const_void_pointer<I>& hint
            ) noexcept
            {
                auto& alloc = get<I>(*this);
                auto&& ptr = alloc_traits<I>::try_allocate(alloc, count, hint);

                return ptr == nullptr ? nullptr : set_alloc_meta<I>(ptr);
            }

            template<::std::size_t I>
            constexpr byte* raw_allocate_at( // NOLINT(*-exception-escape)
                const alloc_size_type<I> count,
                const alloc_const_void_pointer<I>& hint,
                ::std::exception_ptr& exception
            ) noexcept
            {
                alloc_pointer<I> ptr = nullptr;

                if(::std::is_constant_evaluated())
                {
                    auto ptr = raw_allocate_at<I>(count, hint);
                    if(ptr == nullptr) exception = ::std::make_exception_ptr(::std::bad_alloc{});

                    return ptr;
                }

                try
                {
                    ptr = alloc_traits<I>::allocate(get<I>(*this), count, hint);

                    return set_alloc_meta<I>(ptr);
                }
                catch(...)
                {
                    if(ptr != nullptr) alloc_traits<I>::deallocate(get<I>(*this), ptr, count);
                    exception = ::std::current_exception();
                }


                return nullptr;
            }

            static constexpr ::std::pair<byte*, ::std::size_t> get_alloc_meta(byte* ptr)
            {
                ptr -= sizeof(::std::size_t);
                return {ptr, *reinterpret_cast<const ::std::size_t*>(ptr)};
            }

            template<::std::size_t I>
            static constexpr auto to_alloc_void_pointer(const void* const ptr) noexcept
            {
                return pointer_traits<alloc_const_void_pointer<I>>::to_pointer(ptr);
            }

            template<bool Noexcept, ::std::size_t... I>
            constexpr byte* raw_allocate_impl(
                const ::std::index_sequence<I...>,
                ::std::size_t count,
                const void* const hint
            ) noexcept(Noexcept)
            {
                byte* ptr = nullptr;

                count += sizeof(::std::size_t);

                if constexpr(Noexcept)
                {
                    empty = ( //
                        ( //

                            ptr = raw_allocate_at<I>(
                                auto_cast(count),
                                to_alloc_void_pointer<I>(hint)
                            ),
                            ptr != nullptr
                        ) ||
                        ...
                    );
                }
                else
                {
                    ::std::array<::std::exception_ptr, sizeof...(I)> exceptions{};

                    const auto alloc_f = [this]<::std::size_t J>( // clang-format off
                    const index_constant<J>,
                    const alloc_size_type<J> count,
                    const void* const hint,
                    ::std::exception_ptr& exception
                ) noexcept -> byte* // clang-format on
                    {
                        try
                        {
                            return raw_allocate_at<J>(
                                count,
                                to_alloc_void_pointer<J>(hint),
                                exception
                            );
                        }
                        catch(...)
                        {
                            exception = ::std::current_exception();
                        }

                        return nullptr;
                    };

                    const bool res = ( //
                        ( //
                            ptr =
                                alloc_f(index_constant<I>{}, auto_cast(count), hint, exceptions[I]),
                            static_cast<bool>(exceptions[I])
                        ) &&
                        ...
                    );

                    if(res) throw aggregate_bad_alloc<exceptions.size()>{::std::move(exceptions)};
                }

                return ptr;
            }

            template<::std::size_t... I>
            constexpr void deallocate_impl(
                const ::std::index_sequence<I...>,
                const ::std::size_t alloc_index,
                byte* const ptr,
                const ::std::size_t count
            ) noexcept
            {
                using deallocate_fn = void (*)(composed_allocator&, byte*, ::std::size_t);

                static constexpr ::std::array<deallocate_fn, sizeof...(I)> deallocate_f = {
                    +[]( // NOLINT(*-exception-escape)
                         composed_allocator& instance,
                         byte* const ptr,
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
                return ((get<I>(*this) == get<I>(other)) && ...);
            }

            template<::std::size_t... I>
                requires is_always_equal::value
            constexpr bool equal_impl(const ::std::index_sequence<I...>, const composed_allocator&)
                const noexcept
            {
                return true;
            }

            friend constexpr bool
                operator==(const composed_allocator& left, const composed_allocator& right) noexcept
            {
                return left.equal_impl(::std::index_sequence_for<Allocators...>{}, right);
            };

        public:
            using Base::Base; // clang-format on

            constexpr void swap(composed_allocator& other) noexcept
            {
                ::std::ranges::swap(static_cast<Base&>(*this), static_cast<Base&>(other));
            }

            [[nodiscard]] T* allocate(const ::std::size_t count, const void* const hint = nullptr)
            {
                return ::std::is_constant_evaluated() ?
                    try_allocate(count, hint) :
                    reinterpret_cast<T*>( //
                        raw_allocate_impl<false>(
                            ::std::index_sequence_for<Allocators...>{},
                            count * sizeof(T),
                            hint
                        )
                    );
            }

            // allocate the memory in sequence of allocators
            [[nodiscard]] constexpr T*
                try_allocate(const ::std::size_t count, const void* const hint = nullptr) noexcept
            {
                const auto ptr = raw_allocate_impl<true>(
                    ::std::index_sequence_for<Allocators...>{},
                    count * sizeof(T),
                    hint
                );

                if(::std::is_constant_evaluated())
                {
                    // TODO
                    // constexpr byte* null_ptr = nullptr;

                    // const auto diff = ptr - null_ptr;

                    // return static_cast<T*>(nullptr) + diff / sizeof(T);
                }

                return reinterpret_cast<T*>(ptr);
            }

            constexpr void deallocate(T* const ptr, const ::std::size_t count) noexcept
            {
                const auto [src_ptr, index] = get_alloc_meta(reinterpret_cast<byte*>(ptr));

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
        indexed_values<typename allocator_traits<Allocators>::template rebind_alloc<byte>...>,
        Allocators... // clang-format off
    >; // clang-format on
}

// NOLINTEND(*-reinterpret-cast, *-pointer-arithmetic)