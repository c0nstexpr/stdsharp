//
// Created by BlurringShadow on 2021-10-4.
//

#pragma once

#include <deque>
#include <forward_list>
#include <list>
#include <set>
#include <map>
#include <type_traits>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <queue>

#include "../memory/memory.h"
#include "../ranges/ranges.h"
#include "../functional/operations.h"
#include "../utility/pack_get.h"

namespace stdsharp
{
    template<typename Container>
        requires requires { typename Container::allocator_type; }
    struct allocator_of<Container> : ::std::type_identity<typename Container::allocator_type>
    {
    };

    template<typename T, auto Size>
    struct allocator_of<::std::array<T, Size>>
    {
        using type = ::std::allocator<T>;
    };
}

namespace stdsharp::containers
{
    template<typename ValueType, typename Allocator>
    concept erasable = requires(Allocator allocator_instance, ValueType* ptr) //
    {
        requires allocator_req<Allocator>;
        requires ::std::same_as<
            Allocator,
            typename ::std::allocator_traits<Allocator>:: //
            template rebind_alloc<ValueType> // clang-format off
        >; // clang-format on
        requires ::std::destructible<ValueType>;
        ::std::allocator_traits<Allocator>::destroy(allocator_instance, ptr);
    };

    template<typename ValueType, typename Allocator>
    concept nothrow_erasable = requires(Allocator allocator_instance, ValueType* ptr) //
    {
        requires erasable<ValueType, Allocator>;
        requires noexcept(::std::allocator_traits<Allocator>::destroy(allocator_instance, ptr));
    };

    template<typename Container>
    concept container_erasable = requires //
    {
        requires erasable<
            typename ::std::decay_t<Container>::value_type,
            allocator_of_t<::std::decay_t<Container>> // clang-format off
        >; // clang-format on
    };

    template<typename Container>
    concept container_nothrow_erasable = requires //
    {
        requires nothrow_erasable<
            typename ::std::decay_t<Container>::value_type,
            allocator_of_t<::std::decay_t<Container>> // clang-format off
        >; // clang-format on
    };

    template<typename ValueType, typename Allocator>
    concept move_insertable =
        requires(Allocator allocator_instance, ValueType* ptr, ValueType&& rv) //
    {
        requires allocator_req<Allocator>;
        requires ::std::same_as<
            Allocator,
            typename ::std::allocator_traits<Allocator>:: //
            template rebind_alloc<ValueType> // clang-format off
        >; // clang-format on
        requires ::std::move_constructible<ValueType>;
        ::std::allocator_traits<Allocator>::construct(allocator_instance, ptr, ::std::move(rv));
    };

    template<typename ValueType, typename Allocator>
    concept nothrow_move_insertable =
        requires(Allocator allocator_instance, ValueType* ptr, ValueType&& rv) //
    {
        requires move_insertable<ValueType, Allocator>;
        requires concepts::nothrow_move_constructible<ValueType>;
        requires noexcept(
            ::std::allocator_traits<Allocator>::construct(allocator_instance, ptr, ::std::move(rv))
        );
    };

    template<typename Container>
    concept container_move_insertable = requires //
    {
        requires move_insertable<
            typename ::std::decay_t<Container>::value_type, // clang-format off
                allocator_of_t<::std::decay_t<Container>>
        >; // clang-format on
    };

    template<typename Container>
    concept container_nothrow_move_insertable = requires //
    {
        requires nothrow_move_insertable<
            typename ::std::decay_t<Container>::value_type,
            allocator_of_t<::std::decay_t<Container>> // clang-format off
        >; // clang-format on
    };

    template<typename ValueType, typename Allocator>
    concept copy_insertable = requires(Allocator allocator_instance, ValueType* ptr, ValueType v) //
    {
        requires move_insertable<ValueType, Allocator> && ::std::copy_constructible<ValueType>;
        ::std::allocator_traits<Allocator>::construct(allocator_instance, ptr, v);
    };

    template<typename ValueType, typename Allocator>
    concept nothrow_copy_insertable =
        requires(Allocator allocator_instance, ValueType* ptr, ValueType v) //
    {
        requires nothrow_move_insertable<ValueType, Allocator>;
        requires concepts::nothrow_copy_constructible<ValueType>;
        requires noexcept(
            ::std::allocator_traits<Allocator>::construct(allocator_instance, ptr, v) //
        );
    };

