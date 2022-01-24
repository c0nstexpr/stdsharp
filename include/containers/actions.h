//
// Created by BlurringShadow on 2021-10-6.
//

#pragma once
#include <range/v3/action.hpp>

#include "functional/cpo.h"
#include "functional/functional.h"
#include "containers/containers.h"

namespace stdsharp::containers::actions
{
    namespace details
    {
        template<typename Container, typename... Args>
        concept seq_emplace_req =
            sequence_container<Container> && container_emplace_constructible<Container, Args...>;

        template<typename Container>
        concept associative_like_req =
            associative_container<Container> || unordered_associative_container<Container>;
    }

    struct emplace_fn
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
            requires container_emplace_constructible<Container, Args...>
        constexpr decltype(auto) operator()(Container& container, Args&&... args) const
        {
            return container.emplace(::std::forward<Args>(args)...);
        }
    };

    inline constexpr functional::cpo_fn<emplace_fn> emplace{};

    namespace details
    {
        struct emplace_back_default_fn
        {
            template<typename Container, typename... Args>
                requires ::std::invocable<
                    decltype(emplace),
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
    }

    struct emplace_back_fn :
        ::ranges::overloaded<
            details::emplace_back_mem_fn,
            details::emplace_back_default_fn // clang-format off
        > // clang-format on
    {
    };

    inline constexpr functional::cpo_fn<emplace_back_fn> emplace_back{};

    namespace details
    {
        struct emplace_front_default_fn
        {
            template<typename Container, typename... Args>
                requires ::std::invocable<
                    decltype(emplace),
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
            template<
                typename... Args,
                details::seq_emplace_req<Args...> Container
                // clang-format off
            > // clang-format on
                requires requires(Container instance, Args&&... args)
                { // clang-format off
                    { instance.emplace_front(::std::forward<Args>(args)...) } ->
                        ::std::same_as<typename ::std::decay_t<Container>::reference>;
                } // clang-format on
            constexpr decltype(auto) operator()(Container& container, Args&&... args) const
            {
                return container.emplace_front(::std::forward<Args>(args)...);
            }
        };
    }

    struct emplace_front_fn :
        ::ranges::overloaded<
            details::emplace_front_mem_fn,
            details::emplace_front_default_fn // clang-format off
        > // clang-format on
    {
    };

    inline constexpr functional::cpo_fn<emplace_front_fn> emplace_front;

    struct erase_fn
    {
        template<details::associative_like_req Container>
        constexpr auto operator()(
            Container& container,
            const ::std::equality_comparable_with<
                typename ::std::decay_t<Container>::key_type // clang-format off
            > auto& key // clang-format on
        ) const
        {
            return container.erase(key);
        }

        template<sequence_container Container>
            requires(!stdsharp::containers::details::std_array<Container>)
        constexpr auto operator()(
            Container& container,
            const ::std::equality_comparable_with<
                typename ::std::decay_t<Container>::value_type // clang-format off
            > auto& value // clang-format on
        ) const
        {
            return ::std::erase(container, value);
        }

        template<
            typename Container,
            ::std::convertible_to<ranges::const_iterator_t<Container>>... ConstIter
            // clang-format off
        > // clang-format on
            requires requires
            {
                requires(
                    !stdsharp::containers::details::std_array<Container> &&
                        sequence_container<Container> ||
                    details::associative_like_req<Container>);
                requires sizeof...(ConstIter) <= 1;
            }
        constexpr auto operator()(
            Container& container,
            const decltype(container.cbegin()) const_iter_begin,
            const ConstIter... const_iter_end //
        ) const noexcept(noexcept(container.erase(const_iter_begin, const_iter_end...)))
        {
            return container.erase(
                const_iter_begin,
                static_cast<ranges::const_iterator_t<Container>>(const_iter_end)... //
            );
        }
    };

    inline constexpr functional::cpo_fn<erase_fn> erase;

    struct erase_if_fn
    {
        template<
            typename Container,
            ::std::predicate<::std::ranges::range_value_t<Container>> Predicate // clang-format off
        > // clang-format on
            requires requires(Container container, Predicate&& predicate_fn)
            {
                ::std::erase_if(container, ::std::forward<Predicate>(predicate_fn));
            }
        constexpr auto operator()(Container& container, Predicate&& predicate_fn) const
        {
            return ::std::erase_if(container, ::std::forward<Predicate>(predicate_fn));
        }
    };

    inline constexpr functional::cpo_fn<erase_if_fn> erase_if{};

    namespace details
    {
        struct pop_front_default_fn
        {
            template<typename Container>
                requires ::std::invocable<
                    decltype(erase),
                    Container&,
                    ranges::const_iterator_t<Container> // clang-format off
                > // clang-format on
            constexpr void operator()(Container& container) const noexcept( //
                concepts::nothrow_invocable<
                    decltype(erase),
                    Container&,
                    decltype(container.cbegin()) // clang-format off
                > // clang-format on
            )
            {
                erase(container, container.cbegin());
            }
        };

        struct pop_front_mem_fn
        {
            template<sequence_container Container>
                requires requires(Container instance) // clang-format off
                {
                    { instance.pop_front() } -> ::std::same_as<void>;
                } // clang-format on
            constexpr void operator()(Container& container) const
                noexcept(noexcept(container.pop_front()))
            {
                container.pop_front();
            }
        };
    }

    struct pop_front_fn :
        ::ranges::overloaded<
            details::pop_front_mem_fn,
            details::pop_front_default_fn // clang-format off
        > // clang-format on 
    {
    };

    inline constexpr functional::cpo_fn<pop_front_fn> pop_front{};

    namespace details
    {
        struct pop_back_default_fn
        {
            template<typename Container>
                requires ::std::invocable<
                    decltype(erase),
                    Container&,
                    ranges::const_iterator_t<Container> // clang-format off
                > // clang-format on
            constexpr void operator()(Container& container) const noexcept( //
                concepts::nothrow_invocable<
                    decltype(erase),
                    Container&,
                    decltype(container.cbegin()) // clang-format off
                > // clang-format on
            )
            {
                erase(container, container.cbegin());
            }
        };

        struct pop_back_mem_fn
        {
            template<sequence_container Container>
                requires requires(Container instance) // clang-format off
                {
                    { instance.pop_back() } -> ::std::same_as<void>;
                } // clang-format on
            constexpr void operator()(Container& container) const
                noexcept(noexcept(container.pop_back()))
            {
                container.pop_back();
            }
        };
    }

    struct pop_back_fn :
        ::ranges::overloaded<
            details::pop_back_mem_fn,
            details::pop_back_default_fn // clang-format off
        > // clang-format on 
    {
    };

    inline constexpr functional::cpo_fn<pop_back_fn> pop_back;

    struct resize_fn
    {
        template<typename Container>
        using size_type = ::std::ranges::range_size_t<Container>;

        template<sequence_container Container>
            requires requires(Container container, size_type<Container> size)
            { // clang-format off
                { container.resize(size) } -> ::std::same_as<void>; // clang-format on
            }
        constexpr void operator()(Container& container, const size_type<Container> size) const
        {
            container.resize(size);
        }
    };

    inline constexpr functional::cpo_fn<resize_fn> resize{};

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
            requires(::std::invocable<decltype(emplace), Container&, Args>&&...)
        constexpr auto operator()(Args&&... args) const noexcept(
            concepts::nothrow_default_initializable<Container> &&
            (concepts::nothrow_invocable<decltype(emplace), Container&, Args> && ...) && //
            noexcept(make_container_fn::reserved<Container, sizeof...(Args)>()) //
        )
        {
            Container container{};
            make_container_fn::reserved<sizeof...(Args)>(container);
            (emplace(container, ::std::forward<Args>(args)), ...);
            return container;
        }

        template<typename... Args>
            requires(::std::invocable<decltype(emplace_back), Container&, Args>&&...)
        constexpr auto operator()(Args&&... args) const noexcept(
            (concepts:: // clang-format off
                nothrow_invocable<decltype(emplace_back), Container&, Args> &&...) &&
            noexcept(make_container_fn::reserved<Container, sizeof...(Args)>())) // clang-format on
        {
            Container container{};
            make_container_fn::reserved<sizeof...(Args)>(container);
            (emplace_back(container, ::std::forward<Args>(args)), ...);
            return container;
        }
    };

    template<typename Container>
    inline constexpr functional::cpo_fn<make_container_fn<Container>> make_container{};
}
