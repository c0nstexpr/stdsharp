#pragma once
#include "functional/operations.h"
#include "functional/bind.h"
#include "tuple/tuple.h"

namespace stdsharp::functional
{
    namespace details
    {
        template<typename>
        struct arithmetic_operation;

        template<>
        struct arithmetic_operation<functional::plus_assign> :
            ::std::type_identity<functional::minus_assign>
        {
        };

        template<>
        struct arithmetic_operation<functional::minus_assign> :
            ::std::type_identity<functional::plus_assign>
        {
        };

        template<>
        struct arithmetic_operation<functional::multiplies_assign> :
            ::std::type_identity<functional::divides_assign>
        {
        };

        template<>
        struct arithmetic_operation<functional::divides_assign> :
            ::std::type_identity<functional::multiplies_assign>
        {
        };

        template<>
        struct arithmetic_operation<functional::negate_assign> :
            ::std::type_identity<functional::negate_assign>
        {
        };

        template<>
        struct arithmetic_operation<functional::logical_not_assign> :
            ::std::type_identity<functional::logical_not_assign>
        {
        };

        struct specialized_operation_fn
        {
            template<
                typename... Args,
                ::std::invocable<Args...> Operation,
                auto Op = arithmetic_operation<Operation>{} // clang-format off
            > // clang-format on
            [[nodiscard]] constexpr auto operator()(const Operation, Args&&... args) const
                noexcept(noexcept(bind(Op, ::std::forward<Args>(args)...)))
            {
                return bind(Op, ::std::forward<Args>(args)...);
            }
        };

        struct default_operation_fn
        {
            template<::std::copy_constructible T, typename... Args>
            [[nodiscard]] constexpr auto operator()(
                const ::std::invocable<T&, Args...> auto&,
                T& t,
                Args&&... //
            ) const noexcept(concepts::nothrow_copy_constructible<T>)
            {
                return bind(assign_v, t, copy(t));
            }
        };

        using operation_fn = sequenced_invocables<specialized_operation_fn, default_operation_fn>;
    }

    inline constexpr struct symmetric_operation_fn
    {
        template<typename... Args>
            requires(
                ::std::invocable<details::operation_fn, Args...> &&
                !cpo_invocable<symmetric_operation_fn, Args...>)
        constexpr decltype(auto) operator()(Args&&... args) const
            noexcept(concepts::nothrow_invocable<details::operation_fn, Args...>)
        {
            return details::operation_fn{}(::std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires cpo_invocable<symmetric_operation_fn, Args...>
        constexpr decltype(auto) operator()(Args&&... args) const
            noexcept(cpo_nothrow_invocable<symmetric_operation_fn, Args...>)
        {
            return cpo(*this, ::std::forward<Args>(args)...);
        }
    } symmetric_operation{};
}