    template<typename Container>
    concept container_copy_insertable = requires //
    {
        requires copy_insertable<
            typename ::std::decay_t<Container>::value_type,
            allocator_of_t<::std::decay_t<Container>> // clang-format off
        >; // clang-format on
    };

    template<typename Container>
    concept container_nothrow_copy_insertable = requires //
    {
        requires nothrow_copy_insertable<
            typename ::std::decay_t<Container>::value_type,
            allocator_of_t<::std::decay_t<Container>> // clang-format off
        >; // clang-format on
    };

    template<typename ValueType, typename Allocator, typename... Args>
    concept emplace_constructible =
        requires(Allocator allocator_instance, ValueType* ptr, Args&&... args) //
    {
        requires ::std::same_as<
            Allocator,
            typename ::std::allocator_traits<Allocator>:: //
            template rebind_alloc<ValueType> // clang-format off
        >; // clang-format on
        requires ::std::constructible_from<ValueType, Args...>;
        ::std::allocator_traits<Allocator>::construct(
            allocator_instance,
            ptr,
            ::std::forward<Args>(args)...
        );
    };

    template<typename ValueType, typename Allocator, typename... Args>
    concept nothrow_emplace_constructible =
        requires(Allocator allocator_instance, ValueType* ptr, Args&&... args) //
    {
        requires concepts::nothrow_constructible_from<ValueType, Args...>;
        requires noexcept(::std::allocator_traits<Allocator>::construct(
            allocator_instance,
            ptr,
            ::std::forward<Args>(args)...
        ));
    };

    template<typename Container, typename... Args>
    concept container_emplace_constructible = requires //
    {
        requires emplace_constructible<
            typename ::std::decay_t<Container>::value_type,
            allocator_of_t<::std::decay_t<Container>>,
            Args... // clang-format off
        >; // clang-format on
    };

    template<typename Container, typename... Args>
    concept container_nothrow_emplace_constructible = requires //
    {
        requires nothrow_emplace_constructible<
            typename ::std::decay_t<Container>::value_type,
            allocator_of_t<::std::decay_t<Container>>,
            Args... // clang-format off
        >; // clang-format on
    };

    namespace details
    {
        template<typename T, typename U>
        concept iterator_identical = requires(
            ::std::iterator_traits<T> t_traits,
            ::std::iterator_traits<U> u_traits
        ) //
        {
            requires ::std::same_as<
                typename decltype(t_traits)::value_type,
                typename decltype(u_traits)::value_type // clang-format off
            >; // clang-format on
            requires ::std::same_as<
                typename decltype(t_traits)::difference_type,
                typename decltype(u_traits)::difference_type // clang-format off
            >; // clang-format on
            requires ::std::
                same_as<typename decltype(t_traits)::pointer, typename decltype(u_traits)::pointer>;
            requires ::std::same_as<
                typename decltype(t_traits)::reference,
                typename decltype(u_traits)::reference // clang-format off
            >; // clang-format on
        };

        template<typename>
        struct insert_return_type_of;

        template<template<typename, typename> typename InsertReturnT, typename T, typename U>
        struct insert_return_type_of<InsertReturnT<T, U>>
        {
            template<typename V, typename W>
            using type = InsertReturnT<V, W>;
        };

        template<typename ContainerType>
        concept container_insertable = requires(
            ContainerType instance,
            typename ContainerType::value_type value,
            const decltype(value)& const_value,
            typename ContainerType::iterator iter,
            typename ContainerType::const_iterator const_iter
        ) //
        {
            requires functional::logical_imply(
                container_move_insertable<ContainerType>,
                requires // clang-format off
                {
                    { instance.insert(const_iter, ::std::move(value)) } ->
                        ::std::same_as<decltype(iter)>; // clang-format on

                    requires functional::logical_imply(
                        container_copy_insertable<decltype(instance)>,
                        requires // clang-format off
                        {
                            { instance.insert(const_iter, const_value) } ->
                                ::std::same_as<decltype(iter)>; // clang-format on
                        }
                    );
                }
            );
        };
    }

    template<typename Iterator, typename Node>
    using insert_return_type =
        details::insert_return_type_of<::std::set<int>::insert_return_type>::type<Iterator, Node>;

