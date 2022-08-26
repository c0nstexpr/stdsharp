//
// Created by BlurringShadow on 2021-10-6.
//

#pragma once

#include <utility>

#include <range/v3/action.hpp>

#include "../functional/invocables.h"
#include "../tuple/tuple.h"
#include "containers.h"

namespace stdsharp::actions
{
    inline constexpr struct emplace_fn
    {
        template<typename... Args, typename Container>
            requires containers::sequence_container<Container> &&
                containers::container_emplace_constructible<Container, Args...>
        constexpr decltype(auto) operator()(
            Container& container,
            const decltype(container.cbegin()) iter,
            Args&&... args //
        ) const
        {
            return container.emplace(iter, ::std::forward<Args>(args)...);
        }

        template<containers::associative_like_container Container, typename... Args>
            requires containers::container_emplace_constructible<Container, Args...>
        constexpr decltype(auto) operator()(Container& container, Args&&... args) const
        {
            return container.emplace(::std::forward<Args>(args)...);
        }
    } emplace;

    namespace details
    {
        using namespace containers;

        template<typename Container>
        using const_iter_t = ranges::const_iterator_t<Container>;

        void erase(auto&&, auto&&) = delete;

        struct erase_fn
        {
            template<sequence_container Container>
                requires container_erasable<Container>
            constexpr auto operator()(
                Container& container,
                const ::std::equality_comparable_with<
                    typename ::std::decay_t<Container>::value_type> auto& value //
            ) const requires requires
            {
                erase(container, value);
            }
            {
                return erase(container, value);
            }

            template<associative_like_container Container>
                requires container_erasable<Container>
            constexpr auto operator()(
                Container& container,
                const ::std::equality_comparable_with<
                    typename ::std::decay_t<Container>::key_type> auto& key //
            ) const
            {
                return container.erase(key);
            }

            template<
                typename Container, // clang-format off
                ::std::convertible_to<const_iter_t<Container>>... ConstIter
            > // clang-format on
                requires container_erasable<Container> && requires
                {
                    requires(
                        sequence_container<Container> || // clang-format off
                        associative_like_container<Container> // clang-format on
                    );
                    requires sizeof...(ConstIter) <= 1;
                }
            constexpr auto operator()(
                Container& container,
                const decltype(container.cbegin()) const_iter_begin,
                const ConstIter... const_iter_end //
            ) const
            {
                return container.erase(
                    const_iter_begin,
                    static_cast<const_iter_t<Container>>(const_iter_end)... //
                );
            }
        };
    }

    inline namespace cpo
    {
        using details::erase_fn;

        inline constexpr erase_fn erase{};
    }

#define STDSHARP_EMPLACE_WHERE_ACTION(where, iter)                                          \
    namespace details                                                                       \
    {                                                                                       \
        struct emplace_##where##_default_fn                                                 \
        {                                                                                   \
            template<typename Container, typename... Args>                                  \
                requires ::std::                                                            \
                    invocable<emplace_fn, Container&, const_iter_t<Container>, Args...>     \
            constexpr decltype(auto) operator()(Container& container, Args&&... args) const \
            {                                                                               \
                return *actions::emplace(                                                   \
                    container, container.c##iter(), ::std::forward<Args>(args)...);         \
            }                                                                               \
        };                                                                                  \
                                                                                            \
        struct emplace_##where##_mem_fn                                                     \
        {                                                                                   \
            template<                                                                       \
                typename... Args,                                                           \
                containers::container_emplace_constructible<Args...> Container>             \
                requires requires(Container instance)                                       \
                {                                                                           \
                    requires containers::container<Container>;                              \
                    instance.emplace_##where(::std::declval<Args>()...);                    \
                }                                                                           \
            constexpr typename ::std::decay_t<Container>::reference                         \
                operator()(Container& container, Args&&... args) const                      \
            {                                                                               \
                return container.emplace_##where(::std::forward<Args>(args)...);            \
            }                                                                               \
        };                                                                                  \
    }                                                                                       \
                                                                                            \
    using emplace_##where##_fn = functional::sequenced_invocables<                          \
        details::emplace_##where##_mem_fn,                                                  \
        details::emplace_##where##_default_fn>;                                             \
                                                                                            \
    inline constexpr emplace_##where##_fn emplace_##where{};

    STDSHARP_EMPLACE_WHERE_ACTION(back, end)
    STDSHARP_EMPLACE_WHERE_ACTION(front, begin)

#undef STDSHARP_EMPLACE_WHERE_ACTION

    namespace details
    {
        void erase_if(auto&&, auto&&) = delete;

        struct adl_erase_if_fn
        {
            template<
                containers::container_erasable Container,
                containers::container_predicatable<Container> Predicate // clang-format off
            > // clang-format on
            constexpr auto operator()(Container& container, Predicate&& predicate_fn) const //
                requires requires
            { // clang-format off
                requires ::std::same_as<
                    decltype(erase_if(container, ::std::declval<Predicate>())),
                    ::std::ranges::range_size_t<Container>
                >;
            } // clang-format on
            {
                return erase_if(container, ::std::forward<Predicate>(predicate_fn));
            }
        };

