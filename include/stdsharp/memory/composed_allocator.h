#include "allocator_traits.h"
#include "pointer_traits.h"
#include "../type_traits/indexed_traits.h"
#include "../tuple/tuple.h"
#include <compare>

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
            static constexpr auto deallocate_at(
                Tuple& alloc,
                value_type* const ptr,
                const ::std::size_t count
            ) noexcept
            {
                alloc_traits<J>::deallocate(
                    get<J>(alloc),
                    pointer_traits<typename alloc_traits<J>::pointer>::to_pointer(ptr),
                    count
                );
            }

            template<::std::size_t Begin = 0, ::std::size_t Size = size>
            static constexpr auto recursive_deallocate(
                Tuple& alloc,
                const ::std::size_t index,
                value_type* const ptr,
                const ::std::size_t count
            ) noexcept
            {
                if constexpr(Size == 1)
                {
                    if(Begin == index) deallocate_at<Begin>(alloc, ptr, count);
                }
                else
                {
                    constexpr auto half_size = Size / 2;
                    constexpr auto mid_point = Begin + half_size - 1;

                    const auto compared = mid_point <=> index;

                    if(compared == ::std::strong_ordering::equal)
                        deallocate_at<mid_point>(alloc, ptr, count);
                    else if(compared == ::std::strong_ordering::less)
                        recursive_deallocate<mid_point + 1, half_size>(alloc, index, ptr, count);
                    else if(compared == ::std::strong_ordering::greater)
                        recursive_deallocate<Begin, half_size>(alloc, index, ptr, count);
                }
            }

            struct alloc_ret
            {
                ::std::size_t index;
                value_type* ptr;
            };

            [[nodiscard]] static constexpr alloc_ret try_allocate(
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

                return {alloc_index, ptr};
            }

            [[nodiscard]] static constexpr alloc_ret allocate(
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
            allocate(const ::std::size_t count, const void* const hint = nullptr)
        {
            return traits::allocate(allocators, count, hint);
        }

        [[nodiscard]] constexpr alloc_ret
            try_allocate(const ::std::size_t count, const void* const hint = nullptr) noexcept
        {
            return traits::try_allocate(allocators, count, hint);
        }

        constexpr void deallocate(const alloc_ret ret, const ::std::size_t count) noexcept
        {
            traits::template recursive_deallocate<>(allocators, ret.index, ret.ptr, count);
        }
    };
}