    template<typename Container, typename... Elements>
    concept container = requires //
    {
        typename ::std::decay_t<Container>;
        requires requires(
            ::std::decay_t<Container> instance,
            const decltype(instance) const_instance,
            typename decltype(instance)::value_type value,
            ::std::ranges::range_reference_t<decltype(instance)> rng_ref,
            typename decltype(instance)::reference ref,
            typename decltype(instance)::const_reference const_ref,
            typename decltype(instance)::iterator iter,
            typename decltype(instance)::const_iterator const_iter,
            typename decltype(instance)::difference_type diff,
            typename decltype(instance)::size_type size // clang-format off
        ) // clang-format on
        {
            requires ::std::ranges::forward_range<decltype(instance)>;

            requires ::std::destructible<decltype(instance)>;

            requires functional::logical_imply(
                         (concepts::nothrow_default_initializable<Elements> && ...),
                         concepts::nothrow_default_initializable<decltype(instance)> //
                     ) ||
                functional::logical_imply(
                         (::std::default_initializable<Elements> && ...),
                         ::std::default_initializable<decltype(instance)> //
                );

            requires functional::logical_imply(
                         (concepts::nothrow_copy_constructible<Elements> && ...) &&
                             container_nothrow_copy_insertable<decltype(instance)>,
                         concepts::nothrow_copy_constructible<decltype(instance)> //
                     ) ||
                functional::logical_imply(
                         (::std::copy_constructible<Elements> && ...) &&
                             container_copy_insertable<decltype(instance)>,
                         ::std::copy_constructible<decltype(instance)> //
                );
            requires functional::logical_imply(
                         (concepts::nothrow_copy_assignable<Elements> && ...) &&
                             concepts::nothrow_copy_assignable<decltype(value)> &&
                             container_nothrow_copy_insertable<decltype(instance)>,
                         concepts::nothrow_copy_assignable<decltype(instance)> //
                     ) ||
                functional::logical_imply(
                         (concepts::copy_assignable<Elements> && ...) &&
                             concepts::copy_assignable<decltype(value)> &&
                             container_copy_insertable<decltype(instance)>,
                         concepts::copy_assignable<decltype(instance)> //
                );

            requires functional::logical_imply(
                         (::std::move_constructible<Elements> && ...),
                         ::std::move_constructible<decltype(instance)> //
                     ) ||
                functional::logical_imply(
                         (concepts::nothrow_move_constructible<Elements> && ...),
                         concepts::nothrow_move_constructible<decltype(instance)> //
                );

            requires functional::logical_imply(
                ::std::equality_comparable<decltype(value)>,
                ::std::equality_comparable<decltype(instance)> //
            );

            requires requires(allocator_of_t<decltype(instance)> alloc) //
            {
                requires allocator_req<decltype(alloc)>;

                requires container_erasable<decltype(instance)>;

                requires functional::logical_imply(
                             ::std::allocator_traits<decltype(alloc
                                 )>::propagate_on_container_move_assignment::value ||
                                 container_nothrow_move_insertable<decltype(instance)> &&
                                     (concepts::nothrow_move_assignable<Elements> && ...),
                             concepts::nothrow_move_assignable<decltype(instance)> //
                         ) ||
                    functional::logical_imply(
                             ::std::allocator_traits<decltype(alloc
                                 )>::propagate_on_container_move_assignment::value ||
                                 container_move_insertable<decltype(instance)> &&
                                     (concepts::move_assignable<Elements> && ...),
                             concepts::move_assignable<decltype(instance)> //
                    );
            };

            requires ::std::
                same_as<decltype(value), ::std::ranges::range_value_t<decltype(instance)>>;
            requires ::std::same_as<decltype(ref), ::std::add_lvalue_reference_t<decltype(value)>>;
            requires ::std::
                same_as<decltype(const_ref), type_traits::add_const_lvalue_ref_t<decltype(value)>>;
            requires ::std::same_as<decltype(iter), ::std::ranges::iterator_t<decltype(instance)>>;
            requires ::std::same_as<decltype(iter), ::std::ranges::sentinel_t<decltype(instance)>>;
            requires ::std::
                same_as<decltype(const_iter), ranges::const_iterator_t<decltype(instance)>>;
            requires ::std::
                same_as<decltype(const_iter), ranges::const_sentinel_t<decltype(instance)>>;
            requires ::std::same_as<decltype(ref), decltype(rng_ref)> ||
                ::std::same_as<decltype(const_ref), decltype(rng_ref)>;
            requires ::std::same_as<
                decltype(const_ref),
                ranges::range_const_reference_t<decltype(instance)> // clang-format off
            >; // clang-format on
            requires ::std::signed_integral<decltype(diff)>;
            requires ::std::
                same_as<decltype(diff), ::std::ranges::range_difference_t<decltype(instance)>>;
            requires ::std::unsigned_integral<decltype(size)>;
            requires requires //
            {
                []<typename T, typename U>(::std::forward_list<T, U>*) {}(&instance); //
            } ||
                requires //
            {
                requires ::std::
                    same_as<decltype(size), ::std::ranges::range_size_t<decltype(instance)>>;
            };
            requires(
                ::std::numeric_limits<decltype(size)>::max() >
                ::std::numeric_limits<decltype(diff)>::max()
            ); // clang-format off

            { instance.max_size() } -> ::std::same_as<decltype(size)>;
            { instance.empty() } -> ::std::convertible_to<bool>; // clang-format on
        };
    };

