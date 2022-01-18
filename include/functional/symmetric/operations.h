#include "containers/actions.h"
#include "functional/functional.h"
#include "functional/operations.h"
#include "pattern_match.h"
#include "type_traits/type_traits.h"

namespace stdsharp::functional::symmetric
{
    namespace details
    {
        template<typename>
        struct arithmetic_operation;

        template<>
        struct arithmetic_operation<plus_assign> : ::std::type_identity<minus_assign>
        {
        };

        template<>
        struct arithmetic_operation<minus_assign> : ::std::type_identity<plus_assign>
        {
        };

        template<>
        struct arithmetic_operation<multiplies_assign> : ::std::type_identity<divides_assign>
        {
        };

        template<>
        struct arithmetic_operation<divides_assign> : ::std::type_identity<multiplies_assign>
        {
        };

        template<>
        struct arithmetic_operation<negate_assign> : ::std::type_identity<negate_assign>
        {
        };

        template<>
        struct arithmetic_operation<logical_not_assign> : ::std::type_identity<logical_not_assign>
        {
        };
    }

    struct operation_t : nodiscard_tag_t
    {
        template<
            typename... Args,
            ::std::invocable<Args...> Operation,
            auto Op = details::arithmetic_operation<Operation>{} // clang-format off
        > // clang-format on
        [[nodiscard]] constexpr auto operator()(const Operation, Args&&... args) const
            noexcept(noexcept(bind_ref_front(Op, ::std::forward<Args>(args)...)))
        {
            return bind_ref_front(Op, ::std::forward<Args>(args)...);
        }

        template<typename Container, typename Iter, typename... Args>
            requires ::std::invocable<containers::actions::emplace_fn, Container, Iter, Args...>
        [[nodiscard]] constexpr auto operator()(
            const containers::actions::emplace_fn,
            Container& container,
            const Iter iter,
            Args&&... //
        ) const noexcept(concepts::nothrow_copy_constructible<Iter>)
        {
            return bind_ref_front(containers::actions::erase, container, copy(iter));
        }

        template<typename Container, typename... Args>
            requires ::std::invocable<containers::actions::emplace_back_fn, Container, Args...>
        [[nodiscard]] constexpr auto operator()(
            const containers::actions::emplace_back_fn,
            Container& container,
            Args&&... //
        ) const noexcept
        {
            return bind_ref_front(containers::actions::pop_back, container);
        }

        template<::std::copy_constructible T, typename... Args>
        [[nodiscard]] constexpr auto operator()(
            const ::std::invocable<T, Args...> auto&,
            T& t,
            Args&&... //
        ) const noexcept(concepts::nothrow_copy_constructible<T>)
        {
            return bind_ref_front(assign_v, t, copy(t));
        }
    };

    inline constexpr auto operation = tagged_cpo<operation_t>;
}