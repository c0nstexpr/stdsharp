#pragma once

#include "allocator_traits.h"
#include "pointer_traits.h"
#include "../default_operator.h"
#include "../functional/operations.h"
#include "../type_traits/function.h"
#include "../cstdint/cstdint.h"

// NOLINTBEGIN(*-reinterpret-cast, *-pointer-arithmetic)
namespace stdsharp
{
    template<::std::size_t I>
    class aggregate_bad_alloc : public ::std::bad_alloc
    {
        ::std::array<::std::exception_ptr, I> exceptions_;

    public:
        template<typename... Args>
        constexpr aggregate_bad_alloc(Args&&... args) noexcept:
            exceptions_{::std::forward<Args>(args)...}
        {
        }

        [[nodiscard]] constexpr const auto& exceptions() const noexcept { return exceptions_; }
    };

    template<::std::size_t I>
    aggregate_bad_alloc(const ::std::array<::std::exception_ptr, I>&) -> aggregate_bad_alloc<I>;

    template<::std::size_t I>
    aggregate_bad_alloc(::std::array<::std::exception_ptr, I>&&) -> aggregate_bad_alloc<I>;

    template<typename T, allocator_req... Allocators>
        requires requires(
            allocator_traits<Allocators>... traits,
            typename decltype(traits)::template rebind_alloc<byte>... byte_alloc,
            allocator_traits<decltype(byte_alloc)>... byte_traits
        ) //
    { requires(allocator_req<decltype(byte_alloc)> && ...); }
    class composed_allocator :
        indexed_values<typename allocator_traits<Allocators>::template rebind_alloc<byte>...>
    {
        using base =
            indexed_values<typename allocator_traits<Allocators>::template rebind_alloc<byte>...>;

        template<::std::size_t I>
        using alloc_t = typename base::template type<I>;

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

    private:
        template<::std::size_t... I>
        constexpr void
            copy_assign_impl(const composed_allocator& other, const ::std::index_sequence<I...>) noexcept
        {
            const auto f = []<::std::size_t J>( // clang-format off
                const index_constant<J>,
                composed_allocator& instance,
                const composed_allocator& other
            ) // clang-format on
            {
                if constexpr(alloc_traits<J>::propagate_on_container_copy_assignment::value)
                    get<J>(instance) = get<J>(other);
            };

            (f(index_constant<I>{}, *this, other), ...);
        }

        template<::std::size_t... I>
        constexpr void
            move_assign_impl(composed_allocator&& other, const ::std::index_sequence<I...>) noexcept
        {
            const auto f = []<::std::size_t J>( // clang-format off
                const index_constant<J>,
                composed_allocator& instance,
                composed_allocator& other
            ) // clang-format on
            {
                if constexpr(alloc_traits<J>::propagate_on_container_move_assignment::value)
                    get<J>(instance) = ::std::move(get<J>(other));
            };

            (f(index_constant<I>{}, *this, other), ...);
        }

        template<::std::size_t... I>
        constexpr void
            swap_impl(composed_allocator& other, const ::std::index_sequence<I...>) noexcept
        {
            const auto f = []<::std::size_t J>( // clang-format off
                const index_constant<J>,
                composed_allocator& instance,
                composed_allocator& other
            ) // clang-format on
            {
                if constexpr(alloc_traits<J>::propagate_on_container_swap::value)
                    ::std::ranges::swap(get<J>(instance), get<J>(other));
            };

            (f(index_constant<I>{}, *this, other), ...);
        }

        template<::std::size_t I>
        static constexpr byte* assign_alloc_index(const alloc_pointer<I>& ptr)
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

            try
            {
                if(ptr != nullptr) return assign_alloc_index<I>(ptr);
            }
            catch(...)
            {
                alloc_traits<I>::deallocate(alloc, ptr, count);
            }

            return nullptr;
        }

        template<::std::size_t I>
        constexpr byte* raw_allocate_at(
            const alloc_size_type<I> count,
            const alloc_const_void_pointer<I>& hint,
            ::std::exception_ptr& exception
        ) noexcept
        {
            bool allocated = false;
            auto& alloc = get<I>(*this);
            alloc_pointer<I> ptr;

            try
            {
                ptr = alloc_traits<I>::allocate(alloc, count, hint);
                allocated = true;
                return assign_alloc_index<I>(ptr);
            }
            catch(...)
            {
                if(allocated) alloc_traits<I>::deallocate(alloc, ptr, count);
                exception = ::std::current_exception();
            }

            return nullptr;
        }

