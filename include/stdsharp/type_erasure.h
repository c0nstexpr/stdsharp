#pragma once

#include "cassert/cassert.h"
#include "memory/static_allocator.h"
#include "memory/scoped_memory.h"
#include "type_traits/object.h"
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
        class erasure
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
    }

    template<typename ErasedT, typename... Func>
    using basic_erasure = typename details::erasure<ErasedT, Func...>::type;

    template<typename ErasedT, typename... Func>
    class ref_erasure
    {
        using erasure_t = basic_erasure<ErasedT, Func...>;

        erasure_t erasure_;
        ::std::reference_wrapper<ErasedT> ref_;

        template<::std::size_t I>
        using func_traits = typename erasure_t::template func_traits<I>;

        struct default_ret : private_object<ref_erasure>
        {
        };

    public:
        using erased_t = typename erasure_t::erased_t;

        template<typename... Ptr>
        constexpr ref_erasure(ErasedT& t, const Ptr... ptr): ref_(t), erasure_(ptr...)
        {
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
            return erasure_.template invoke_at<I, Ret>(ref_.get(), ::std::forward<Args>(args)...);
        }
    };

    template<typename ErasedT, typename... Func>
    ref_erasure(ErasedT&, Func...) -> ref_erasure<ErasedT, Func...>;

    template<typename Allocator, typename... Func>
    class erasure
    {
    public:
        using erased_t = typename Allocator::value_type;

    private:
        using erasure_t = basic_erasure<erased_t, Func...>;

        erasure_t erasure_;

        indexed_values<sbo_allocator<erased_t>, Allocator> allocator_;
        scope::memory_t<decltype(allocator_)> storage_;

        template<::std::size_t I>
        using func_traits = typename erasure_t::template func_traits<I>;

        struct default_ret : private_object<erasure>
        {
        };

    public:
        template<typename... Ptr>
        constexpr erasure(ErasedT& t, const Ptr... ptr): storage_(t), erasure_(ptr...)
        {
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
            return erasure_.template invoke_at<I, Ret>(storage_, ::std::forward<Args>(args)...);
        }
    };

    template<typename ErasedT, typename... Func>
    erasure(ErasedT&, Func...) -> erasure<ErasedT, Func...>;
}