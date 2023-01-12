#pragma once
#include <exception>
#include <variant>

#include "allocator_traits.h"
#include "../default_operator.h"
#include "../functional/operations.h"
#include "../type_traits/function.h"

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
            typename decltype(traits)::pointer... pointers,
            typename ::std::common_type_t<decltype(pointers)...> common_p,
            typename decltype(traits)::void_pointer... void_pointers,
            typename ::std::common_type_t<decltype(void_pointers)...> common_vp,
            typename decltype(traits)::const_pointer... const_pointers,
            typename ::std::common_type_t<decltype(const_pointers)...> common_cp,
            typename decltype(traits)::difference_type... difference_values,
            typename ::std::common_type_t<decltype(difference_values)...> common_diff,
            typename decltype(traits)::size_type... size_values,
            typename ::std::common_type_t<decltype(size_values)...> common_size
        ) //
    {
        requires(::std::convertible_to<decltype(common_vp), decltype(void_pointers)> && ...);
        requires(::std::convertible_to<decltype(common_diff), decltype(difference_values)> && ...);
        requires(::std::convertible_to<decltype(common_size), decltype(size_values)> && ...);

        requires inter_convertible<decltype(common_p), decltype(common_vp)>;
    }
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
        using alloc_void_pointer = typename alloc_traits<I>::void_pointer;

        template<::std::size_t I>
        using alloc_const_void_pointer = typename alloc_traits<I>::const_void_pointer;

        template<::std::size_t I>
        using alloc_difference_type = typename alloc_traits<I>::difference_type;

        template<::std::size_t I>
        using alloc_size_type = typename alloc_traits<I>::size_type;

    public:
        using value_type = T;

        using difference_type =
            ::std::common_type_t<typename allocator_traits<Allocators>::difference_type...>;

        using size_type = ::std::common_type_t<typename allocator_traits<Allocators>::size_type...>;

        using pointer = ::std::common_type_t<typename allocator_traits<Allocators>::pointer...>;

        using void_pointer =
            ::std::common_type_t<typename allocator_traits<Allocators>::void_pointer...>;

        using const_pointer =
            ::std::common_type_t<typename allocator_traits<Allocators>::const_pointer...>;

        using const_void_pointer =
            ::std::common_type_t<typename allocator_traits<Allocators>::const_void_pointer...>;

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
        using size_alloc_traits = typename alloc_traits<I>::template rebind_traits<::std::size_t>;

        template<
            ::std::size_t I,
            typename SizePointer = typename size_alloc_traits<I>::pointer>
        constexpr alloc_void_pointer<I> assign_alloc_index(const alloc_void_pointer<I>& ptr) //
            noexcept(noexcept(*(static_cast<SizePointer>(ptr)) = I, ptr + sizeof(::std::size_t)))
        {
            *(static_cast<SizePointer>(ptr)) = I;

            return ptr + sizeof(::std::size_t);
        }

        template<::std::size_t I>
        constexpr alloc_void_pointer<I> raw_allocate_at(
            const alloc_size_type<I> count,
            const alloc_const_void_pointer<I>& hint
        ) noexcept
        {
            auto&& ptr = auto_cast(alloc_traits<I>::try_allocate(get<I>(*this), count, hint));

            if(ptr != nullptr) return assign_alloc_index<I>(::std::move(ptr));

            return {nullptr};
        }

        template<::std::size_t I>
        constexpr alloc_void_pointer<I> raw_allocate_at(
            const alloc_size_type<I> count,
            const alloc_const_void_pointer<I>& hint,
            ::std::exception_ptr& exception
        ) noexcept
        {
            try
            {
                return assign_alloc_index<I>(
                    ::std::move(auto_cast(alloc_traits<I>::allocate(get<I>(*this), count, hint)))
                );
            }
            catch(...)
            {
                exception = ::std::current_exception();

                return {nullptr};
            }
        }

        constexpr auto get_alloc_info(const void_pointer& ptr) //
            noexcept(noexcept(ptr + sizeof(::std::size_t)))
        {
            using ConstSizePointer = typename size_alloc_traits<I>::const_pointer;

            struct local
            {
                void_pointer ptr;

                [[nodiscard]] constexpr ::std::size_t get_alloc_index() const
                    noexcept(noexcept(*(static_cast<ConstSizePointer>(ptr))))
                {
                    return *(static_cast<ConstSizePointer>(ptr));
                }
            };

            return local{ptr - sizeof(::std::size_t)};
        }

        template<bool Noexcept, ::std::size_t... I>
        constexpr void_pointer raw_allocate_impl(
            const ::std::index_sequence<I...>,
            size_type count,
            const const_void_pointer& hint
        ) noexcept(Noexcept)
        {
            void_pointer ptr{nullptr};

            count += sizeof(::std::size_t);

            if constexpr(Noexcept)
            {
                try
                {
                    ( //
                        ( //
                            empty =
                                (ptr =
                                     auto_cast(raw_allocate_at<I>(auto_cast(count), auto_cast(hint))
                                     )),
                            static_cast<bool>(ptr != nullptr)
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
                    const const_void_pointer& hint,
                    ::std::exception_ptr& exception
                ) noexcept -> void_pointer // clang-format on
                {
                    try
                    {
                        return auto_cast(raw_allocate_at<J>(count, auto_cast(hint), exception));
                    }
                    catch(...)
                    {
                        exception = ::std::current_exception();
                    }

                    return {nullptr};
                };

                const bool res = ( //
                    ( //
                        empty = ( //
                            ptr =
                                alloc_f(index_constant<I>{}, auto_cast(count), hint, exceptions[I])
                        ),
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
        [[nodiscard]] constexpr pointer
            allocate(const size_type count, const const_void_pointer& hint = nullptr)
        {
            return auto_cast( //
                raw_allocate_impl<false>(
                    ::std::index_sequence_for<Allocators...>{},
                    count * sizeof(T),
                    hint
                )
            );
        }

        // allocate the memory in sequence of allocators
        [[nodiscard]] constexpr pointer
            try_allocate(const size_type count, const const_void_pointer& hint = nullptr) noexcept
        {
            return raw_allocate_impl<true>(
                ::std::index_sequence_for<Allocators...>{},
                count * sizeof(T),
                hint
            );
        }

        constexpr void deallocate(pointer& ptr, size_type count) noexcept
        {
            const auto alloc_info = get_alloc_info(auto_cast(ptr));
            const auto index = alloc_info.get_alloc_index();
            const auto& void_p = alloc_info.ptr;

            count += sizeof(::std::size_t);

            {
                get<index>(*this).deallocate(auto_cast(ptr), count * sizeof(T));
            }
        }

        [[nodiscard]] constexpr auto max_size() const noexcept {}
    };
}