//
// Created by BlurringShadow on 2021-10-6.
//

#pragma once
#include "containers/containers.h"

namespace stdsharp::containers::actions
{
    namespace details
    {
        template<typename Container, typename... Args>
        concept seq_emplace_req = ::stdsharp::containers::sequence_container<Container> &&
            ::stdsharp::containers::container_emplace_constructible<Container, Args...>;

        template<typename Container, typename... Args>
        concept seq_mem_emplace_back_req =
            ::stdsharp::containers::actions::details::seq_emplace_req<Container, Args...> &&
            requires(Container instance, Args&&... args) // clang-format off
            {
                { instance.emplace_back(::std::forward<Args>(args)...) } ->
                    ::std::same_as<typename ::std::decay_t<Container>::reference>;
            }; // clang-format on

        template<typename Container, typename... Args>
        concept seq_mem_emplace_front_req =
            ::stdsharp::containers::actions::details::seq_emplace_req<Container, Args...> &&
            requires(Container instance, Args&&... args) // clang-format off
            {
                { instance.emplace_front(::std::forward<Args>(args)...) } ->
                    ::std::same_as<typename ::std::decay_t<Container>::reference>;
            }; // clang-format on

        template<typename Container>
        concept seq_mem_pop_back_req = ::stdsharp::containers::sequence_container<Container> &&
            requires(Container instance) // clang-format off
            {
                { instance.pop_back() } -> ::std::same_as<void>;
            }; // clang-format on

        template<typename Container>
        concept seq_mem_pop_front_req = ::stdsharp::containers::sequence_container<Container> &&
            requires(Container instance) // clang-format off
            {
                { instance.pop_front() } -> ::std::same_as<void>;
            }; // clang-format on

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

    struct emplace_back_fn
    {
        template<
            typename Container,
            typename... Args // clang-format off
        > // clang-format on
            requires(
                !::stdsharp::containers::actions::details::
                    seq_mem_emplace_back_req<Container, Args...> &&
                ::std::invocable<
                    ::stdsharp::containers::actions::emplace_fn,
                    Container&,
                    ::stdsharp::ranges::const_iterator_t<Container>,
                    Args... // clang-format off
                >
            ) // clang-format on
        constexpr decltype(auto) operator()(Container& container, Args&&... args) const noexcept( //
            ::stdsharp::concepts::nothrow_invocable<
                ::stdsharp::containers::actions::emplace_fn,
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

        template<
            typename... Args,
            ::stdsharp::containers::actions::details::seq_mem_emplace_back_req<Args...>
                Container // clang-format off
        > // clang-format on
        constexpr decltype(auto) operator()(Container& container, Args&&... args) const
            noexcept(noexcept(container.emplace_back(::std::forward<Args>(args)...)))
        {
            return container.emplace_back(::std::forward<Args>(args)...);
        }
    };

    inline constexpr auto emplace_back =
        ::stdsharp::functional::tagged_cpo<::stdsharp::containers::actions::emplace_back_fn>;

    struct emplace_front_fn
    {
    public:
        template<
            typename Container,
            typename... Args // clang-format off
        > // clang-format on
            requires(
                !::stdsharp::containers::actions::details::
                    seq_mem_emplace_front_req<Container, Args...> &&
                ::std::invocable<
                    ::stdsharp::containers::actions::emplace_fn,
                    Container&,
                    ::stdsharp::ranges::const_iterator_t<Container>,
                    Args... // clang-format off
                >
            ) // clang-format on
        constexpr decltype(auto) operator()(Container& container, Args&&... args) const noexcept( //
            ::stdsharp::concepts::nothrow_invocable<
                ::stdsharp::containers::actions::emplace_fn,
                Container&,
                decltype(container.cbegin()),
                Args... // clang-format off
            > && // clang-format on
            noexcept(*container.cbegin()) //
        )
        {
            return *::stdsharp::containers::actions:: //
                emplace(container, container.cbegin(), ::std::forward<Args>(args)...);
        }

        template<
            typename... Args,
            ::stdsharp::containers::actions::details:: // clang-format off
                seq_mem_emplace_front_req<Args...> Container
        > // clang-format on
        constexpr decltype(auto) operator()(Container& container, Args&&... args) const
            noexcept(noexcept(container.emplace_front(::std::forward<Args>(args)...)))
        {
            return container.emplace_front(::std::forward<Args>(args)...);
        }
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
        constexpr typename ::std::decay_t<Container>::size_type
            operator()(Container& container, const KeyType& key) const
            noexcept(noexcept(container.erase(key)))
        {
            return container.erase(key);
        }

        template<
            ::stdsharp::containers::sequence_container Container,
            ::std::equality_comparable_with<typename ::std::decay_t<Container>::value_type>
                ValueType // clang-format off
        > // clang-format on
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
            ::std::predicate<::std::ranges::range_value_t<Container>> Predicate
            // clang-format off
        > // clang-format on
        constexpr auto operator()(Container& container, Predicate&& predicate_fn) const
            noexcept(noexcept(::std::erase_if(container, ::std::forward<Predicate>(predicate_fn))))
        {
            return ::std::erase_if(container, ::std::forward<Predicate>(predicate_fn));
        }
    };

    inline constexpr auto erase_if =
        ::stdsharp::functional::tagged_cpo<::stdsharp::containers::actions::erase_if_fn>;

    struct pop_front_fn
    {
        template<typename Container>
            requires(
                !::stdsharp::containers::actions::details::seq_mem_pop_front_req<Container> &&
                ::std::invocable<
                    ::stdsharp::containers::actions::erase_fn,
                    Container&,
                    ::stdsharp::ranges::const_iterator_t<Container> // clang-format off
                > // clang-format on
            )
        constexpr void operator()(Container& container) const noexcept( //
            ::stdsharp::concepts::nothrow_invocable<
                ::stdsharp::containers::actions::erase_fn,
                Container&,
                decltype(container.cbegin()) // clang-format off
            > // clang-format on
        )
        {
            ::stdsharp::containers::actions::erase(container, container.cbegin());
        }

        template<::stdsharp::containers::actions::details::seq_mem_pop_front_req Container>
        constexpr void operator()(Container& container) const
            noexcept(noexcept(container.pop_front()))
        {
            container.pop_front();
        }
    };

    inline constexpr auto pop_front =
        ::stdsharp::functional::tagged_cpo<::stdsharp::containers::actions::pop_front_fn>;

    struct pop_back_fn
    {
        template<typename Container>
            requires(
                !::stdsharp::containers::actions::details::seq_mem_pop_back_req<Container> &&
                ::std::invocable<
                    ::stdsharp::containers::actions::erase_fn,
                    Container&,
                    ::stdsharp::ranges::const_iterator_t<Container> // clang-format off
                > // clang-format on
            )
        constexpr void operator()(Container& container) const noexcept( //
            ::stdsharp::concepts::nothrow_invocable<
                ::stdsharp::containers::actions::erase_fn,
                Container&,
                decltype(container.cbegin()) // clang-format off
            > // clang-format on
        )
        {
            ::stdsharp::containers::actions::erase(container, container.cend());
        }

        template<::stdsharp::containers::actions::details::seq_mem_pop_back_req Container>
        constexpr void operator()(Container& container) const
            noexcept(noexcept(container.pop_back()))
        {
            container.pop_back();
        }
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
}
