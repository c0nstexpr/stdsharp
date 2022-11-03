//
// Created by BlurringShadow on 2021-10-6.
//

#pragma once

#include <range/v3/action.hpp>

#include "../tuple/tuple.h"
#include "containers.h"

namespace stdsharp::actions
{
    inline constexpr struct emplace_fn
    {
        template<typename... Args, containers::container_emplace_constructible<Args...> Container>
            requires containers::sequence_container<Container>
        constexpr decltype(auto
        ) operator()(Container& container, const decltype(container.cbegin()) iter, Args&&... args)
            const
        {
            return container.emplace(iter, ::std::forward<Args>(args)...);
        }

        template<typename... Args, containers::container_emplace_constructible<Args...> Container>
            requires containers::associative_like_container<Container>
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
    }

    namespace cpo
    {
        namespace details
        {
            using namespace actions::details;

            template<typename Container>
            using const_iter_t = ranges::const_iterator_t<Container>;

            void erase(auto&&, auto&&) = delete;

            struct erase_fn
            {
                template<container_erasable Container>
                constexpr auto operator()(
                    Container& container,
                    const ::std::equality_comparable_with< // clang-format off
                        typename ::std::decay_t<Container>::value_type
                    > auto& value // clang-format on
                ) const
                    requires requires // clang-format off
                    {
                        requires sequence_container<Container>;
                        erase(container, value);
                    } // clang-format on
                {
                    return erase(container, value);
                }

                template<container_erasable Container>
                    requires associative_like_container<Container>
                constexpr auto operator()(
                    Container& container,
                    const ::std::equality_comparable_with<
                        typename ::std::decay_t<Container>::key_type // clang-format off
                    > auto& key // clang-format on
                ) const
                {
                    return container.erase(key);
                }

                template<
                    container_erasable Container,
                    ::std::convertible_to<const_iter_t<Container>>... ConstIter // clang-format off
                > // clang-format on
                    requires requires // clang-format off
                    {
                        requires(sequence_container<Container> || associative_like_container<Container>);
                        requires sizeof...(ConstIter) <= 1;
                    } // clang-format on
                constexpr auto operator()(
                    Container& container,
                    const decltype(container.cbegin()) const_iter_begin,
                    const ConstIter... const_iter_end
                ) const
                {
                    return container.erase(
                        const_iter_begin,
                        static_cast<const_iter_t<Container>>(const_iter_end)...
                    );
                }
            };

        }

        inline namespace cpo_impl
        {
            using details::erase_fn;

            inline constexpr erase_fn erase{};
        }
    }

#define STDSHARP_EMPLACE_WHERE_ACTION(where, iter)                                              \
    namespace details                                                                           \
    {                                                                                           \
        struct emplace_##where##_default_fn                                                     \
        {                                                                                       \
            template<typename Container, typename... Args>                                      \
                requires ::std::                                                                \
                    invocable<emplace_fn, Container&, const_iter_t<Container>, Args...>         \
                constexpr decltype(auto) operator()(Container& container, Args&&... args) const \
            {                                                                                   \
                return *actions::emplace(                                                       \
                    container,                                                                  \
                    container.c##iter(),                                                        \
                    ::std::forward<Args>(args)...                                               \
                );                                                                              \
            }                                                                                   \
        };                                                                                      \
                                                                                                \
        struct emplace_##where##_mem_fn                                                         \
        {                                                                                       \
            template<                                                                           \
                typename... Args,                                                               \
                containers::container_emplace_constructible<Args...> Container>                 \
            constexpr typename ::std::decay_t<Container>::reference                             \
                operator()(Container& container, Args&&... args) const                          \
                requires requires {                                                             \
                             requires containers::container<Container>;                         \
                             container.emplace_##where(::std::declval<Args>()...);              \
                         }                                                                      \
            {                                                                                   \
                return container.emplace_##where(::std::forward<Args>(args)...);                \
            }                                                                                   \
        };                                                                                      \
    }                                                                                           \
                                                                                                \
    using emplace_##where##_fn = functional::sequenced_invocables<                              \
        details::emplace_##where##_mem_fn,                                                      \
        details::emplace_##where##_default_fn>;                                                 \
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
            constexpr auto operator()(Container& container, Predicate&& predicate_fn) const
                requires requires //
            {
                requires ::std::same_as<
                    decltype(erase_if(container, ::std::declval<Predicate>())),
                    ::std::ranges::range_size_t<Container> // clang-format off
                >; // clang-format on
            }
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
                requires requires //
            {
                requires ::std::invocable<decltype(::ranges::remove_if), Container, Predicate>;
                requires ::std::invocable<
                    decltype(::std::ranges::distance),
                    ::std::ranges::iterator_t<Container>,
                    const_iter_t<Container> // clang-format off
                >; // clang-format on
            }
            constexpr auto operator()(Container& container, Predicate predicate_fn) const
            {
                const auto& it = static_cast<const_iter_t<Container>>(
                    ::ranges::remove_if(container, predicate_fn)
                );
                const auto r = ::std::ranges::distance(it, container.cend());
                actions::cpo::erase(container, it, container.cend());
                return r;
            }
        };
    }

    namespace cpo::inline cpo_impl
    {
        using erase_if_fn = functional::
            sequenced_invocables<details::adl_erase_if_fn, details::default_erase_if_fn>;

        inline constexpr erase_if_fn erase_if{};
    }

    inline constexpr struct resize_fn
    {
        template<typename Container>
        using size_type = ::std::ranges::range_size_t<Container>;

        template<containers::sequence_container Container>
        constexpr void operator()(Container& container, const size_type<Container> size) const
            requires requires { container.resize(size); }
        {
            return container.resize(size);
        }
    } resize{};

