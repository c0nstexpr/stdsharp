//
// Created by BlurringShadow on 2021-10-6.
//

#pragma once
#include <range/v3/action.hpp>

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
        template<
            typename... Args,
            actions::details::seq_emplace_req<Args...> Container // clang-format off
        > // clang-format on
        constexpr decltype(auto) operator()(
            Container& container,
            const decltype(container.cbegin()) iter,
            Args&&... args //
        ) const
        {
            return container.emplace(iter, ::std::forward<Args>(args)...);
        }

        template<
            typename... Args,
            container_emplace_constructible<Args...> Container // clang-format off
        > // clang-format on
            requires actions::details::associative_like_req<Container>
        constexpr decltype(auto) operator()(Container& container, Args&&... args) const
        {
            return container.emplace(::std::forward<Args>(args)...);
        }
    };

    inline constexpr auto emplace = functional::tagged_cpo<actions::emplace_fn>;

    namespace details
    {
        struct emplace_back_default_fn
        {
            template<typename Container, typename... Args>
                requires ::std::invocable<
                    decltype(actions::emplace),
                    Container&,
                    ranges::const_iterator_t<Container>,
                    Args... // clang-format off
                > // clang-format on
            constexpr decltype(auto) operator()(Container& container, Args&&... args) const
            {
                return *actions:: //
                    emplace(container, container.cend(), ::std::forward<Args>(args)...);
            }
        };

        struct emplace_back_mem_fn
        {
            template<
                typename... Args,
                actions::details::seq_emplace_req<Args...> Container
                // clang-format off
            > // clang-format on
                requires requires(Container instance, Args&&... args)
                { // clang-format off
                    { instance.emplace_back(::std::forward<Args>(args)...) } ->
                        ::std::same_as<typename ::std::decay_t<Container>::reference>;
                } // clang-format on
            constexpr decltype(auto) operator()(Container& container, Args&&... args) const
            {
                return container.emplace_back(::std::forward<Args>(args)...);
            }
        };
    }

    struct emplace_back_fn :
        ::ranges::overloaded<
            actions::details::emplace_back_mem_fn,
            actions::details::emplace_back_default_fn // clang-format off
        > // clang-format on
    {
    };

    inline constexpr auto emplace_back = functional::tagged_cpo<actions::emplace_back_fn>;

    namespace details
    {
        struct emplace_front_default_fn
        {
            template<typename Container, typename... Args>
                requires ::std::invocable<
                    decltype(actions::emplace),
                    Container&,
                    ranges::const_iterator_t<Container>,
                    Args... // clang-format off
                > // clang-format on
            constexpr decltype(auto) operator()(Container& container, Args&&... args) const
            {
                return *actions::emplace(
                    container, container.cend(), ::std::forward<Args>(args)...);
            }
        };

        struct emplace_front_mem_fn
        {
            template<
                typename... Args,
                actions::details::seq_emplace_req<Args...> Container
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
            actions::details::emplace_front_mem_fn,
            actions::details::emplace_front_default_fn // clang-format off
        > // clang-format on
    {
    };

    inline constexpr auto emplace_front = functional::tagged_cpo<actions::emplace_front_fn>;

    struct erase_fn
    {
        template<
            actions::details::associative_like_req Container,
            ::std::equality_comparable_with<typename ::std::decay_t<Container>::key_type>
                KeyType // clang-format off
        > // clang-format on
        constexpr auto operator()(Container& container, const KeyType& key) const
        {
            return container.erase(key);
        }

        template<
            sequence_container Container,
            ::std::equality_comparable_with<typename ::std::decay_t<Container>::value_type>
                ValueType // clang-format off
        > // clang-format on
            requires(!stdsharp::containers::details::std_array<Container>)
        constexpr auto operator()(Container& container, const ValueType& value) const
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
                    actions::details::associative_like_req<Container>);
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

    inline constexpr auto erase = functional::tagged_cpo<actions::erase_fn>;

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

    inline constexpr auto erase_if = functional::tagged_cpo<actions::erase_if_fn>;

    namespace details
    {
        struct pop_front_default_fn
        {
            template<typename Container>
                requires ::std::invocable<
                    decltype(actions::erase),
                    Container&,
                    ranges::const_iterator_t<Container> // clang-format off
                > // clang-format on
            constexpr void operator()(Container& container) const noexcept( //
                concepts::nothrow_invocable<
                    decltype(actions::erase),
                    Container&,
                    decltype(container.cbegin()) // clang-format off
                > // clang-format on
            )
            {
                actions::erase(container, container.cbegin());
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
            actions::details::pop_front_mem_fn,
            actions::details::pop_front_default_fn // clang-format off
        > // clang-format on 
    {
    };

    inline constexpr auto pop_front = functional::tagged_cpo<actions::pop_front_fn>;

    namespace details
    {
        struct pop_back_default_fn
        {
            template<typename Container>
                requires ::std::invocable<
                    decltype(actions::erase),
                    Container&,
                    ranges::const_iterator_t<Container> // clang-format off
                > // clang-format on
            constexpr void operator()(Container& container) const noexcept( //
                concepts::nothrow_invocable<
                    decltype(actions::erase),
                    Container&,
                    decltype(container.cbegin()) // clang-format off
                > // clang-format on
            )
            {
                actions::erase(container, container.cbegin());
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
            actions::details::pop_back_mem_fn,
            actions::details::pop_back_default_fn // clang-format off
        > // clang-format on 
    {
    };

    inline constexpr auto pop_back = functional::tagged_cpo<actions::pop_back_fn>;

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

    inline constexpr auto resize = functional::tagged_cpo<actions::resize_fn>;

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
            requires(::std::invocable<decltype(actions::emplace), Container&, Args>&&...)
        constexpr auto operator()(Args&&... args) const noexcept(
            concepts::nothrow_default_initializable<Container> &&
            (concepts::nothrow_invocable<decltype(actions::emplace), Container&, Args> && ...) && //
            noexcept(make_container_fn::reserved<Container, sizeof...(Args)>()) //
        )
        {
            Container container{};
            make_container_fn::reserved<sizeof...(Args)>(container);
            (actions::emplace(container, ::std::forward<Args>(args)), ...);
            return container;
        }

        template<typename... Args>
            requires(::std::invocable<decltype(actions::emplace_back), Container&, Args>&&...)
        constexpr auto operator()(Args&&... args) const noexcept(
            (concepts:: // clang-format off
                nothrow_invocable<decltype(actions::emplace_back), Container&, Args> &&...) &&
            noexcept(make_container_fn::reserved<Container, sizeof...(Args)>())) // clang-format on
        {
            Container container{};
            make_container_fn::reserved<sizeof...(Args)>(container);
            (actions::emplace_back(container, ::std::forward<Args>(args)), ...);
            return container;
        }
    };

    template<typename Container>
    inline constexpr auto make_container =
        functional::tagged_cpo<actions::make_container_fn<Container>>;
}