        struct default_erase_if_fn
        {
            template<
                containers::container_erasable Container,
                containers::container_predicatable<Container> Predicate // clang-format off
            > // clang-format on
                requires requires(::std::ranges::iterator_t<Container> iter)
                {
                    requires ::std::permutable<decltype(iter)>;
                    ::std::ranges::distance(
                        iter,
                        ::std::declval<const_iter_t<Container>>() //
                    );
                }
            constexpr auto operator()(Container& container, Predicate predicate_fn) const
            { // TODO: Replace with range algo
                const auto& it = static_cast<const_iter_t<Container>>(
                    ::std::remove_if(container.begin(), container.end(), predicate_fn) //
                );
                const auto r = ::std::ranges::distance(it, container.cend());
                actions::erase(container, it, container.cend());
                return r;
            }
        };

        struct resize_impl
        {
        };
    }
    inline namespace cpo
    {
        using erase_if_fn = functional::
            sequenced_invocables<details::adl_erase_if_fn, details::default_erase_if_fn>;

        inline constexpr erase_if_fn erase_if{};
    }

    inline constexpr struct resize_fn : details::resize_impl
    {
        template<typename Container>
        using size_type = ::std::ranges::range_size_t<Container>;

        template<containers::sequence_container Container>
            requires requires(Container container, size_type<Container> size)
            {
                container.resize(size);
            }
        constexpr void operator()(Container& container, const size_type<Container> size) const
        {
            return container.resize(size);
        }
    } resize{};

#define STDSHARP_POP_WHERE_ACTION(where, iter)                                                    \
    namespace details                                                                             \
    {                                                                                             \
        struct pop_##where##_default_fn                                                           \
        {                                                                                         \
            template<typename Container>                                                          \
                requires ::std::invocable<actions::erase_fn, Container&, const_iter_t<Container>> \
            constexpr void operator()(Container& container) const                                 \
            {                                                                                     \
                actions::erase(container, container.c##iter());                                   \
            }                                                                                     \
        };                                                                                        \
                                                                                                  \
        struct pop_##where##_mem_fn                                                               \
        {                                                                                         \
            template<containers::sequence_container Container>                                    \
                requires requires(Container instance)                                             \
                {                                                                                 \
                    requires ::std::same_as<decltype(instance.pop_##where()), void>;              \
                }                                                                                 \
            constexpr void operator()(Container& container) const                                 \
            {                                                                                     \
                return container.pop_##where();                                                   \
            }                                                                                     \
        };                                                                                        \
    }                                                                                             \
                                                                                                  \
    using pop_##where##_fn = functional::                                                         \
        sequenced_invocables<details::pop_##where##_mem_fn, details::pop_##where##_default_fn>;   \
                                                                                                  \
    inline constexpr pop_##where##_fn pop_##where{};

    STDSHARP_POP_WHERE_ACTION(front, begin)
    STDSHARP_POP_WHERE_ACTION(back, end)

#undef STDSHARP_POP_WHERE_ACTION

    namespace details
    {
        template<typename Container>
        struct emplace_make_container_fn
        {
        private:
            template<::std::size_t Count, auto HasMember = requires(Container container)
            {
                container.reserve(Count);
            }
            > static constexpr auto reserved(Container& container) noexcept(!HasMember)
            {
                if constexpr(HasMember) container.reserve(Count);
            }

        public:
            template<typename... Args>
                requires(
                    (::std::invocable<actions::emplace_back_fn, Container&, Args> && ...) &&
                    !::std::constructible_from<Container, Args...>)
            constexpr auto operator()(Args&&... args) const noexcept(
                (concepts:: //
                 nothrow_invocable<actions::emplace_back_fn, Container&, Args> &&
                 ...) && //
                noexcept(reserved<Container, sizeof...(Args)>()) //
            )
            {
                Container container{};
                reserved<sizeof...(Args)>(container);
                (emplace_back(container, ::std::forward<Args>(args)), ...);
                return container;
            }

            template<typename... Args>
                requires(
                    (::std::invocable<actions::emplace_fn, Container&, Args> && ...) &&
                    !::std::constructible_from<Container, Args...>)
            constexpr auto operator()(Args&&... args) const noexcept(
                (concepts::nothrow_invocable<actions::emplace_fn, Container&, Args> && ...) && //
                noexcept(reserved<Container, sizeof...(Args)>()) //
            )
            {
                Container container{};
                reserved<sizeof...(Args)>(container);
                (emplace(container, ::std::forward<Args>(args)), ...);
                return container;
            }
        };
    }

    template<typename Container>
    struct make_container_fn :
        functional::sequenced_invocables<
            functional::constructor_fn<Container>,
            details::emplace_make_container_fn<Container> // clang-format off
        > // clang-format on
    {
    private:
        using base = functional::sequenced_invocables<
            functional::constructor_fn<Container>,
            details::emplace_make_container_fn<Container> // clang-format off
        >; // clang-format on

    public:
        template<
            typename... Tuples,
            typename ValueType = ::std::ranges::range_value_t<Container>,
            ::std::invocable<base, Tuples...> ApplyFn = tuples_each_apply_fn<
                ::std::conditional_t<true, ValueType, Tuples>... // clang-format off
            >
        > // clang-format on
            requires(!::std::invocable<base, ::std::piecewise_construct_t, Tuples...>)
        constexpr auto operator()(const ::std::piecewise_construct_t, Tuples&&... tuples) const
            noexcept(concepts::nothrow_invocable<ApplyFn, base, Tuples...>)
        {
            return tuples_each_apply<::std::conditional_t<true, ValueType, Tuples>...>(
                static_cast<const base&>(*this),
                ::std::forward<Tuples>(tuples)... //
            );
        }
    };

    template<typename Container>
    inline constexpr make_container_fn<Container> make_container{};
}