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

            constexpr void operator()() const noexcept
            {
                alloc_traits::deallocate(
                    alloc_,
                    pointer_traits<typename alloc_traits::pointer>::to_pointer(ptr_),
                    count_
                );
            }

            constexpr auto ptr() const noexcept { return ptr_; }

            constexpr auto count() const noexcept { return count_; }

            constexpr auto& allocator() const noexcept { return alloc_.get(); }

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
        template<typename, typename>
        struct composed_allocator_traits;

        template<
            typename Tuple,
            ::std::size_t... I // clang-format off
        > // clang-format on
            requires requires(::std::tuple_element_t<I, Tuple>... allocator) //
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
            true;
        }
        struct composed_allocator_traits<Tuple, ::std::index_sequence<I...>>
        {
            template<::std::size_t J>
            using alloc = ::std::tuple_element_t<J, Tuple>;

            template<::std::size_t J>
            using alloc_traits = allocator_traits<alloc<J>>;

            using value_type = typename alloc<0>::value_type;

            static constexpr auto size = sizeof...(I);

            class deallocator
            {
                template<::std::size_t J>
                static constexpr void
                    deallocate(Tuple& alloc, value_type* const ptr, const ::std::size_t count)
                {
                    alloc_traits<J>::deallocate(
                        cpo::get<J>(alloc),
                        pointer_traits<typename alloc_traits<J>::pointer>::to_pointer(ptr),
                        count
                    );
                }

                static constexpr ::std::array deallocators{&deallocate<I>...};

            public:
                constexpr deallocator(
                    Tuple& alloc,
                    const ::std::size_t index,
                    value_type* const ptr,
                    const ::std::size_t count
                ) noexcept:
                    alloc_(alloc), index_(index), ptr_(ptr), count_(count)
                {
                }

                constexpr void operator()() noexcept { deallocators[index_](alloc_, ptr_, count_); }

                constexpr auto ptr() const noexcept { return ptr_; }

                constexpr auto count() const noexcept { return count_; }

                constexpr auto& allocator() const noexcept { return alloc_.get(); }

            private:
                ::std::reference_wrapper<Tuple> alloc_;
                ::std::size_t index_;
                value_type* ptr_;
                ::std::size_t count_;
            };

            [[nodiscard]] static constexpr details::raii_memory<deallocator> try_allocate(
                ::std::same_as<Tuple> auto& allocators,
                const ::std::size_t count,
                const void* const hint
            ) noexcept
            {
                value_type* ptr = nullptr;
                ::std::size_t alloc_index = 0;

                empty = ( //
                    ( //
                        alloc_index = I,
                        ptr = pointer_traits<typename alloc_traits<I>::pointer>:: //
                        to_address( //
                            alloc_traits<I>::try_allocate(
                                get<I>(allocators),
                                auto_cast(count),
                                pointer_traits<typename alloc_traits<I>::const_void_pointer>:: //
                                to_pointer(hint)
                            )
                        ),
                        ptr != nullptr
                    ) ||
                    ...
                );

                return {allocators, alloc_index, ptr, count};
            }

            [[nodiscard]] static constexpr details::raii_memory<deallocator> allocate(
                ::std::same_as<Tuple> auto& allocators,
                const ::std::size_t count,
                const void* const hint //
            )
            {
                value_type* ptr = nullptr;
                ::std::size_t alloc_index = 0;
                ::std::array<::std::exception_ptr, size> exceptions;

                empty = ( //
                    ( //
                        alloc_index = I,
                        [&ptr, &allocators, count, hint, &exceptions]
                        {
                            try
                            {
                                ptr = pointer_traits<typename alloc_traits<I>::pointer>:: //
                                    to_address( //
                                        alloc_traits<I>::allocate(
                                            get<I>(allocators),
                                            auto_cast(count),
                                            pointer_traits< // clang-format off
                                                typename alloc_traits<I>::const_void_pointer
                                            >::to_pointer(hint) // clang-format on
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

                return {allocators, alloc_index, ptr, count};
            }
        };

        template<typename T>
            requires requires { type_size_seq_t<T>{}; }
        using composed_allocator_traits_t = composed_allocator_traits<T, type_size_seq_t<T>>;
    }

    template<typename Tuple>
        requires requires { details::composed_allocator_traits_t<Tuple>{}; }
    [[nodiscard]] constexpr auto
        allocate(Tuple& allocators, const ::std::size_t count, const void* const hint = nullptr)
    {
        return details::composed_allocator_traits_t<Tuple>::allocate(allocators, count, hint);
    }

    template<typename Tuple>
        requires requires { details::composed_allocator_traits_t<Tuple>{}; }
    [[nodiscard]] constexpr auto try_allocate(
        Tuple& allocators,
        const ::std::size_t count,
        const void* const hint = nullptr
    ) noexcept
    {
        return details::composed_allocator_traits_t<Tuple>::try_allocate(allocators, count, hint);
    }
}