//
// Created by BlurringShadow on 2021-10-6.
//

#pragma once
#include <range/v3/action.hpp>

#include "functional/cpo.h"
#include "functional/trivial_overload.h"
#include "functional/functional.h"
#include "containers/containers.h"

// TODO: MSVC ADL bug
// move definition to "functional" namespace
namespace stdsharp::details
{
    struct adl_erase_fn
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
    };

    struct adl_erase_if_fn
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
    };
}

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

        struct seq_emplace_fn
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
        };

        struct associative_like_emplace_fn
        {
            template<associative_like_req Container, typename... Args>
                requires container_emplace_constructible<Container, Args...>
            constexpr decltype(auto) operator()(Container& container, Args&&... args) const
            {
                return container.emplace(::std::forward<Args>(args)...);
            }
        };

        // TODO: MSVC unknown bug
        // combine these two emplace fn into one
        using emplace_fn =
            functional::trivial_overload<seq_emplace_fn, associative_like_emplace_fn>;

        struct default_erase_fn
        {
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

        using erase_fn =
            functional::trivial_overload<default_erase_fn, stdsharp::details::adl_erase_fn>;
    }

    inline constexpr struct emplace_fn
    {
        template<typename... Args>
            requires(
                ::std::invocable<details::emplace_fn, Args...> &&
                !functional::cpo_invocable<emplace_fn, Args...>)
        constexpr decltype(auto) operator()(Args&&... args) const
        {
            return details::emplace_fn{}(::std::forward<Args>(args)...);
        }

        // template<typename... Args>
        //     requires(!functional::cpo_invocable<emplace_fn, Args...>)
        // constexpr decltype(auto) operator()(
        //     details::seq_emplace_req<Args...> auto& container,
        //     const decltype(container.cbegin()) iter,
        //     Args&&... args //
        // ) const
        // {
        //     return container.emplace(iter, ::std::forward<Args>(args)...);
        // }

        // template<details::associative_like_req Container, typename... Args>
        //     requires(
        //         containers::container_emplace_constructible<Container, Args...> &&
        //         !functional::cpo_invocable<emplace_fn, Args...>)
        // constexpr decltype(auto) operator()(Container& container, Args&&... args) const
        // {
        //     return container.emplace(::std::forward<Args>(args)...);
        // }

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
        ::ranges::overloaded<details::emplace_back_mem_fn, details::emplace_back_default_fn>
    {
    } emplace_back{};

    inline constexpr struct emplace_front_fn :
        ::ranges::overloaded<details::emplace_front_mem_fn, details::emplace_front_default_fn>
    {
    } emplace_front{};

    namespace details
    {
        struct erase_if_fn
        {
            template<typename Container, container_predicatable<Container> Predicate>
            constexpr auto operator()(Container& container, Predicate predicate_fn) const
            {
                const auto& it = static_cast<ranges::const_iterator_t<Container>>(
                    ::std::remove_if(container.begin(), container.end(), predicate_fn) //
                );
                const auto r = ::std::distance(it, container.cend());
                erase(container, it, container.cend());
                return r;
            }
        };
    }

    inline constexpr struct erase_if_fn :
        ::ranges::overloaded<stdsharp::details::adl_erase_if_fn, details::erase_if_fn>
    {
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
        ::ranges::overloaded<details::pop_front_mem_fn, details::pop_front_default_fn>
    {
    } pop_front{};

    inline constexpr struct pop_back_fn :
        ::ranges::overloaded<details::pop_back_mem_fn, details::pop_back_default_fn>
    {
    } pop_back{};

    template<typename Container>
    struct make_container_fn
    {
    private:
        template<::std::size_t Count>
        static constexpr auto reserved(Container& container) noexcept(noexcept( //
            functional::optional_invoke(
                []() noexcept(noexcept(container.reserve(Count))) requires requires {
                    container.reserve(Count);
                } {
                    ::std::declval<Container&>().reserve(Count); //
                } // clang-format off
                )
            ) // clang-format on
        )
        {
            functional::optional_invoke(
                [&container]() noexcept(noexcept(container.reserve(Count))) requires requires {
                    container.reserve(Count);
                } {
                    container.reserve(Count); //
                } //
            );
        }

    public:
        template<typename... Args>
            requires(::std::invocable<actions::emplace_back_fn, Container&, Args>&&...)
        constexpr auto operator()(Args&&... args) const noexcept(
            (concepts:: // clang-format off
                nothrow_invocable<actions::emplace_back_fn, Container&, Args> &&...) &&
            noexcept(reserved<Container, sizeof...(Args)>())
        ) // clang-format on
        {
            Container container{};
            reserved<sizeof...(Args)>(container);
            (emplace_back(container, ::std::forward<Args>(args)), ...);
            return container;
        }

        template<typename... Tuples>
            requires requires(Container c)
            {
                (
                    []<typename... Args>(::std::tuple<Args...>, Container c) {
                        c.emplace_back(::std::declval<Args>()...);
                    }(::std::declval<Tuples>(), c),
                    ...);
            }
        constexpr auto operator()(const ::std::piecewise_construct_t, Tuples&&... tuples) const
        {
            Container container{};
            const auto& fn = functional::bind_ref_front(emplace_back, container);

            reserved<sizeof...(Tuples)>(container);
            (::std::apply(fn, ::std::forward<Tuples>(tuples)), ...);
            return container;
        }
    };

    template<typename Container>
    inline constexpr make_container_fn<Container> make_container{};
}