    template<typename Container, typename... Elements>
    concept reversible_container = container<Container, Elements...> &&
        requires //
    {
        typename ::std::decay_t<Container>;
        requires requires(
            ::std::decay_t<Container> instance,
            const decltype(instance)& const_instance,
            typename decltype(instance)::reverse_iterator iter,
            typename decltype(instance)::const_reverse_iterator const_iter
        ) // clang-format off
        {
            { instance.rbegin() } -> ::std::same_as<decltype(iter)>;
            { instance.rend() } -> ::std::same_as<decltype(iter)>;
            { const_instance.rbegin() } -> ::std::same_as<decltype(const_iter)>;
            { const_instance.rend() } -> ::std::same_as<decltype(const_iter)>;
            { instance.crbegin() } -> ::std::same_as<decltype(const_iter)>;
            { instance.crend() } -> ::std::same_as<decltype(const_iter)>; // clang-format on
        };
    };

    template<typename Container, typename... Elements>
    concept allocator_aware_container = requires //
    {
        requires requires(
            ::std::decay_t<Container> instance,
            typename decltype(instance)::value_type value,
            typename decltype(instance)::allocator_type alloc
        ) //
        {
            requires allocator_req<decltype(alloc)>;
            requires container<Container, decltype(alloc), Elements...>;
            requires ::std::constructible_from<decltype(instance), decltype(alloc)>;

            requires functional::logical_imply(
                container_copy_insertable<decltype(instance)>,
                ::std::constructible_from<
                    decltype(instance),
                    const decltype(instance)&,
                    decltype(alloc) // clang-format off
                > // clang-format on
            );
            requires functional::logical_imply(
                container_move_insertable<decltype(instance)>,
                ::std::constructible_from<
                    decltype(instance),
                    decltype(instance),
                    decltype(alloc) // clang-format off
                > // clang-format on
            );
            requires ::std::same_as<decltype(value), typename decltype(alloc)::value_type>;

            // clang-format off
            { instance.get_allocator() } -> ::std::same_as<decltype(alloc)>; // clang-format on
        };
    };

    template<typename Container, typename... Elements>
    concept sequence_container = container<Container, Elements...> &&
        requires //
    {
        requires requires(
            ::std::decay_t<Container> instance,
            const decltype(instance)& const_instance,
            typename decltype(instance)::value_type value,
            const decltype(value)& const_value,
            typename decltype(instance)::iterator iter,
            typename decltype(instance)::const_iterator const_iter,
            typename decltype(instance)::size_type size
        ) //
        {
            requires functional::logical_imply(
                container_copy_insertable<decltype(instance)>,
                ::std::constructible_from<
                    decltype(instance),
                    decltype(size),
                    decltype(value) // clang-format off
                > // clang-format on
            );

            requires functional::logical_imply(
                container_emplace_constructible<decltype(instance), decltype(value)>,
                requires(::std::initializer_list<decltype(value)> v_list) //
                {
                    requires ::std::constructible_from<
                        decltype(instance),
                        decltype(const_iter),
                        decltype(const_iter) // clang-format off
                    >;
                    requires ::std::constructible_from<decltype(instance), decltype(v_list)>;
                    { instance.insert(const_iter, v_list) } -> ::std::same_as<decltype(iter)>;
                    { instance.assign(v_list) } -> ::std::same_as<void>;

                    { instance.emplace(const_iter, const_value) } ->
                        ::std::same_as<decltype(iter)>;

                    { instance.insert(const_iter, const_iter, const_iter) } ->
                        ::std::same_as<decltype(iter)>;
                    { instance.assign(const_iter, const_iter) } -> ::std::same_as<void>;

                    { instance.erase(const_iter) } -> ::std::same_as<decltype(iter)>;
                    { instance.erase(const_iter, const_iter) } -> ::std::same_as<decltype(iter)>;

                    { instance.clear() } -> ::std::same_as<void>; // clang-format on
                }
            );

            requires details::container_insertable<decltype(instance)>;

            requires functional::logical_imply(
                container_copy_insertable<decltype(instance)> &&
                    concepts::copy_assignable<decltype(value)>, // clang-format off
                requires(decltype(size) n)
                {
                    requires ::std::assignable_from<
                        decltype(instance)&,
                        ::std::initializer_list<decltype(value)>
                    >;

                    { instance.insert(const_iter, n, const_value) } -> ::std::same_as<decltype(iter)>;

                    { instance.assign(n, const_value) } -> ::std::same_as<void>;
                }
            );

            { instance.front() } -> ::std::same_as<typename decltype(instance)::reference>;
            { const_instance.front() } ->
                ::std::same_as<typename decltype(instance)::const_reference>; // clang-format on
        };
    };

