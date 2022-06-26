//
// Created by BlurringShadow on 2021-10-6.
//

#pragma once

#include <utility>

#include <range/v3/action.hpp>

#include "../functional/invocables.h"
#include "../tuple/tuple.h"
#include "containers.h"
#include "../details/prologue.h"

namespace stdsharp::actions
{
    namespace details
    {
        using namespace containers;

        template<typename Container, typename... Args>
        concept seq_emplace_req =
            sequence_container<Container> && container_emplace_constructible<Container, Args...>;

        template<typename Container>
        concept associative_like_req =
            associative_container<Container> || unordered_associative_container<Container>;
    }

    inline constexpr struct emplace_fn
    {
    private:
        struct impl
        {
            template<typename... Args>
            constexpr decltype(auto) operator()(
                details::seq_emplace_req<Args...> auto& container,
                const decltype(container.cbegin()) iter,
                Args&&... args //
            ) const
            {
                return container.emplace(iter, ::std::forward<Args>(args)...);
            }

            template<details::associative_like_req Container, typename... Args>
                requires details::container_emplace_constructible<Container, Args...>
            constexpr decltype(auto) operator()(Container& container, Args&&... args) const
            {
                return container.emplace(::std::forward<Args>(args)...);
            }
        };

    public:
        template<typename... Args>
            requires(
                !functional::cpo_invocable<emplace_fn, Args...> &&
                ::std::invocable<impl, Args...> //
            )
        constexpr decltype(auto) operator()(Args&&... args) const //
        {
            return impl{}(::std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires functional::cpo_invocable<emplace_fn, Args...>
        constexpr decltype(auto) operator()(Args&&... args) const
        {
            return functional::cpo_invoke(*this, ::std::forward<Args>(args)...);
        }
    } emplace{};

    namespace details
    {
        void erase(auto&&, auto&&) = delete;

        struct erase_fn
        {
        private:
            struct impl
            {
                template<containers::sequence_container Container>
                    requires containers::container_erasable<Container>
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

                template<details::associative_like_req Container>
                    requires containers::container_erasable<Container>
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
                    ::std::convertible_to<ranges::const_iterator_t<Container>>... ConstIter
                > // clang-format on
                    requires details::container_erasable<Container> && requires
                    {
                        requires(
                            containers::sequence_container<Container> || // clang-format off
                            details::associative_like_req<Container> // clang-format on
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
                        static_cast<ranges::const_iterator_t<Container>>(const_iter_end)... //
                    );
                }
            };

        public:
            template<typename... Args>
                requires(
                    ::std::invocable<impl, Args...> &&
                    !functional::cpo_invocable<erase_fn, Args...> //
                )
            constexpr decltype(auto) operator()(Args&&... args) const
            {
                return impl{}(::std::forward<Args>(args)...);
            }

            template<typename... Args>
                requires functional::cpo_invocable<erase_fn, Args...>
            constexpr decltype(auto) operator()(Args&&... args) const
            {
                return functional::cpo_invoke(*this, ::std::forward<Args>(args)...);
            }
        };
    }

    inline namespace cpo
    {
        using details::erase_fn;
        inline constexpr erase_fn erase{};
    }

    inline constexpr struct emplace_back_fn
    {
    private:
        struct default_fn
        {
            template<typename Container, typename... Args>
                requires ::std::invocable<
                    emplace_fn,
                    Container&,
                    ranges::const_iterator_t<Container>,
                    Args... // clang-format off
                > // clang-format on
            constexpr decltype(auto) operator()(Container& container, Args&&... args) const
            {
                return *emplace(container, container.cend(), ::std::forward<Args>(args)...);
            }
        };

        struct mem_fn
        {
            template<typename... Args, details::seq_emplace_req<Args...> Container>
                requires requires(Container instance)
                {
                    instance.emplace_back(::std::declval<Args>()...);
                }
            constexpr typename ::std::decay_t<Container>::reference
                operator()(Container& container, Args&&... args) const
            {
                return container.emplace_back(::std::forward<Args>(args)...);
            }
        };

    public:
        template<
            typename... Args,
            ::std::invocable<Args...> Fn = // clang-format off
                functional::sequenced_invocables<mem_fn, default_fn>
        > // clang-format on
        constexpr decltype(auto) operator()(Args&&... args) const
        {
            Fn{}(::std::forward<Args>(args)...);
        }
    } emplace_back{};

    inline constexpr struct emplace_front_fn
    {
    private:
        struct default_fn
        {
            template<typename Container, typename... Args>
                requires ::std::invocable<
                    emplace_fn,
                    Container&,
                    ranges::const_iterator_t<Container>,
                    Args... // clang-format off
                > // clang-format on
            constexpr decltype(auto) operator()(Container& container, Args&&... args) const
            {
                return *emplace(container, container.cend(), ::std::forward<Args>(args)...);
            }
        };

        struct mem_fn
        {
            template<typename... Args, details::seq_emplace_req<Args...> Container>
                requires requires(Container instance)
                {
                    instance.emplace_front(::std::declval<Args>()...);
                }
            constexpr typename ::std::decay_t<Container>::reference
                operator()(Container& container, Args&&... args) const
            {
                return container.emplace_front(::std::forward<Args>(args)...);
            }
        };

    public:
        template<
            typename... Args,
            ::std::invocable<Args...> Fn = // clang-format off
                functional::sequenced_invocables<mem_fn, default_fn>
        > // clang-format on
        constexpr decltype(auto) operator()(Args&&... args) const
        {
            Fn{}(::std::forward<Args>(args)...);
        }
    } emplace_front{};

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
                { erase_if(container, ::std::declval<Predicate>()) } ->
                    ::std::same_as<::std::ranges::range_size_t<Container>>;
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
                        ::std::declval<ranges::const_iterator_t<Container>>() //
                    );
                }
            constexpr auto operator()(Container& container, Predicate predicate_fn) const
            { // TODO: Replace with range algo
                const auto& it = static_cast<ranges::const_iterator_t<Container>>(
                    ::std::remove_if(container.begin(), container.end(), predicate_fn) //
                );
                const auto r = ::std::ranges::distance(it, container.cend());
                actions::erase(container, it, container.cend());
                return r;
            }
        };
    }
    inline namespace cpo
    {
        using erase_if_fn = functional::
            sequenced_invocables<details::adl_erase_if_fn, details::default_erase_if_fn>;
        inline constexpr erase_if_fn erase_if{};
    }

    inline constexpr struct resize_fn
    {
    private:
        struct impl
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
        };

    public:
        template<typename... Args>
            requires(
                ::std::invocable<impl, Args...> && !functional::cpo_invocable<resize_fn, Args...> //
            )
        constexpr decltype(auto) operator()(Args&&... args) const
        {
            return impl{}(::std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires functional::cpo_invocable<resize_fn, Args...>
        constexpr decltype(auto) operator()(Args&&... args) const
        {
            return functional::cpo_invoke(*this, ::std::forward<Args>(args)...);
        }
    } resize{};

    inline constexpr struct pop_front_fn
    {
    private:
        struct default_fn
        {
            template<typename Container>
                requires ::std::
                    invocable<actions::erase_fn, Container&, ranges::const_iterator_t<Container>>
            constexpr void operator()(Container& container) const
            {
                erase(container, container.cbegin());
            }
        };

        struct mem_fn
        {
            template<containers::sequence_container Container>
                requires requires(Container instance) { instance.pop_front(); }
            constexpr void operator()(Container& container) const { return container.pop_front(); }
        };

    public:
        template<
            typename... Args,
            ::std::invocable<Args...> Fn = // clang-format off
                functional::sequenced_invocables<mem_fn, default_fn>
        > // clang-format on
        constexpr decltype(auto) operator()(Args&&... args) const
        {
            Fn{}(::std::forward<Args>(args)...);
        }
    } pop_front{};

    inline constexpr struct pop_back_fn
    {
    private:
        struct default_fn
        {
            template<typename Container>
                requires ::std::
                    invocable<actions::erase_fn, Container&, ranges::const_iterator_t<Container>>
            constexpr void operator()(Container& container) const
            {
                erase(container, container.cbegin());
            }
        };

        struct mem_fn
        {
            template<containers::sequence_container Container>
                requires requires(Container instance) { instance.pop_back(); }
            constexpr void operator()(Container& container) const { return container.pop_back(); }
        };

    public:
        template<
            typename... Args,
            ::std::invocable<Args...> Fn = // clang-format off
                functional::sequenced_invocables<mem_fn, default_fn>
        > // clang-format on
        constexpr decltype(auto) operator()(Args&&... args) const
        {
            Fn{}(::std::forward<Args>(args)...);
        }
    } pop_back{};

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
                noexcept(reserved<Container, sizeof...(Args)>()))
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

#include "../details/epilogue.h"