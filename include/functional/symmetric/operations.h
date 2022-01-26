#include "containers/actions.h"
#include "pattern_match.h"
#include "functional/get.h"

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
                noexcept(noexcept(bind_ref_front(Op, ::std::forward<Args>(args)...)))
            {
                return bind_ref_front(Op, ::std::forward<Args>(args)...);
            }

            template<typename Container, typename Iter, typename... Args>
                requires ::std::invocable<actions::emplace_fn, Container, Iter, Args...>
            [[nodiscard]] constexpr auto operator()(
                const actions::emplace_fn,
                Container& container,
                const Iter iter,
                Args&&... //
            ) const noexcept(concepts::nothrow_copy_constructible<Iter>)
            {
                return bind_ref_front(actions::erase, container, ::std::move(iter));
            }

            template<typename... Args>
                requires ::std::invocable<actions::emplace_back_fn, Args...>
            [[nodiscard]] constexpr auto operator()(
                const actions::emplace_back_fn,
                Args&&... args //
            ) const noexcept
            {
                return bind_ref_front(
                    actions::pop_back,
                    pack_get<0>(::std::forward<Args&&>(args)...) //
                );
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
                return bind_ref_front(assign_v, t, copy(t));
            }
        };

        using operation_fn = ::ranges::overloaded<specialized_operation_fn, default_operation_fn>;
    }

    inline constexpr struct operation_fn
    {
        template<typename... Args>
            requires(
                ::std::invocable<details::operation_fn, Args...> &&
                !cpo_invocable<operation_fn, Args...>)
        constexpr decltype(auto) operator()(Args&&... args) const
            noexcept(concepts::nothrow_invocable<details::operation_fn, Args...>)
        {
            return details::operation_fn{}(::std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires cpo_invocable<operation_fn, Args...>
        constexpr decltype(auto) operator()(Args&&... args) const
            noexcept(cpo_nothrow_invocable<operation_fn, Args...>)
        {
            return cpo(*this, ::std::forward<Args>(args)...);
        }
    } operation{};
}