    template<typename Container>
    concept contiguous_container =
        container<Container> && ::std::ranges::contiguous_range<::std::decay_t<Container>>;

    namespace details
    {
        template<typename Container>
        concept unique_associative = requires(
            ::std::decay_t<Container> instance,
            typename decltype(instance)::node_type node,
            typename decltype(instance)::insert_return_type insert_return_v
        ) // clang-format off
        {
            { instance.insert(::std::move(node)) } ->
                ::std::same_as<decltype(insert_return_v)>;
            requires ::std::same_as<
                decltype(insert_return_v),
                insert_return_type<decltype(instance.begin()), decltype(node)>
            >;
        }; // clang-format on

        template<typename Container>
        concept multikey_associative = requires(
            ::std::decay_t<Container> instance,
            typename decltype(instance)::node_type node
        ) // clang-format off
        {
            { instance.insert(::std::move(node)) } ->
                ::std::same_as<typename decltype(instance)::iterator>; // clang-format on
        };

        template<typename Container, typename... Args>
        struct optional_constructible
        {
            template<typename... Optional>
            static constexpr auto value = []<::std::size_t... I>(const ::std::index_sequence<I...>)
            {
                constexpr ::std::size_t count = sizeof...(Optional);

                if constexpr(count == 0) return ::std::constructible_from<Container, Args...>;
                else
                {
                    using last_t = pack_get_t<count - 1, Optional...>;

                    constexpr auto res = ::std::constructible_from<
                        Container,
                        Args...,
                        pack_get_t<I, Optional...>...,
                        last_t // clang-format off
                    >; // clang-format on

                    if constexpr(::std::default_initializable<last_t>)
                        return res && value<pack_get_t<I, Optional...>...>();
                    else return res;
                }
            }
            ( //
                ::std::make_index_sequence<
                    sizeof...(Optional) == 0 ? sizeof...(Optional) :
                                               sizeof...(Optional) - 1 // clang-format off
                >{} // clang-format on
            );
        };
    }

    template<typename Container, typename... Elements>
    concept associative_like_container = allocator_aware_container<Container, Elements...> &&
        requires //
    {
        requires requires(
            ::std::decay_t<Container> instance,
            const decltype(instance)& const_instance,
            typename decltype(instance)::key_type key,
            typename decltype(instance)::const_reference const_ref,
            typename decltype(instance)::value_type value,
            typename decltype(instance)::node_type node,
            typename decltype(instance)::iterator iter,
            typename decltype(instance)::const_iterator const_iter,
            typename decltype(instance)::size_type size,
            ::std::initializer_list<decltype(value)> v_list
        ) //
        {
            requires details::container_insertable<decltype(instance)>;

            requires functional::logical_imply(
                container_emplace_constructible<decltype(instance), decltype(value)>,
                requires // clang-format off
                {
                    { instance.emplace_hint(const_iter, const_ref) } ->
                        ::std::same_as<decltype(iter)>;
                    { instance.insert(const_iter, const_iter) } ->
                        ::std::same_as<void>;
                    { instance.insert(v_list) } -> ::std::same_as<void>;
                }
            );

            { instance.extract(const_iter) } -> ::std::same_as<decltype(node)>;
            { instance.extract(key) } -> ::std::same_as<decltype(node)>;

            { instance.erase(key) } -> ::std::same_as<decltype(size)>;

            { instance.find(key) } -> ::std::same_as<decltype(iter)>;
            { const_instance.find(key) } -> ::std::same_as<decltype(const_iter)>;

            { const_instance.count(key) } -> ::std::same_as<decltype(size)>;

            { const_instance.contains(key) } -> ::std::same_as<bool>;

            { instance.equal_range(key) } ->
                ::std::same_as<::std::pair<decltype(iter), decltype(iter)>>;
            { const_instance.equal_range(key) } ->
                ::std::same_as<::std::pair<decltype(const_iter), decltype(const_iter)>>;

            { instance.insert(const_iter, ::std::move(node)) } -> ::std::same_as<decltype(iter)>;

            { instance.erase(const_iter) } -> ::std::same_as<decltype(iter)>;
            { instance.erase(const_iter, const_iter) } -> ::std::same_as<decltype(iter)>;
            { instance.erase(iter) } -> ::std::same_as<decltype(iter)>;

            { instance.merge(instance) } -> ::std::same_as<void>;

            { instance.clear() } -> ::std::same_as<void>; // clang-format on
        };
    };

