#pragma once

#include "allocator_traits.h"
#include "pointer_traits.h"
#include "../type_traits/indexed_traits.h"
#include "../tuple/tuple.h"

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
        template<typename T, typename = type_size_seq_t<T>>
        struct composed_allocator_traits;

        template<
            typename Tuple,
            ::std::size_t... I // clang-format off
        > // clang-format on
            requires requires(Tuple tuple, ::std::tuple_element_t<I, Tuple>... allocator) //
        {
            requires all_same<
                typename ::std::tuple_element_t<0, Tuple>::value_type,
                typename decltype(allocator)::value_type... // clang-format off
            >; // clang-format on

            ( //
                pointer_traits<
                    typename allocator_traits<decltype(allocator)>::const_void_pointer>:: //
                to_pointer({}),
                ...
            );
        }
        struct composed_allocator_traits<Tuple, ::std::index_sequence<I...>>
        {
            template<::std::size_t J>
            using alloc = ::std::tuple_element_t<J, Tuple>;

            template<::std::size_t J>
            using alloc_traits = allocator_traits<alloc<J>>;

            using value_type = typename alloc<0>::value_type;

            static constexpr auto size = sizeof...(I);

            template<::std::size_t J>
            using pointer_traits = pointer_traits<typename alloc_traits<J>::pointer>;

            template<::std::size_t J>
            using const_void_pointer_traits =
                stdsharp::pointer_traits<typename alloc_traits<J>::const_void_pointer>;

            template<::std::size_t J>
            struct deallocate_at
            {
                constexpr void
                    operator()(Tuple& alloc, value_type* const ptr, const ::std::uintmax_t n)
                        const noexcept
                {
                    alloc_traits<J>::deallocate(
                        get<J>(alloc),
                        pointer_traits<J>::to_pointer(ptr),
                        auto_cast(n)
                    );
                }
            };

            template<::std::size_t J>
            struct construct_at
            {
                template<typename T, typename... Args>
                constexpr void operator()(Tuple& alloc, T* const ptr, Args&&... args) const
                    requires requires //
                {
                    alloc_traits<J>::construct(get<J>(alloc), ptr, ::std::declval<Args>()...); //
                }
                {
                    alloc_traits<J>::construct(get<J>(alloc), ptr, ::std::forward<Args>(args)...);
                }
            };

            template<::std::size_t J>
            struct destroy_at
            {
                template<typename T>
                constexpr void operator()(Tuple& alloc, T* const ptr) const
                    requires requires { alloc_traits<J>::destroy(get<J>(alloc), ptr); }
                {
                    alloc_traits<J>::destroy(get<J>(alloc), ptr);
                }
            };

            template<template<::std::size_t> typename Fn, typename... Args>
                requires(::std::invocable<Fn<I>, Tuple&, Args...> && ...)
            static constexpr auto invoke_by_index(
                Tuple& alloc,
                const ::std::size_t index,
                Args&&... args
            ) noexcept((noexcept(Fn<I>{}(alloc, ::std::forward<Args>(args)...)) && ...))
            {
                static constexpr ::std::array impl{
                    +[](Tuple& alloc, const Args&&... args)
                    {
                        Fn<I>{}(alloc, ::std::forward<Args>(args)...); //
                    }... //
                };

                impl[index](alloc, ::std::forward<Args>(args)...);
            }

            struct alloc_ret
            {
                ::std::size_t index;
                value_type* ptr;
            };

            [[nodiscard]] static constexpr alloc_ret try_allocate(
                ::std::same_as<Tuple> auto& allocators,
                const ::std::uintmax_t n,
                const void* const hint
            ) noexcept
            {
                value_type* ptr = nullptr;
                ::std::size_t alloc_index = 0;

                empty = ( //
                    ( //
                        alloc_index = I,
                        ptr = pointer_traits<I>:: //
                        to_address( //
                            alloc_traits<I>::try_allocate(
                                get<I>(allocators),
                                auto_cast(n),
                                const_void_pointer_traits<I>::to_pointer(hint)
                            )
                        ),
                        ptr != nullptr
                    ) ||
                    ...
                );

                return {alloc_index, ptr};
            }

            [[nodiscard]] static constexpr alloc_ret allocate(
                ::std::same_as<Tuple> auto& allocators,
                const ::std::uintmax_t n,
                const void* const hint //
            )
            {
                value_type* ptr = nullptr;
                ::std::size_t alloc_index = 0;
                ::std::array<::std::exception_ptr, size> exceptions;

                empty = ( //
                    ( //
                        alloc_index = I,
                        [&ptr, &allocators, n, hint, &exceptions]
                        {
                            try
                            {
                                ptr = pointer_traits<I>:: //
                                    to_address( //
                                        alloc_traits<I>::allocate(
                                            get<I>(allocators),
                                            auto_cast(n),
                                            const_void_pointer_traits<I>::to_pointer(hint)
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

                if(ptr == nullptr) throw aggregate_bad_alloc<size>{::std::move(exceptions)};

                return {alloc_index, ptr};
            }
        };
    }

    template<typename... Allocator>
        requires requires { details::composed_allocator_traits<indexed_values<Allocator...>>{}; }
    struct composed_allocator
    {
    private:
        using values = indexed_values<Allocator...>;

        using traits = details::composed_allocator_traits<values>;

    public:
        using value_type = typename traits::value_type;
        using alloc_ret = typename traits::alloc_ret;

        values allocators;

        [[nodiscard]] constexpr alloc_ret
            allocate(const ::std::uintmax_t n, const void* const hint = nullptr)
        {
            return traits::allocate(allocators, n, hint);
        }

        [[nodiscard]] constexpr alloc_ret
            try_allocate(const ::std::uintmax_t n, const void* const hint = nullptr) noexcept
        {
            return traits::try_allocate(allocators, n, hint);
        }

        constexpr void deallocate( // NOLINT(*-exception-escape)
            const alloc_ret ret,
            const ::std::size_t n
        ) noexcept
        {
            traits::template invoke_by_index<traits::template deallocate_at>(
                allocators,
                ret.index,
                ret.ptr,
                n
            );
        }

        template<typename T, typename... Args>
        constexpr void construct(const ::std::size_t index, T* const ptr, Args&&... args)
            requires requires //
        {
            traits::template invoke_by_index<traits::template construct_at>(
                allocators,
                ptr,
                index,
                ::std::declval<Args>()...
            );
        }
        {
            traits::template invoke_by_index<traits::template construct_at>(
                allocators,
                ptr,
                index,
                ::std::forward<Args>(args)...
            );
        }

        template<typename T>
        constexpr void destroy(const ::std::size_t index, T* const ptr)
            requires requires //
        {
            traits::template invoke_by_index<traits::template destroy_at>(
                allocators,
                index,
                ptr
            ); //
        }
        {
            traits::template invoke_by_index<traits::template destroy_at>(allocators, index, ptr);
        }
    };
}