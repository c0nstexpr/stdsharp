//
// Created by BlurringShadow on 2021-10-6.
//

#pragma once
#include <range/v3/action.hpp>

#include "tuple/tuple.h"
#include "containers/containers.h"

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

        struct emplace_fn
        {
            template<typename... Args>
            constexpr decltype(auto) operator()(
                seq_emplace_req<Args...> auto& container,
                const decltype(container.cbegin()) iter,
                Args&&... args //
            ) const
            {
                return container.emplace(iter, ::std::forward<Args>(args)...);
            }

            template<associative_like_req Container, typename... Args>
                requires container_emplace_constructible<Container, Args...>
            constexpr decltype(auto) operator()(Container& container, Args&&... args) const
            {
                return container.emplace(::std::forward<Args>(args)...);
            }
        };

        struct erase_fn
        {
            template<containers::sequence_container Container>
                requires containers::container_erasable<Container>
            constexpr auto operator()(
                Container& container,
                const ::std::equality_comparable_with<
                    typename ::std::decay_t<Container>::value_type> auto& value //
            ) const
            {
                return erase(container, value);
            }

            template<associative_like_req Container>
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
                typename Container,
                ::std::convertible_to<
                    ranges::const_iterator_t<Container>>... ConstIter // clang-format off
            > // clang-format on
                requires container_erasable<Container> && requires
                {
                    requires(sequence_container<Container> || associative_like_req<Container>);
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
    }

    inline constexpr struct emplace_fn
    {
        template<typename... Args>
            requires(
                !functional::cpo_invocable<emplace_fn, Args...> &&
                ::std::invocable<details::emplace_fn, Args...> //
            )
        constexpr decltype(auto) operator()(Args&&... args) const //
        {
            return details::emplace_fn{}(::std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires functional::cpo_invocable<emplace_fn, Args...>
        constexpr decltype(auto) operator()(Args&&... args) const
        {
            return functional::cpo(*this, ::std::forward<Args>(args)...);
        }
    } emplace{};

    inline constexpr struct erase_fn
    {
        template<typename... Args>
            requires(
                ::std::invocable<details::erase_fn, Args...> &&
                !functional::cpo_invocable<erase_fn, Args...>)
        constexpr decltype(auto) operator()(Args&&... args) const
        {
            return details::erase_fn{}(::std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires functional::cpo_invocable<erase_fn, Args...>
        constexpr decltype(auto) operator()(Args&&... args) const
        {
            return functional::cpo(*this, ::std::forward<Args>(args)...);
        }
    } erase{};

    namespace details
    {
        struct emplace_back_default_fn
        {
            template<typename Container, typename... Args>
                requires ::std::invocable<
                    actions::emplace_fn,
                    Container&,
                    ranges::const_iterator_t<Container>,
                    Args... // clang-format off
                > // clang-format on
            constexpr decltype(auto) operator()(Container& container, Args&&... args) const
            {
                return *emplace(container, container.cend(), ::std::forward<Args>(args)...);
            }
        };

        struct emplace_back_mem_fn
        {
            template<typename... Args, seq_emplace_req<Args...> Container>
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

        struct emplace_front_default_fn
        {
            template<typename Container, typename... Args>
                requires ::std::invocable<
                    actions::emplace_fn,
                    Container&,
                    ranges::const_iterator_t<Container>,
                    Args... // clang-format off
                > // clang-format on
            constexpr decltype(auto) operator()(Container& container, Args&&... args) const
            {
                return *emplace(container, container.cend(), ::std::forward<Args>(args)...);
            }
        };

        struct emplace_front_mem_fn
        {
            template<typename... Args, seq_emplace_req<Args...> Container>
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

        struct resize_fn
        {
            template<typename Container>
            using size_type = ::std::ranges::range_size_t<Container>;

            template<sequence_container Container>
                requires requires(Container container, size_type<Container> size)
                {
                    container.resize(size);
                }
            constexpr void operator()(Container& container, const size_type<Container> size) const
            {
                return container.resize(size);
            }
        };
    }

    inline constexpr struct emplace_back_fn :
        functional::
            sequenced_invocables<details::emplace_back_mem_fn, details::emplace_back_default_fn>
    {
    } emplace_back{};

    inline constexpr struct emplace_front_fn :
        functional::
            sequenced_invocables<details::emplace_front_mem_fn, details::emplace_front_default_fn>
    {
    } emplace_front{};

    inline constexpr struct erase_if_fn
    {
        template<typename Container, containers::container_predicatable<Container> Predicate>
            requires requires
            {
                erase_if(::std::declval<Container&>(), ::std::declval<Predicate>());
            }
        constexpr ::std::ranges::range_size_t<Container>
            operator()(Container& container, Predicate&& predicate_fn) const
        {
            return erase_if(container, ::std::forward<Predicate>(predicate_fn));
        }

        template<typename Container, containers::container_predicatable<Container> Predicate>
        constexpr auto operator()(Container& container, Predicate predicate_fn) const
        {
            const auto& it = static_cast<ranges::const_iterator_t<Container>>(
                ::std::remove_if(container.begin(), container.end(), predicate_fn) //
            );
            const auto r = ::std::distance(it, container.cend());
            erase(container, it, container.cend());
            return r;
        }

    } erase_if{};

    inline constexpr struct resize_fn
    {
        template<typename... Args>
            requires(
                ::std::invocable<details::resize_fn, Args...> &&
                !functional::cpo_invocable<resize_fn, Args...>)
        constexpr decltype(auto) operator()(Args&&... args) const
        {
            return details::resize_fn{}(::std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires functional::cpo_invocable<resize_fn, Args...>
        constexpr decltype(auto) operator()(Args&&... args) const
        {
            return functional::cpo(*this, ::std::forward<Args>(args)...);
        }
    } resize{};

    namespace details
    {
        struct pop_front_default_fn
        {
            template<typename Container>
                requires ::std::
                    invocable<actions::erase_fn, Container&, ranges::const_iterator_t<Container>>
            constexpr void operator()(Container& container) const
            {
                erase(container, container.cbegin());
            }
        };

        struct pop_front_mem_fn
        {
            template<sequence_container Container>
                requires requires(Container instance) { instance.pop_front(); }
            constexpr void operator()(Container& container) const { return container.pop_front(); }
        };

        struct pop_back_default_fn
        {
            template<typename Container>
                requires ::std::
                    invocable<actions::erase_fn, Container&, ranges::const_iterator_t<Container>>
            constexpr void operator()(Container& container) const
            {
                erase(container, container.cbegin());
            }
        };

        struct pop_back_mem_fn
        {
            template<sequence_container Container>
                requires requires(Container instance) { instance.pop_back(); }
            constexpr void operator()(Container& container) const { return container.pop_back(); }
        };
    }

    inline constexpr struct pop_front_fn :
        functional::sequenced_invocables<details::pop_front_mem_fn, details::pop_front_default_fn>
    {
    } pop_front{};

    inline constexpr struct pop_back_fn :
        functional::sequenced_invocables<details::pop_back_mem_fn, details::pop_back_default_fn>
    {
    } pop_back{};

    namespace details
    {
        template<typename Container>
        struct direct_make_container_fn
        {
            template<typename... Args>
                requires ::std::constructible_from<Container, Args...> ||
                    ::std::constructible_from<Container, Args...>
            constexpr auto operator()(Args&&... args) const { return Container{args...}; }

            template<
                typename... Args,
                typename List = decltype(::std::initializer_list{::std::declval<Args>()...})>
                requires ::std::constructible_from<Container, Args...> ||
                    ::std::constructible_from<Container, Args...>
            constexpr auto operator()(Args&&... args) const { return Container{args...}; }
        };
    }

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
