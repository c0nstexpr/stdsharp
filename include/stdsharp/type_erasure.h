#pragma once

#include "cassert/cassert.h"
#include "memory/composed_allocator.h"
#include "memory/static_allocator.h"
#include "type_traits/member.h"

namespace stdsharp
{
    namespace details
    {
        template<typename T>
        struct dispatch_traits
        {
            using func_traits = member_function_traits<T>;

            template<typename = typename func_traits::args_t>
            struct impl;

            template<template<typename...> typename Seq, typename... Args>
            struct impl<Seq<Args...>>
            {
                template<typename U>
                using qualified_t = apply_qualifiers<
                    U,
                    func_traits::is_const,
                    func_traits::is_volatile,
                    func_traits::ref_type // clang-format off
                >; // clang-format on

                template<typename Storage>
                using type = typename func_traits::result_t (*)(Storage, Args...) //
                    noexcept(func_traits::is_noexcept);
            };

            using type = impl<>;
        };

        template<typename T>
            requires requires { member_function_traits<T>{}; }
        using dispatch_traits_t = typename dispatch_traits<::std::decay_t<T>>::type;

        template<typename ErasedT, typename... Func>
            requires requires { (dispatch_traits_t<Func>{}, ...); }
        class basic_erasure
        {
            using func_types = stdsharp::indexed_types<Func...>;

            template<typename = ::std::index_sequence_for<Func...>>
            class impl;

            template<::std::size_t... I>
            class impl<::std::index_sequence<I...>>
            {
                template<::std::size_t J>
                using traits = dispatch_traits_t<::std::tuple_element_t<J, func_types>>;

                template<::std::size_t J>
                using dispatch_t = typename traits<J>::template type<
                    typename traits<J>::template qualified_t<ErasedT>>;

                indexed_values<dispatch_t<I>...> dispatchers_;

            public:
                using erased_t = ErasedT;

                template<::std::size_t J>
                using func_traits = function_traits<dispatch_t<J>>;

                impl() = default;

                template<typename... T>
                    requires(::std::same_as<nullptr_t, T> || ...)
                constexpr impl(const T...) = delete;

                constexpr impl(const dispatch_t<I>... t): dispatchers_(t...)
                {
                    debug_assert<::std::invalid_argument>(
                        [&]() noexcept { return ((t == nullptr) || ...); },
                        "one of the arguments is null pointer"
                    );
                }

                template<::std ::size_t J, typename Ret = typename func_traits<J>::result_t>
                struct invocable_at
                {
                    using ptr = const typename func_traits<J>::ptr_t&;

                    template<typename... Args>
                    struct with
                    {
                        static constexpr auto invocable = invocable_r<Ret, ptr, Args...>;

                        static constexpr auto nothrow_invocable =
                            nothrow_invocable_r<Ret, ptr, Args...>;
                    };
                };

                template<
                    ::std ::size_t J,
                    typename Ret = typename func_traits<J>::result_t,
                    typename... Args,
                    typename Invocable =
                        typename invocable_at<J, Ret>::template with<Args...> // clang-format off
                > // clang-format on
                    requires Invocable::invocable
                constexpr Ret invoke_at(Args&&... args) const noexcept(Invocable::nothrow_invocable)
                {
                    return invoke_r<Ret>(get<J>(dispatchers_), ::std ::forward<Args>(args)...);
                }
            };

            using type = impl<>;
        };

        struct alignas(::std::max_align_t) common_erased_base
        {
        };
    }

    template<typename ErasedT, typename... Func>
    using basic_erasure = typename details::basic_erasure<ErasedT, Func...>::type;

    template<typename ErasedT, typename... Func>
    class ref_erasure
    {
        using erasure_t = basic_erasure<ErasedT, Func...>;

        erasure_t erasure_;
        ::std::reference_wrapper<ErasedT> ref_;

        template<::std::size_t I>
        using func_traits = typename erasure_t::template func_traits<I>;

        struct default_ret;

    public:
        using erased_t = ErasedT;

        template<typename... Ptr>
        constexpr ref_erasure(ErasedT& t, const Ptr... ptr): ref_(t), erasure_(ptr...)
        {
        }

        constexpr decltype(auto) get() const noexcept { return ref_.get(); }

        template<::std ::size_t J, typename Ret = default_ret>
        struct invocable_at
        {
            template<typename... Args>
            using with = typename ::std::conditional_t<
                ::std::same_as<Ret, default_ret>,
                typename erasure_t::template invocable_at<J>,
                typename erasure_t::template invocable_at<J, Ret> // clang-format off
            >::template with<erased_t&, Args...>; // clang-format on
        };

