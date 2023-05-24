#pragma once

#include <range/v3/action.hpp>

#include "concepts.h"

namespace stdsharp::actions
{
    namespace details
    {
        template<typename T>
        using container_citer = T::const_iterator;
    }

    inline constexpr struct emplace_fn
    {
        template<typename... Args, container_emplace_constructible<Args...> Container>
            requires sequence_container<Container>
        constexpr decltype(auto) operator()(
            Container& container,
            const details::container_citer<Container> iter,
            Args&&... args
        ) const
        {
            return container.emplace(iter, cpp_forward(args)...);
        }

        template<typename... Args, container_emplace_constructible<Args...> Container>
            requires associative_like_container<Container>
        constexpr decltype(auto) operator()(Container& container, Args&&... args) const
        {
            return container.emplace(cpp_forward(args)...);
        }
    } emplace;

    namespace cpo
    {
        namespace details
        {
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
                    ::std::convertible_to<
                        actions::details::container_citer<Container> // clang-format off
                    >... ConstIter
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
                        static_cast<actions::details::container_citer<Container>>(const_iter_end)...
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
                requires ::std::invocable<                                                      \
                    emplace_fn,                                                                 \
                    Container&,                                                                 \
                    details::container_citer<Container>,                                        \
                    Args...>                                                                    \
            constexpr decltype(auto) operator()(Container& container, Args&&... args) const     \
            {                                                                                   \
                return *actions::emplace(container, container.c##iter(), cpp_forward(args)...); \
            }                                                                                   \
        };                                                                                      \
                                                                                                \
        struct emplace_##where##_mem_fn                                                         \
        {                                                                                       \
            template<typename... Args, container_emplace_constructible<Args...> Container>      \
            constexpr typename ::std::decay_t<Container>::reference                             \
                operator()(Container& container, Args&&... args) const                          \
                requires requires {                                                             \
                    requires stdsharp::container<Container>;                                    \
                    container.emplace_##where(::std::declval<Args>()...);                       \
                }                                                                               \
            {                                                                                   \
                return container.emplace_##where(cpp_forward(args)...);                         \
            }                                                                                   \
        };                                                                                      \
    }                                                                                           \
    using emplace_##where##_fn = sequenced_invocables<                                          \
        details::emplace_##where##_mem_fn,                                                      \
        details::emplace_##where##_default_fn>;                                                 \
                                                                                                \
    inline constexpr emplace_##where##_fn emplace_##where{};

    STDSHARP_EMPLACE_WHERE_ACTION(back, end)
    STDSHARP_EMPLACE_WHERE_ACTION(front, begin)

#undef STDSHARP_EMPLACE_WHERE_ACTION

    namespace cpo::inline cpo_impl
    {
        namespace details
        {
            void erase_if(auto&&, auto&&) = delete;

            struct adl_erase_if_fn
            {
                template<
                    container_erasable Container,
                    container_predicatable<Container> Predicate // clang-format off
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
                    return erase_if(container, cpp_forward(predicate_fn));
                }
            };

            struct default_erase_if_fn
            {
                template<
                    container_erasable Container,
                    container_predicatable<Container> Predicate // clang-format off
                > // clang-format on
                    requires requires //
                {
                    requires ::std::
                        invocable<decltype(::std::ranges::remove_if), Container, Predicate>;
                    requires ::std::invocable<
                        cpo::erase_fn,
                        Container&,
                        actions::details::container_citer<Container>,
                        actions::details::container_citer<Container> // clang-format off
                    >; // clang-format on
                }
                constexpr auto operator()(Container& container, Predicate&& predicate_fn) const
                {
                    const auto& it = ::std::ranges::remove_if(container, cpp_forward(predicate_fn));
                    const auto removed_size = it.size();
                    cpo::erase(container, it.begin(), it.end());
                    return removed_size;
                }
            };
        }

        using erase_if_fn =
            sequenced_invocables<details::adl_erase_if_fn, details::default_erase_if_fn>;

        inline constexpr erase_if_fn erase_if{};
    }

    inline constexpr struct resize_fn
    {
        template<typename Container>
        using size_type = ::std::ranges::range_size_t<Container>;

        template<sequence_container Container>
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
                    invocable<cpo::erase_fn, Container&, details::container_citer<Container>>   \
                constexpr void operator()(Container& container) const                           \
            {                                                                                   \
                cpo::erase(container, container.c##iter());                                     \
            }                                                                                   \
        };                                                                                      \
                                                                                                \
        struct pop_##where##_mem_fn                                                             \
        {                                                                                       \
            template<typename Container>                                                        \
            constexpr void operator()(Container& container) const                               \
                requires requires {                                                             \
                    requires sequence_container<Container>;                                     \
                    requires ::std::same_as<decltype(container.pop_##where()), void>;           \
                }                                                                               \
            {                                                                                   \
                return container.pop_##where();                                                 \
            }                                                                                   \
        };                                                                                      \
    }                                                                                           \
                                                                                                \
    using pop_##where##_fn =                                                                    \
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
                (nothrow_invocable<actions::emplace_back_fn, Container&, Args>&&...) && //
                noexcept(reserved<Container, sizeof...(Args)>())
            )
            {
                Container container{};
                reserved<sizeof...(Args)>(container);
                (emplace_back(container, cpp_forward(args)), ...);
                return container;
            }

            template<typename... Args>
                requires(::std::invocable<actions::emplace_fn, Container&, Args> && ...)
            constexpr auto operator()(Args&&... args) const noexcept( //
                (nothrow_invocable<actions::emplace_fn, Container&, Args>&&...) && //
                noexcept(reserved<Container, sizeof...(Args)>())
            )
            {
                Container container{};
                reserved<sizeof...(Args)>(container);
                (emplace(container, cpp_forward(args)), ...);
                return container;
            }
        };

        template<typename Container>
        static constexpr auto regular_make_container = make_sequenced_invoke(
            constructor<Container>{},
            details::emplace_make_container_fn<Container>{} // clang-format off
        ); // clang-format on

        template<typename Container>
        using regular_make_container_fn = decltype(regular_make_container<Container>);

        template<typename Container>
        struct make_container_from_tuple_fn
        {
            template<typename Tuple, typename ValueType = ::std::ranges::range_value_t<Container>>
                requires requires //
            {
                ::std::apply(regular_make_container_fn<Container>{}, ::std::declval<Tuple>()); //
            }
            constexpr auto operator()(
                const ::std::piecewise_construct_t,
                Tuple&& tuple
            ) const noexcept( //
                noexcept( //
                    ::std::apply(regular_make_container_fn<Container>{}, ::std::declval<Tuple>())
                )
            )
            {
                return ::std::apply(regular_make_container_fn<Container>{}, cpp_forward(tuple));
            }
        };
    }

    template<typename Container>
    using make_container_fn = sequenced_invocables<
        details::emplace_make_container_fn<Container>,
        details::make_container_from_tuple_fn<Container> // clang-format off
    >; // clang-format on

    template<typename Container>
    inline constexpr make_container_fn<Container> make_container;
}