    template<typename Container, typename... Elements>
    concept associative_container = requires //
    {
        requires requires(
            ::std::decay_t<Container> instance,
            const decltype(instance)& const_instance,
            typename decltype(instance)::value_type value,
            typename decltype(instance)::key_type key,
            allocator_of_t<decltype(instance)> alloc,
            typename decltype(instance)::key_compare key_cmp,
            typename decltype(instance)::value_compare value_cmp,
            typename decltype(instance)::iterator iter,
            typename decltype(instance)::const_iterator const_iter,
            typename decltype(instance)::size_type size,
            ::std::initializer_list<decltype(value)> v_list
        ) //
        {
            requires associative_like_container<decltype(instance), decltype(key_cmp)>;

            requires ::std::copyable<decltype(key_cmp)>;
            requires ::std::predicate<decltype(key_cmp), decltype(key), decltype(key)>;
            requires ::std::constructible_from<decltype(instance), const decltype(key_cmp)>;

            requires ::std::copyable<decltype(value_cmp)>;
            requires ::std::predicate<decltype(value_cmp), decltype(value), decltype(value)>;

            requires functional::logical_imply(
                container_emplace_constructible<decltype(instance), decltype(value)>,
                details::optional_constructible<
                    decltype(instance),
                    decltype(const_iter),
                    decltype(const_iter) // clang-format off
                >::template value<decltype(key_cmp), decltype(alloc)> &&
                details::optional_constructible<
                    decltype(instance),
                    decltype(v_list)
                >::template value<decltype(key_cmp), decltype(alloc)>
            );

#ifdef __cpp_lib_ranges_to_container
            requires details::optional_constructible<
                decltype(instance),
                ::std::from_range_t,
                decltype(instance) // clang-format off
            >::template value<decltype(key_cmp), decltype(alloc)>;
#endif

            { instance.key_comp() } -> ::std::same_as<decltype(key_cmp)>;
            { instance.value_comp() } -> ::std::same_as<decltype(value_cmp)>;

            { instance.lower_bound(key) } -> ::std::same_as<decltype(iter)>;
            { const_instance.lower_bound(key) } -> ::std::same_as<decltype(const_iter)>;
            { instance.upper_bound(key) } -> ::std::same_as<decltype(iter)>;
            { const_instance.upper_bound(key) } -> ::std::same_as<decltype(const_iter)>; // clang-format on
        };
    };

    template<typename Container, typename... Elements>
    concept unique_associative_container =
        associative_container<Container, Elements...> && details::unique_associative<Container>;

    template<typename Container, typename... Elements>
    concept multikey_associative_container =
        associative_container<Container, Elements...> && details::multikey_associative<Container>;