        template<
            ::std ::size_t I,
            typename Ret = default_ret,
            typename... Args,
            typename Invocable =
                typename invocable_at<I, Ret>::template with<Args...> // clang-format off
        > // clang-format on
            requires Invocable::invocable
        constexpr decltype(auto) invoke_at(Args&&... args) const
            noexcept(Invocable::nothrow_invocable)
        {
            return erasure_.template invoke_at<I, Ret>(ref_.get(), ::std::forward<Args>(args)...);
        }
    };

    template<typename ErasedT, typename... Func>
    ref_erasure(ErasedT&, Func...) -> ref_erasure<ErasedT, Func...>;

    namespace details
    {
        using default_erasure_allocator = ::std::allocator<common_erased_base>;

        template<allocator_req Allocator, typename... Func>
        class erasure
        {
            using erased_t = common_erased_base;

            using alloc_traits = allocator_traits<Allocator>;

            template<
                allocator_req RebindAlloc = typename alloc_traits::template rebind_alloc<erased_t>>
            struct impl
            {
                using erasure_t = basic_erasure<erased_t, Func...>;

                erasure_t erasure_;
                composed_allocator<sbo_allocator<erased_t>, RebindAlloc> allocator_;

                struct storage
                {
                    ::std::uintmax_t count{};
                    typename decltype(allocator_)::alloc_ret allocated{nullptr, 2};
                    void (*destructor)(const storage&) noexcept {};

                    constexpr storage() = default;

                    constexpr storage(
                        const ::std::uintmax_t count,
                        decltype(allocator_)& allocator
                    ):
                        count(count), allocated(allocator.allocate(count))
                    {
                    }

                    template<typename T>
                        requires requires //
                    {
                        requires ::std::constructible_from<::std::decay_t<T>, T>; //
                    }
                    constexpr void construct(T&& t)
                    {
                        ::std::construct_at(
                            get_ptr<::std::decay_t<T>>(allocated.ptr),
                            ::std::forward<T>(t)
                        );
                        destructor = [](const storage& this_) noexcept
                        {
                            this_.get<::std::decay_t<T>>->~Box(); //
                        };
                    }

                    constexpr void destroy() noexcept
                    {
                        if(destructor != nullptr) (*destructor)(*this);
                    }

                    constexpr void deallocate(decltype(allocator_)& allocator) noexcept
                    {
                        allocator.deallocate(allocated, count);
                    }

                    template<typename T>
                    constexpr T* get_ptr() const noexcept
                    {
                        return static_cast<T*>(static_cast<void*>(allocated.ptr));
                    }

                    template<typename T>
                    constexpr T& get() const noexcept
                    {
                        return *get_ptr<T>();
                    }
                } storage_;

                template<::std::size_t I>
                using func_traits = typename erasure_t::template func_traits<I>;

                struct default_ret;

            public:
                impl() = default;

                template<::std::destructible T, typename... Ptr>
                constexpr impl(T&& t, const Ptr... ptr):
                    erasure_(ptr...),
                    storage_(ceil_reminder(sizeof(T), sizeof(erased_t)), allocator_)
                {
                    storage_.construct(::std::forward<T>(t));
                }

                impl(const impl&) = default;
                impl(impl&&) noexcept = default;
                impl& operator=(const impl&) = default;
                impl& operator=(impl&&) noexcept = default;

                constexpr ~erasure()
                {
                    if(storage_.count == 0) return;

                    storage_.destroy();
                    storage_.deallocate(allocator_);
                }

                template<::std ::size_t J, typename Ret = default_ret>
                struct invocable_at
                {
                    template<typename... Args>
                    using with = typename ::std::conditional_t<
                        ::std::same_as<Ret, default_ret>,
                        typename erasure_t::template invocable_at<J>,
                        typename erasure_t::template invocable_at<J, Ret> // clang-format off
                    >::template with<erased_t&, Args...>; // clang-format on
                };

                template<
                    ::std ::size_t I,
                    typename Ret = default_ret,
                    typename... Args,
                    typename Invocable =
                        typename invocable_at<I, Ret>::template with<Args...> // clang-format off
                 > // clang-format on
                    requires Invocable::invocable
                constexpr Ret invoke_at(Args&&... args) const noexcept(Invocable::nothrow_invocable)
                {
                    return erasure_.template invoke_at<I, Ret>(
                        storage_,
                        ::std::forward<Args>(args)...
                    );
                }
            };
        };
    }

    template<typename Allocator = details::default_erasure_allocator, typename... Func>
    struct erasure : details::erasure<Allocator, Func...>::template impl<>
    {
        using details::erasure<Allocator, Func...>::template impl<>::impl;
    };

    template<typename... Func>
    erasure(Func...) -> erasure<details::default_erasure_allocator, Func...>;
}