#define STDSHARP_POP_WHERE_ACTION(where, iter)                                                  \
    namespace details                                                                           \
    {                                                                                           \
        struct pop_##where##_default_fn                                                         \
        {                                                                                       \
            template<typename Container>                                                        \
                requires ::std::                                                                \
                    invocable<actions::cpo::erase_fn, Container&, const_iter_t<Container>>      \
                constexpr void operator()(Container& container) const                           \
            {                                                                                   \
                actions::cpo::erase(container, container.c##iter());                            \
            }                                                                                   \
        };                                                                                      \
                                                                                                \
        struct pop_##where##_mem_fn                                                             \
        {                                                                                       \
            template<typename Container>                                                        \
            constexpr void operator()(Container& container) const                               \
                requires requires {                                                             \
                             requires sequence_container<Container>;                            \
                             requires ::std::same_as<decltype(container.pop_##where()), void>;  \
                         }                                                                      \
            {                                                                                   \
                return container.pop_##where();                                                 \
            }                                                                                   \
        };                                                                                      \
    }                                                                                           \
                                                                                                \
    using pop_##where##_fn = functional::                                                       \
        sequenced_invocables<details::pop_##where##_mem_fn, details::pop_##where##_default_fn>; \
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
            template<
                ::std::size_t Count,
                auto HasMember = requires(Container container) { container.reserve(Count); }>
            static constexpr auto reserved(Container& container) noexcept(!HasMember)
            {
                if constexpr(HasMember) container.reserve(Count);
            }

        public:
            template<typename... Args>
                requires(::std::invocable<actions::emplace_back_fn, Container&, Args> && ...)
            constexpr auto operator()(Args&&... args) const noexcept( //
                (concepts::nothrow_invocable<actions::emplace_back_fn, Container&, Args>&&...) && //
                noexcept(reserved<Container, sizeof...(Args)>())
            )
            {
                Container container{};
                reserved<sizeof...(Args)>(container);
                (emplace_back(container, ::std::forward<Args>(args)), ...);
                return container;
            }

            template<typename... Args>
                requires(::std::invocable<actions::emplace_fn, Container&, Args> && ...)
            constexpr auto operator()(Args&&... args) const noexcept( //
                (concepts::nothrow_invocable<actions::emplace_fn, Container&, Args>&&...) && //
                noexcept(reserved<Container, sizeof...(Args)>())
            )
            {
                Container container{};
                reserved<sizeof...(Args)>(container);
                (emplace(container, ::std::forward<Args>(args)), ...);
                return container;
            }
        };

        template<typename Container>
        using regular_make_container_fn = functional::sequenced_invocables<
            functional::construct_fn<Container>,
            details::emplace_make_container_fn<Container> // clang-format off
        >; // clang-format on

        template<typename Container>
        struct make_container_from_tuple_fn
        {
            template<
                typename... Tuples,
                typename ValueType = ::std::ranges::range_value_t<Container>,
                ::std::invocable<regular_make_container_fn<Container>, Tuples...> ApplyFn =
                    tuples_each_apply_fn<::std::conditional_t<true, ValueType, Tuples>...>
                // clang-format off
            > // clang-format on
            constexpr auto operator()(const ::std::piecewise_construct_t, Tuples&&... tuples) const
                noexcept(concepts::nothrow_invocable<
                         ApplyFn,
                         regular_make_container_fn<Container>,
                         Tuples...>)
            {
                return ApplyFn{}(
                    regular_make_container_fn<Container>{},
                    ::std::forward<Tuples>(tuples)...
                );
            }
        };
    }

    template<typename Container>
    using make_container_fn = functional::sequenced_invocables<
        functional::construct_fn<Container>,
        details::emplace_make_container_fn<Container>,
        details::make_container_from_tuple_fn<Container> // clang-format off
    >; // clang-format on

    template<typename Container>
    inline constexpr make_container_fn<Container> make_container{};
}