        static constexpr ::std::pair<byte*, ::std::size_t> get_alloc_index(byte* ptr)
        {
            ptr -= sizeof(::std::size_t);
            return {ptr, *reinterpret_cast<const ::std::size_t*>(ptr)};
        }

        template<::std::size_t I>
        static constexpr alloc_const_void_pointer<I> to_alloc_void_pointer(const void* const ptr) //
            noexcept
        {
            return auto_cast(pointer_traits<alloc_const_pointer<I>>::to_pointer(ptr));
        }

        template<::std::size_t I, typename Traits = pointer_traits<alloc_const_void_pointer<I>>>
            requires requires { Traits::to_pointer({}); }
        static constexpr alloc_const_void_pointer<I> to_alloc_void_pointer(const void* const ptr) //
            noexcept
        {
            return Traits::to_pointer(ptr);
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
                try
                {
                    ( //
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
                catch(...)
                {
                }
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
                        return raw_allocate_at<J>(count, to_alloc_void_pointer<J>(hint), exception);
                    }
                    catch(...)
                    {
                        exception = ::std::current_exception();
                    }

                    return nullptr;
                };

                const bool res = ( //
                    ( //
                        ptr = alloc_f(index_constant<I>{}, auto_cast(count), hint, exceptions[I]),
                        static_cast<bool>(exceptions[I])
                    ) &&
                    ...
                );

                if(res) throw aggregate_bad_alloc{::std::move(exceptions)};
            }

            return ptr;
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
        constexpr bool
            equal_impl(const ::std::index_sequence<I...>, const composed_allocator&) const noexcept
        {
            return true;
        }

        friend constexpr bool
            operator==(const composed_allocator& left, const composed_allocator& right) noexcept
        {
            return left.equal_impl(::std::index_sequence_for<Allocators...>{}, right);
        };

    public:
        using base::base;

        composed_allocator() = default;
        ~composed_allocator() = default;
        composed_allocator(const composed_allocator&) = default;
        composed_allocator(composed_allocator&&) noexcept = default;

        constexpr composed_allocator& operator=(const composed_allocator& other) noexcept
            requires(propagate_on_container_copy_assignment::value)
        {
            copy_assign_impl(other, ::std::index_sequence_for<Allocators...>{});
            return *this;
        }

        composed_allocator& operator=(const composed_allocator& other) = default;

        constexpr composed_allocator& operator=(composed_allocator&& other) noexcept
            requires(propagate_on_container_move_assignment::value)
        {
            move_assign_impl(::std::move(other), ::std::index_sequence_for<Allocators...>{});
            return *this;
        }

        composed_allocator& operator=(composed_allocator&& other) noexcept = default;

        constexpr void swap(composed_allocator& other) noexcept
            requires(propagate_on_container_swap::value)
        {
            swap_impl(other, ::std::index_sequence_for<Allocators...>{});
        }

        constexpr void swap(composed_allocator& other) noexcept
        {
            ::std::ranges::swap(static_cast<base&>(*this), static_cast<base&>(other));
        }

        // allocate the memory in sequence of allocators
        [[nodiscard]] constexpr T*
            allocate(const ::std::size_t count, const void* const hint = nullptr)
        {
            return reinterpret_cast<T*>( //
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
            return reinterpret_cast<T*>( //
                raw_allocate_impl<true>(
                    ::std::index_sequence_for<Allocators...>{},
                    count * sizeof(T),
                    hint
                )
            );
        }

        constexpr void deallocate(T* const ptr, const ::std::size_t count) noexcept
        {
            const auto index = get_alloc_index(reinterpret_cast<byte*>(ptr));
            const auto& void_p = alloc_info.ptr;

            count += sizeof(::std::size_t);

            {
                get<index>(*this).deallocate(auto_cast(ptr), count * sizeof(T));
            }
        }

        [[nodiscard]] constexpr auto max_size() const noexcept {}
    };
}

// NOLINTEND(*-reinterpret-cast, *-pointer-arithmetic)