    template<typename Container, typename... Elements>
    concept unordered_associative_container = requires //
    {
        requires requires(
            ::std::decay_t<Container> instance,
            const decltype(instance)& const_instance,
            typename decltype(instance)::value_type value,
            typename decltype(instance)::key_type key,
            typename decltype(instance)::key_equal key_equal,
            typename decltype(instance)::hasher hasher,
            allocator_of_t<decltype(instance)> alloc,
            typename decltype(instance)::reference ref,
            typename decltype(instance)::const_reference const_ref,
            typename decltype(instance)::iterator iter,
            typename decltype(instance)::const_iterator const_iter,
            typename decltype(instance)::local_iterator local_iter,
            typename decltype(instance)::const_local_iterator const_local_iter,
            typename decltype(instance)::difference_type diff,
            typename decltype(instance)::size_type size,
            ::std::initializer_list<decltype(value)> v_list
        ) //
        {
            requires associative_like_container<decltype(instance), decltype(hasher), Elements...>;

            requires ::std::copyable<decltype(key_equal)>;
            requires ::std::predicate<decltype(key_equal), decltype(key), decltype(key)>;

            requires ::std::copyable<decltype(hasher)>;
            requires concepts::invocable_r<decltype(hasher), ::std::size_t, decltype(key)>;

            requires details::iterator_identical<decltype(iter), decltype(local_iter)>;
            requires details::iterator_identical<decltype(const_iter), decltype(const_local_iter)>;

            requires details::optional_constructible<decltype(instance), decltype(size)>::
                template value<decltype(hasher), decltype(key_equal), decltype(alloc)>;

            requires functional::logical_imply(
                container_emplace_constructible<decltype(instance), decltype(value)>,
                requires //
                {
                    requires details::optional_constructible<
                        decltype(instance),
                        decltype(const_iter),
                        decltype(const_iter) // clang-format off
                    >::template value<
                        decltype(size),
                        decltype(hasher),
                        decltype(key_equal),
                        decltype(alloc)
                    >; // clang-format on

                    requires details::optional_constructible<
                        decltype(instance),
                        decltype(v_list)>::template value< // clang-format off
                        decltype(size),
                        decltype(hasher),
                        decltype(key_equal),
                        decltype(alloc)
                    >; // clang-format on

#ifdef __cpp_lib_ranges_to_container
                    requires details::optional_constructible<
                        decltype(instance),
                        ::std::from_range_t,
                        decltype(instance) // clang-format off
                    >::template value<
                        decltype(size),
                        decltype(hasher),
                        decltype(key_equal),
                        decltype(alloc)
                    >; // clang-format on
#endif
                } //
            );

            requires functional::logical_imply(
                container_copy_insertable<decltype(instance)> &&
                    concepts::copy_assignable<decltype(value)> &&
                    ::std::default_initializable<decltype(hasher)> &&
                    ::std::default_initializable<decltype(key_equal)> &&
                    ::std::default_initializable<decltype(alloc)>,
                ::std::constructible_from<decltype(instance), decltype(v_list)> //
            ); // clang-format off

            { instance.key_eq() } -> ::std::same_as<decltype(key_equal)>;
            { instance.hash_function() } -> ::std::same_as<decltype(hasher)>;

            { const_instance.bucket(key) } -> ::std::same_as<decltype(size)>;
            { const_instance.bucket_count() } -> ::std::same_as<decltype(size)>;
            { const_instance.max_bucket_count() } -> ::std::same_as<decltype(size)>;

            { instance.max_load_factor(0.0f) } -> ::std::same_as<void>;

            { const_instance.load_factor()  } -> ::std::same_as<float>;
            { const_instance.max_load_factor() } -> ::std::same_as<float>;

            { const_instance.bucket_size(size) } -> ::std::same_as<decltype(size)>;

            { instance.rehash(size) } -> ::std::same_as<void>;

            { instance.reserve(size) } -> ::std::same_as<void>;

            { instance.begin(size) } -> ::std::same_as<decltype(local_iter)>;
            { instance.end(size) } -> ::std::same_as<decltype(local_iter)>;

            { const_instance.begin(size) } -> ::std::same_as<decltype(const_local_iter)>;
            { const_instance.end(size) } -> ::std::same_as<decltype(const_local_iter)>;
            { const_instance.cbegin(size) } -> ::std::same_as<decltype(const_local_iter)>;
            { const_instance.cend(size) } -> ::std::same_as<decltype(const_local_iter)>; // clang-format on
        };
    };

    template<typename Container, typename... Elements>
    concept unique_unordered_associative_container =
        unordered_associative_container<Container, Elements...> &&
        details::unique_associative<Container>;

    template<typename Container, typename... Elements>
    concept multikey_unordered_associative_container =
        unordered_associative_container<Container, Elements...> &&
        details::multikey_associative<Container>;

    template<typename Predicate, typename Container>
    concept container_predicatable =
        ::std::predicate<Predicate, ranges::range_const_reference_t<Container>>;
}