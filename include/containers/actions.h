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
        concept seq_emplace_req = ::stdsharp::containers::sequence_container<Container> &&
            ::stdsharp::containers::container_emplace_constructible<Container, Args...>;

        template<typename Container>
        concept associative_like_req = ::stdsharp::containers::associative_container<Container> ||
            ::stdsharp::containers::unordered_associative_container<Container>;
    }

    struct emplace_fn
    {
        template<
            typename... Args,
            ::stdsharp::containers::actions::details::seq_emplace_req<Args...>
                Container // clang-format off
        > // clang-format on
        constexpr decltype(auto) operator()(
            Container& container,
            const decltype(container.cbegin()) iter,
            Args&&... args //
        ) const noexcept(noexcept(container.emplace(iter, ::std::forward<Args>(args)...)))
        {
            return container.emplace(iter, ::std::forward<Args>(args)...);
        }

        template<
            typename... Args,
            ::stdsharp::containers::container_emplace_constructible<Args...>
                Container // clang-format off
        > // clang-format on
            requires ::stdsharp::containers::actions::details::associative_like_req<Container>
        constexpr decltype(auto) operator()(Container& container, Args&&... args) const
            noexcept(noexcept(container.emplace(::std::forward<Args>(args)...)))
        {
            return container.emplace(::std::forward<Args>(args)...);
        }
    };

    inline constexpr auto emplace =
        ::stdsharp::functional::tagged_cpo<::stdsharp::containers::actions::emplace_fn>;

    namespace details
    {
        struct emplace_back_default_fn
        {
            template<typename Container, typename... Args>
                requires ::std::invocable<
                    decltype(::stdsharp::containers::actions::emplace),
                    Container&,
                    ::stdsharp::ranges::const_iterator_t<Container>,
                    Args... // clang-format off
                > // clang-format on
            constexpr decltype(auto) operator()(Container& container, Args&&... args) const
                noexcept( //
                    ::stdsharp::concepts::nothrow_invocable<
                        decltype(::stdsharp::containers::actions::emplace),
                        Container&,
                        decltype(container.cbegin()),
                        Args... // clang-format off
                > && // clang-format on
                    noexcept(*container.cbegin()) //
                )
            {
                return *::stdsharp::containers::actions:: //
                    emplace(container, container.cend(), ::std::forward<Args>(args)...);
            }
        };

        struct emplace_back_mem_fn
        {
            template<
                typename... Args,
                ::stdsharp::containers::actions::details::seq_emplace_req<Args...> Container
                // clang-format off
            > // clang-format on
                requires requires(Container instance, Args&&... args)
                { // clang-format off
                    { instance.emplace_back(::std::forward<Args>(args)...) } ->
                        ::std::same_as<typename ::std::decay_t<Container>::reference>;
                } // clang-format on
            constexpr decltype(auto) operator()(Container& container, Args&&... args) const
                noexcept(noexcept(container.emplace_back(::std::forward<Args>(args)...)))
            {
                return container.emplace_back(::std::forward<Args>(args)...);
            }
        };
    }

    struct emplace_back_fn :
        ::ranges::overloaded<
            ::stdsharp::containers::actions::details::emplace_back_mem_fn,
            ::stdsharp::containers::actions::details::emplace_back_default_fn // clang-format off
        > // clang-format on
    {
    };

    inline constexpr auto emplace_back =
        ::stdsharp::functional::tagged_cpo<::stdsharp::containers::actions::emplace_back_fn>;

    namespace details
    {
        struct emplace_front_default_fn
        {
            template<typename Container, typename... Args>
                requires ::std::invocable<
                    decltype(::stdsharp::containers::actions::emplace),
                    Container&,
                    ::stdsharp::ranges::const_iterator_t<Container>,
                    Args... // clang-format off
                > // clang-format on
            constexpr decltype(auto) operator()(Container& container, Args&&... args) const
                noexcept( //
                    ::stdsharp::concepts::nothrow_invocable<
                        decltype(::stdsharp::containers::actions::emplace),
                        Container&,
                        decltype(container.cbegin()),
                        Args... // clang-format off
                > && // clang-format on
                    noexcept(*container.cbegin()) //
                )
            {
                return *::stdsharp::containers::actions:: //
                    emplace(container, container.cend(), ::std::forward<Args>(args)...);
            }
        };

        struct emplace_front_mem_fn
        {
            template<
                typename... Args,
                ::stdsharp::containers::actions::details::seq_emplace_req<Args...> Container
                // clang-format off
            > // clang-format on
                requires requires(Container instance, Args&&... args)
                { // clang-format off
                    { instance.emplace_front(::std::forward<Args>(args)...) } ->
                        ::std::same_as<typename ::std::decay_t<Container>::reference>;
                } // clang-format on
            constexpr decltype(auto) operator()(Container& container, Args&&... args) const
                noexcept(noexcept(container.emplace_front(::std::forward<Args>(args)...)))
            {
                return container.emplace_front(::std::forward<Args>(args)...);
            }
        };
    }

    struct emplace_front_fn :
        ::ranges::overloaded<
            ::stdsharp::containers::actions::details::emplace_front_mem_fn,
            ::stdsharp::containers::actions::details::emplace_front_default_fn // clang-format off
        > // clang-format on
    {
    };

    inline constexpr auto emplace_front =
        ::stdsharp::functional::tagged_cpo<::stdsharp::containers::actions::emplace_front_fn>;

    struct erase_fn
    {
        template<
            ::stdsharp::containers::actions::details::associative_like_req Container,
            ::std::equality_comparable_with<typename ::std::decay_t<Container>::key_type>
                KeyType // clang-format off
        > // clang-format on
        constexpr auto operator()(Container& container, const KeyType& key) const
            noexcept(noexcept(container.erase(key)))
        {
            return container.erase(key);
        }

        template<
            ::stdsharp::containers::sequence_container Container,
            ::std::equality_comparable_with<typename ::std::decay_t<Container>::value_type>
                ValueType // clang-format off
        > // clang-format on
            requires(!stdsharp::containers::details::std_array<Container>)
        constexpr auto operator()(Container& container, const ValueType& value) const
            noexcept(noexcept(::std::erase(container, value)))
        {
            return ::std::erase(container, value);
        }

        template<
            typename Container,
            ::std::same_as<::stdsharp::ranges::const_iterator_t<Container>>... ConstIter
            // clang-format off
        > // clang-format on
            requires requires
            {
                requires(
                    !stdsharp::containers::details::std_array<Container> &&
                        ::stdsharp::containers::sequence_container<Container> ||
                    ::stdsharp::containers::actions::details::associative_like_req<Container>);
                requires sizeof...(ConstIter) <= 1;
            }
        constexpr auto operator()(
            Container& container,
            const decltype(container.cbegin()) const_iter_begin,
            const ConstIter... const_iter_end //
        ) const noexcept(noexcept(container.erase(const_iter_begin, const_iter_end...)))
        {
            return container.erase(const_iter_begin, const_iter_end...);
        }

        template<typename Container>
            requires(
                !stdsharp::containers::details::std_array<Container> &&
                    ::stdsharp::containers::sequence_container<Container> ||
                ::stdsharp::containers::actions::details::associative_like_req<Container>)
        constexpr auto operator()(
            Container& container,
            const decltype(container.cbegin()) const_iter_begin,
            decltype(const_iter_begin) const_iter_end //
        ) const noexcept(noexcept(container.erase(const_iter_begin, const_iter_end)))
        {
            return container.erase(const_iter_begin, const_iter_end);
        }
    };

    inline constexpr auto erase =
        ::stdsharp::functional::tagged_cpo<::stdsharp::containers::actions::erase_fn>;

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
            noexcept(noexcept(::std::erase_if(container, ::std::forward<Predicate>(predicate_fn))))
        {
            return ::std::erase_if(container, ::std::forward<Predicate>(predicate_fn));
        }
    };

    inline constexpr auto erase_if =
        ::stdsharp::functional::tagged_cpo<::stdsharp::containers::actions::erase_if_fn>;

    namespace details
    {
        struct pop_front_default_fn
        {
            template<typename Container>
                requires ::std::invocable<
                    decltype(::stdsharp::containers::actions::erase),
                    Container&,
                    ::stdsharp::ranges::const_iterator_t<Container> // clang-format off
                > // clang-format on
            constexpr void operator()(Container& container) const noexcept( //
                ::stdsharp::concepts::nothrow_invocable<
                    decltype(::stdsharp::containers::actions::erase),
                    Container&,
                    decltype(container.cbegin()) // clang-format off
                > // clang-format on
            )
            {
                ::stdsharp::containers::actions::erase(container, container.cbegin());
            }
        };

        struct pop_front_mem_fn
        {
            template<::stdsharp::containers::sequence_container Container>
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
            ::stdsharp::containers::actions::details::pop_front_mem_fn,
            ::stdsharp::containers::actions::details::pop_front_default_fn // clang-format off
        > // clang-format on 
    {
    };

    inline constexpr auto pop_front =
        ::stdsharp::functional::tagged_cpo<::stdsharp::containers::actions::pop_front_fn>;

    namespace details
    {
        struct pop_back_default_fn
        {
            template<typename Container>
                requires ::std::invocable<
                    decltype(::stdsharp::containers::actions::erase),
                    Container&,
                    ::stdsharp::ranges::const_iterator_t<Container> // clang-format off
                > // clang-format on
            constexpr void operator()(Container& container) const noexcept( //
                ::stdsharp::concepts::nothrow_invocable<
                    decltype(::stdsharp::containers::actions::erase),
                    Container&,
                    decltype(container.cbegin()) // clang-format off
                > // clang-format on
            )
            {
                ::stdsharp::containers::actions::erase(container, container.cbegin());
            }
        };

        struct pop_back_mem_fn
        {
            template<::stdsharp::containers::sequence_container Container>
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
            ::stdsharp::containers::actions::details::pop_back_mem_fn,
            ::stdsharp::containers::actions::details::pop_back_default_fn // clang-format off
        > // clang-format on 
    {
    };

    inline constexpr auto pop_back =
        ::stdsharp::functional::tagged_cpo<::stdsharp::containers::actions::pop_back_fn>;

    struct resize_fn
    {
        template<::stdsharp::containers::sequence_container Container>
            requires requires(Container container, ::std::ranges::range_size_t<Container> size)
            { // clang-format off
                { container.resize(size) } -> ::std::same_as<void>; // clang-format on
            }
        constexpr void operator()(
            Container& container,
            ::std::ranges::range_size_t<Container> size //
        ) const noexcept(noexcept(container.resize(size)))
        {
            container.resize(size);
        }
    };

    inline constexpr auto resize =
        ::stdsharp::functional::tagged_cpo<::stdsharp::containers::actions::resize_fn>;

    template<typename Container>
    struct make_container_fn
    {
    private:
        template<::std::size_t Count>
        static constexpr auto reserved(Container& container) noexcept(noexcept( //
            ::stdsharp::functional::optional_invoke(
                []() noexcept(noexcept(container.reserve(Count))) requires requires
                { container.reserve(Count); } {
                    ::std::declval<Container&>().reserve(Count); //
                } // clang-format off
                )
            ) // clang-format on
        )
        {
            ::stdsharp::functional::optional_invoke(
                [&container]() noexcept(noexcept(container.reserve(Count))) requires requires
                { container.reserve(Count); } {
                    container.reserve(Count); //
                } //
            );
        }

    public:
        template<typename... Args>
            requires(::std::invocable<
                     decltype(::stdsharp::containers::actions::emplace),
                     Container&,
                     Args //clang-format off
                     >&&...) //clang-format on
        constexpr auto operator()(Args&&... args) const noexcept(
            ::stdsharp::concepts::nothrow_default_initializable<Container> &&
            (::stdsharp::concepts::nothrow_invocable<
                 decltype(::stdsharp::containers::actions::emplace),
                 Container&,
                 Args //clang-format off
                 > &&
             ...) && noexcept(make_container_fn::
                                  reserved<Container, sizeof...(Args)>()) //clang-format on
        )
        {
            Container container{};
            make_container_fn::reserved<sizeof...(Args)>(container);
            (::stdsharp::containers::actions::emplace(container, ::std::forward<Args>(args)), ...);
            return container;
        }

        template<typename... Args>
            requires(::std::invocable<
                     decltype(::stdsharp::containers::actions::emplace_back),
                     Container&,
                     Args //clang-format off
                     >&&...) //clang-format on
        constexpr auto operator()(Args&&... args) const noexcept(
            (::stdsharp::concepts::nothrow_invocable<
                 decltype(::stdsharp::containers::actions::emplace_back),
                 Container&,
                 Args //clang-format off
                 > &&
             ...) && noexcept(make_container_fn::
                                  reserved<Container, sizeof...(Args)>()) //clang-format on
        )
        {
            Container container{};
            make_container_fn::reserved<sizeof...(Args)>(container);
            (::stdsharp::containers::actions::emplace_back(container, ::std::forward<Args>(args)),
             ...);
            return container;
        }
    };

    template<typename Container>
    inline constexpr auto make_container = ::stdsharp::functional::tagged_cpo<
        ::stdsharp::containers::actions::make_container_fn<Container>>;
}
