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
            const typename ::std::decay_t<Container>::const_iterator iter,
            Args&&... args //
        ) const noexcept(noexcept(container.emplace(iter, ::std::forward<Args>(args)...)))
        {
            return container.emplace(iter, ::std::forward<Args>(args)...);
        }

        template<typename Container, typename... Args>
            requires requires
            {
                requires(
                    ::stdsharp::containers::associative_container<Container> ||
                    ::stdsharp::containers:: // clang-format off
                        unordered_associative_container<Container>) &&
                    ::stdsharp::containers::container_emplace_constructible<
                        ::std::decay_t<Container>,
                        Args...
                    >; // clang-format on
            }
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
    public:
        template<
            typename Container,
            typename ::std::decay_t<Container>::const_iterator* ConstIter = nullptr,
            typename... Args // clang-format off
        > // clang-format on
            requires(
                !::stdsharp::containers::actions::details::
                    seq_mem_emplace_back_req<Container, Args...> &&
                ::std::invocable<
                    ::stdsharp::containers::actions::emplace_fn,
                    Container&,
                    decltype(*ConstIter),
                    Args... // clang-format off
                >
            ) // clang-format on
        constexpr decltype(auto) operator()(Container& container, Args&&... args) const noexcept( //
            ::stdsharp::concepts::nothrow_invocable<
                ::stdsharp::containers::actions::emplace_fn,
                Container&,
                decltype(*ConstIter),
                Args... // clang-format off
            > && // clang-format on
            noexcept(*::std::declval<decltype(*ConstIter)>()) //
        )
        {
            return *::stdsharp::containers::actions::emplace(
                container,
                ::std::as_const(container).end(),
                ::std::forward<Args>(args)... //
            );
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
            typename ::std::decay_t<Container>::const_iterator* ConstIter = nullptr,
            typename... Args // clang-format off
        > // clang-format on
            requires(
                !::stdsharp::containers::actions::details::
                    seq_mem_emplace_front_req<Container, Args...> &&
                ::std::invocable<
                    ::stdsharp::containers::actions::emplace_fn,
                    Container&,
                    decltype(*ConstIter),
                    Args... // clang-format off
                >
            ) // clang-format on
        constexpr decltype(auto) operator()(Container& container, Args&&... args) const noexcept( //
            ::stdsharp::concepts::nothrow_invocable<
                ::stdsharp::containers::actions::emplace_fn,
                Container&,
                decltype(*ConstIter),
                Args... // clang-format off
            > && // clang-format on
            noexcept(*::std::declval<decltype(*ConstIter)>()) //
        )
        {
            return *::stdsharp::containers::actions::emplace(
                container,
                ::std::as_const(container).begin(),
                ::std::forward<Args>(args)... //
            );
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
    private:
        template<typename Container, typename DecayContainer = ::std::decay_t<Container>>
            requires(
                ::stdsharp::containers::sequence_container<DecayContainer> ||
                ::stdsharp::containers::associative_container<DecayContainer> ||
                ::stdsharp::containers::unordered_associative_container<DecayContainer>)
        static constexpr auto iter_erase(
            Container& container,
            const typename DecayContainer::const_iterator iter // clang-format off
        ) noexcept(noexcept(container.erase(iter))) // clang-format on
        {
            return container.erase(iter);
        }

        template<typename Container, typename DecayContainer = ::std::decay_t<Container>>
            requires(
                ::stdsharp::containers::sequence_container<DecayContainer> ||
                ::stdsharp::containers::associative_container<DecayContainer> ||
                ::stdsharp::containers::unordered_associative_container<DecayContainer>)
        static constexpr auto iter_erase(
            Container& container,
            const typename DecayContainer::const_iterator iter_fst,
            decltype(iter_fst) iter_snd // clang-format off
        ) noexcept(noexcept(container.erase(iter_fst, iter_snd))) // clang-format on
        {
            return container.erase(iter_fst, iter_snd);
        }

    public:
        template<
            typename Container,
            ::std::equality_comparable_with<typename ::std::decay_t<Container>::value_type>
                ValueType // clang-format off
        > // clang-format on
        constexpr auto operator()(Container& container, const ValueType& value_type) const
            noexcept(noexcept(::std::erase(container, value_type)))
        {
            return ::std::erase(container, value_type);
        }

        template<typename Container, typename ConstIter>
            requires requires(Container container, const ConstIter const_iter)
            {
                ::stdsharp::containers::actions::erase_fn::iter_erase(container, const_iter);
            } // clang-format on
        constexpr auto operator()(Container& container, const ConstIter const_iter) const
            noexcept( //
                noexcept(::stdsharp::containers::actions:: //
                         erase_fn::iter_erase(container, const_iter) // clang-format off
                ) // clang-format on
            )
        {
            return ::stdsharp::containers::actions::erase_fn::iter_erase(container, const_iter);
        }

        template<typename Container, typename ConstIter>
            requires requires(Container& container, const ConstIter iter)
            {
                ::stdsharp::containers::actions::erase_fn::iter_erase(container, iter, iter);
            } // clang-format on
        constexpr auto operator()(
            Container& container,
            const ConstIter const_iter_begin,
            const ConstIter const_iter_end //
        ) const noexcept( //
            noexcept(::stdsharp::containers::actions:: //
                     erase_fn::iter_erase(container, const_iter_begin, const_iter_end)
                     // clang-format off
        ) // clang-format on
        )
        {
            return ::stdsharp::containers::actions:: //
                erase_fn::iter_erase(container, const_iter_begin, const_iter_end);
        }
    };

    inline constexpr auto erase =
        ::stdsharp::functional::tagged_cpo<::stdsharp::containers::actions::erase_fn>;

    struct erase_if_fn
    {
        template<
            typename Container,
            typename Predicate,
            typename ::std::decay_t<Container>::value_type* ValueType_ = nullptr // clang-format off
        > // clang-format on
            requires ::std::predicate<Predicate, decltype(*ValueType_)>
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
                    typename ::std::decay_t<Container>::const_iterator // clang-format off
                > // clang-format on
            )
        constexpr void operator()(Container& container) const noexcept( //
            ::stdsharp::concepts::nothrow_invocable<
                ::stdsharp::containers::actions::erase_fn,
                Container&,
                typename ::std::decay_t<Container>::const_iterator // clang-format off
            > // clang-format on
        )
        {
            ::stdsharp::containers::actions::erase(container, ::std::as_const(container).begin());
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
                    typename ::std::decay_t<Container>::const_iterator // clang-format off
                > // clang-format on
            )
        constexpr void operator()(Container& container) const noexcept( //
            ::stdsharp::concepts::nothrow_invocable<
                ::stdsharp::containers::actions::erase_fn,
                Container&,
                typename ::std::decay_t<Container>::const_iterator // clang-format off
            > // clang-format on
        )
        {
            ::stdsharp::containers::actions::erase(container, ::std::as_const(container).end());
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
        template<
            ::stdsharp::containers::sequence_container Container,
            const typename ::std::decay_t<Container>::size_type* SizeT_ =
                nullptr // clang-format off
        > // clang-format on
            requires requires(Container container, decltype(*SizeT_) size)
            { // clang-format off
                { container.resize(size) } -> ::std::same_as<void>; // clang-format on
            }
        constexpr void operator()(Container& container, decltype(*SizeT_) size) const
            noexcept(noexcept(container.resize(size)))
        {
            container.resize(size);
        }
    };

    inline constexpr auto resize =
        ::stdsharp::functional::tagged_cpo<::stdsharp::containers::actions::resize_fn>;
}
