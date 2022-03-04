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

#include "memory/memory.h"
#include "ranges/ranges.h"
#include "concepts/concepts.h"

namespace stdsharp
{
    namespace containers::details
    {
        template<typename ContainerType>
        concept std_array = requires(::std::decay_t<ContainerType> c)
        {
            ::std::tuple_size<decltype(c)>{};
            requires ::std::same_as <
                ::std::array<typename decltype(c)::value_type, ::std::tuple_size_v<decltype(c)>>,
            decltype(c) // clang-format off
            >; // clang-format on
        };

        template<typename ContainerType>
        concept has_allocator_type_alias = requires
        {
            typename ContainerType::allocator_type;
            requires allocator_req<typename ContainerType::allocator_type>;
        };
    }

    template<typename ContainerType>
        requires(
            !(containers::details::std_array<ContainerType> ||
              containers::details::has_allocator_type_alias<ContainerType>) //
        )
    struct allocator_of<ContainerType> :
        ::std::type_identity<::std::allocator<typename ContainerType::value_type>>
    {
    };

    template<containers::details::has_allocator_type_alias ContainerType>
    struct allocator_of<ContainerType> :
        ::std::type_identity<typename ContainerType::allocator_type>
    {
    };

}

namespace stdsharp::containers
{
    template<typename ValueType, typename Allocator>
    concept erasable = allocator_req<Allocator> && // clang-format off
        ::std::same_as<
            Allocator,
            typename ::std::allocator_traits<Allocator>::template rebind_alloc<ValueType>
        > &&
        ::std::destructible<ValueType> &&
        requires(Allocator allocator_instance, ValueType* ptr)
        {
            ::std::allocator_traits<Allocator>::destroy(allocator_instance, ptr);
        }; // clang-format on

    template<typename Container>
    concept container_erasable = !details::std_array<Container> && requires
    {
        requires erasable<
            typename ::std::decay_t<Container>::value_type,
            allocator_of_t<::std::decay_t<Container>> // clang-format off
        >; // clang-format on
    };

    template<typename ValueType, typename Allocator>
    concept move_insertable = allocator_req<Allocator> && // clang-format off
        ::std::same_as<
            Allocator,
            typename ::std::allocator_traits<Allocator>::template rebind_alloc<ValueType>
        > &&
        ::std::move_constructible<ValueType> &&
        requires(Allocator allocator_instance, ValueType* ptr, ValueType&& rv)
        {
            ::std::allocator_traits<Allocator>::
                construct(allocator_instance, ptr, ::std::move(rv));
        }; // clang-format on

    template<typename Container>
    concept container_move_insertable = !details::std_array<Container> && requires
    {
        requires move_insertable<
            typename ::std::decay_t<Container>::value_type, // clang-format off
                allocator_of_t<::std::decay_t<Container>>
        >; // clang-format on
    };

    template<typename ValueType, typename Allocator>
    concept copy_insertable = move_insertable<ValueType, Allocator> && // clang-format off
        ::std::copy_constructible<ValueType> &&
        requires(Allocator allocator_instance, ValueType* ptr, ValueType v)
        {
            ::std::allocator_traits<Allocator>::construct(allocator_instance, ptr, v);
        }; // clang-format on

    template<typename Container>
    concept container_copy_insertable = requires
    {
        requires copy_insertable<
            typename ::std::decay_t<Container>::value_type,
            allocator_of_t<::std::decay_t<Container>> // clang-format off
        >; // clang-format on
    };

    template<typename ValueType, typename Allocator, typename... Args>
    concept emplace_constructible = ::std::same_as<
        Allocator, // clang-format off
        typename ::std::allocator_traits<Allocator>::template rebind_alloc<ValueType>
    > &&
        ::std::constructible_from<ValueType, Args...> &&
        requires(Allocator allocator_instance, ValueType* ptr, Args&&... args)
        {
            ::std::allocator_traits<Allocator>::
                construct(allocator_instance, ptr, ::std::forward<Args>(args)...);
        }; // clang-format on

    template<typename Container, typename... Args>
    concept container_emplace_constructible = !details::std_array<Container> && requires
    {
        requires emplace_constructible<
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
            ::std::iterator_traits<U> u_traits //
        )
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

        template<typename ContainerType, typename... OtherMemberType>
        concept container_special_member =
            ( //
                !(::std::default_initializable<OtherMemberType> && ...) ||
                ::std::default_initializable<ContainerType> // clang-format off
            ) && // clang-format on
            ::std::destructible<ContainerType> &&
            ( //
                container_copy_insertable<ContainerType> &&
                    (::std::copyable<OtherMemberType> && ...) && //
                    ::std::copyable<ContainerType> ||
                (::std::movable<OtherMemberType> && ...) && //
                    ::std::movable<ContainerType> &&
                    concepts::copy_assignable<ContainerType> // clang-format off
            ); // clang-format on

        template<typename>
        struct insert_return_type_of;

        template<template<typename, typename> typename InsertReturnT, typename T, typename U>
        struct insert_return_type_of<InsertReturnT<T, U>>
        {
            template<typename V, typename W>
            using type = InsertReturnT<V, W>;
        };
    }

    template<typename Iterator, typename Node>
    using insert_return_type =
        details::insert_return_type_of<::std::set<int>::insert_return_type>::type<Iterator, Node>;

    template<typename Container>
    concept container = details::std_array<Container> || requires
    {
        typename ::std::decay_t<Container>;
        requires requires(
            ::std::decay_t<Container> instance,
            const decltype(instance) const_instance,
            typename decltype(instance)::value_type value,
            allocator_of_t<decltype(instance)> alloc,
            ::std::ranges::range_reference_t<decltype(instance)> rng_ref,
            typename decltype(instance)::reference ref,
            typename decltype(instance)::const_reference const_ref,
            typename decltype(instance)::iterator iter,
            typename decltype(instance)::const_iterator const_iter,
            typename decltype(instance)::difference_type diff,
            typename decltype(instance)::size_type size)
        {
            requires container_erasable<decltype(instance)> &&
                ::std::ranges::forward_range<decltype(instance)>;
            requires(
                !::std::equality_comparable<decltype(value)> ||
                ::std::equality_comparable<decltype(instance)> //
            );
            requires ::std::
                same_as<decltype(value), ::std::ranges::range_value_t<decltype(instance)>>;
            requires ::std::same_as<decltype(ref), ::std::add_lvalue_reference_t<decltype(value)>>;
            requires ::std::
                same_as<decltype(const_ref), type_traits::add_const_lvalue_ref_t<decltype(value)>>;
            requires ::std::same_as<decltype(iter), ::std::ranges::iterator_t<decltype(instance)>>;
            requires ::std::
                same_as<decltype(const_iter), ranges::const_iterator_t<decltype(instance)>>;
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
            requires ::std::
                same_as<decltype(size), ::std::ranges::range_size_t<decltype(instance)>>;
            requires(
                ::std::numeric_limits<decltype(size)>::max() >
                ::std::numeric_limits<decltype(diff)>::max() //
            );

            requires requires
            {
                typename decltype(instance)::key_type;
                requires details::container_special_member<
                    decltype(instance),
                    decltype(alloc),
                    typename decltype(instance)::key_compare,
                    typename decltype(instance)::value_compare>;
            } || requires
            {
                typename decltype(instance)::key_type;
                requires details::container_special_member<
                    decltype(instance),
                    decltype(alloc),
                    typename decltype(instance)::key_equal,
                    typename decltype(instance)::hasher>;
            } || details::container_special_member<decltype(instance), decltype(alloc)>;

            // clang-format off
            { const_instance.cbegin() } -> ::std::same_as<decltype(const_iter)>;
            { const_instance.cend() } -> ::std::same_as<decltype(const_iter)>;
            { instance.begin() } -> ::std::same_as<decltype(iter)>;
            { instance.end() } -> ::std::same_as<decltype(iter)>;
            { instance.size() } -> ::std::same_as<decltype(size)>;
            { instance.max_size() } -> ::std::same_as<decltype(size)>;
            { instance.empty() } -> ::std::convertible_to<bool>; // clang-format on
        };
    };

    template<typename Handle>
    concept node_handle = concepts::nothrow_movable<Handle> && requires(Handle handle)
    { // clang-format off
            requires noexcept(static_cast<bool>(handle));
            { ::std::as_const(handle).get_allocator() } ->
                ::std::same_as<typename Handle::allocator_type>;
            requires noexcept(::std::as_const(handle).empty());
            { ::std::as_const(handle).empty() } -> ::std::same_as<bool>;
    }; // clang-format on

    namespace details
    {
        template<typename Container>
        concept unique_associative = requires(
            ::std::decay_t<Container> instance,
            typename decltype(instance)::node_type node,
            typename decltype(instance)::insert_return_type insert_return_v //
        )
        { // clang-format off
            { instance.insert(::std::move(node)) } ->
                ::std::same_as<decltype(insert_return_v)>; // clang-format on
            requires ::std::same_as<
                decltype(insert_return_v), // clang-format off
                insert_return_type<typename decltype(instance)::iterator, decltype(node)>
            >; // clang-format on
        };
    }

    template<typename Container>
    concept reversible_container = container<Container> && requires
    {
        typename ::std::decay_t<Container>;
        requires requires(
            ::std::decay_t<Container> instance,
            const decltype(instance)& const_instance,
            typename decltype(instance)::reverse_iterator iter,
            typename decltype(instance)::const_reverse_iterator const_iter //
        )
        { // clang-format off
                { instance.rbegin() } -> ::std::same_as<decltype(iter)>;
                { instance.rend() } -> ::std::same_as<decltype(iter)>;
                { const_instance.rbegin() } -> ::std::same_as<decltype(const_iter)>;
                { const_instance.rend() } -> ::std::same_as<decltype(const_iter)>;
                { instance.crbegin() } -> ::std::same_as<decltype(const_iter)>;
                { instance.crend() } -> ::std::same_as<decltype(const_iter)>; // clang-format on
        };
    };

    template<typename Container>
    concept allocator_aware_container = container<Container> && requires
    {
        requires requires(
            ::std::decay_t<Container> instance,
            typename decltype(instance)::value_type value,
            typename decltype(instance)::allocator_type alloc //
        )
        {
            requires ::std::constructible_from<decltype(instance), decltype(alloc)>;
            requires allocator_req<decltype(alloc)>;
            requires(
                container_copy_insertable<decltype(instance)> &&
                    ::std::constructible_from<
                        decltype(instance),
                        const decltype(instance)&,
                        decltype(alloc) // clang-format off
                     > || // clang-format on
                container_move_insertable<decltype(instance)> &&
                    ::std::constructible_from<
                        decltype(instance),
                        decltype(instance),
                        decltype(alloc) // clang-format off
                    > // clang-format on
            );

            ::std::allocator_traits<decltype(alloc)>:: //
                propagate_on_container_move_assignment::value ||
                container_move_insertable<decltype(instance)>&&
                    concepts::move_assignable<decltype(value)>;
            requires ::std::same_as<decltype(value), typename decltype(alloc)::value_type>;

            // clang-format off
            { instance.get_allocator() } -> ::std::same_as<decltype(alloc)>; // clang-format on
        };
    };

    template<typename Container>
    concept sequence_container = container<Container> && requires
    {
        typename ::std::decay_t<Container>;

        requires details::std_array<::std::decay_t<Container>> || requires(
            ::std::decay_t<Container> instance,
            const decltype(instance)& const_instance,
            typename decltype(instance)::value_type value,
            const decltype(value)& const_value,
            typename decltype(instance)::iterator iter,
            typename decltype(instance)::const_iterator const_iter,
            typename decltype(instance)::size_type size //
        )
        {
            requires !container_copy_insertable<decltype(instance)> ||
                ::std::constructible_from<decltype(instance), decltype(size), decltype(value)>;

            requires !container_emplace_constructible<decltype(instance), decltype(value)> ||
                ::std::constructible_from<
                    decltype(instance),
                    decltype(const_iter),
                    decltype(const_iter)> &&
                    ::std::constructible_from<
                        decltype(instance),
                        ::std::initializer_list<decltype(value)>> &&
                    requires(::std::initializer_list<decltype(value)> v_list)
            { // clang-format off
                { instance.insert(const_iter, v_list) } -> ::std::same_as<decltype(iter)>;
                { instance.assign(v_list) } -> ::std::same_as<void>;
            } && requires
            {
                { instance.emplace(const_iter, const_value) } ->
                    ::std::same_as<decltype(iter)>;

                { instance.insert(const_iter, const_iter, const_iter) } ->
                    ::std::same_as<decltype(iter)>;
                { instance.assign(const_iter, const_iter) } -> ::std::same_as<void>;

                { instance.erase(const_iter) } -> ::std::same_as<decltype(iter)>;
                { instance.erase(const_iter, const_iter) } -> ::std::same_as<decltype(iter)>;

                { instance.clear() } -> ::std::same_as<void>; // clang-format on
            };

            requires !container_move_insertable<decltype(instance)> || requires
            { // clang-format off
                { instance.insert(const_iter, ::std::move(value)) } ->
                    ::std::same_as<decltype(iter)>;

                requires !container_copy_insertable<decltype(instance)> || requires
                {
                    { instance.insert(const_iter, const_value) } ->
                        ::std::same_as<decltype(iter)>; // clang-format on

                    requires !concepts::copy_assignable<decltype(value)> ||
                        requires(decltype(size) n)
                    {
                        requires ::std::assignable_from<
                            decltype(instance)&,
                            ::std::initializer_list<decltype(value)> // clang-format off
                        >;

                        { instance.insert(const_iter, n, const_value) } ->
                             ::std::same_as<decltype(iter)>;

                        { instance.assign(n, const_value) } -> ::std::same_as<void>;
                    };
                };
            };

            { instance.front() } -> ::std::same_as<typename decltype(instance)::reference>;
            { const_instance.front() } ->
                ::std::same_as<typename decltype(instance)::const_reference>; // clang-format on
        };
    };

    template<typename Container>
    concept contiguous_container =
        container<Container> && ::std::ranges::contiguous_range<::std::decay_t<Container>>;

    template<typename Container>
    concept associative_container = container<Container> && requires
    {
        typename ::std::decay_t<Container>;
        requires requires(
            ::std::decay_t<Container> instance,
            const decltype(instance)& const_instance,
            typename decltype(instance)::value_type value,
            typename decltype(instance)::key_type key,
            typename decltype(instance)::key_compare key_cmp,
            typename decltype(instance)::value_compare value_cmp,
            typename decltype(instance)::node_type node,
            allocator_of_t<decltype(instance)> alloc,
            typename decltype(instance)::iterator iter,
            typename decltype(instance)::const_iterator const_iter,
            typename decltype(instance)::size_type size,
            ::std::initializer_list<decltype(value)> v_list //
        )
        {
            requires ::std::copyable<decltype(key_cmp)>;
            requires ::std::copyable<decltype(value_cmp)>;
            requires ::std::predicate<decltype(key_cmp), decltype(key), decltype(key)>;
            requires ::std::predicate<decltype(value_cmp), decltype(value), decltype(value)>;
            requires node_handle<decltype(node)>;

            requires !container_emplace_constructible<decltype(instance), decltype(value)> ||
                ( //
                    !::std::default_initializable<decltype(key_cmp)> ||
                    ::std::constructible_from<
                        decltype(instance),
                        decltype(const_iter),
                        decltype(const_iter) // clang-format off
                    > &&
                    ::std::constructible_from<decltype(instance), decltype(v_list)>
                ) && // clang-format on
                    ::std::constructible_from<
                        decltype(instance),
                        decltype(const_iter),
                        decltype(const_iter),
                        decltype(key_cmp) // clang-format off
                    > &&
                    ::std::constructible_from<decltype(instance), decltype(v_list), decltype(key_cmp)> &&
                    requires
                    {
                        { instance.insert(v_list) } -> ::std::same_as<void>;
                        { instance.emplace_hint(const_iter, ::std::as_const(value)) } ->
                            ::std::same_as<decltype(iter)>;

                        { instance.insert(const_iter, const_iter) } -> ::std::same_as<void>;
                    }; // clang-format on

            requires !container_move_insertable<decltype(instance)> || requires
            { // clang-format off
                { instance.insert(const_iter, ::std::move(value)) } -> ::std::same_as<decltype(iter)>;

                requires !container_copy_insertable<decltype(instance)> || requires
                {
                    { instance.insert(const_iter, ::std::as_const(value)) } ->
                        ::std::same_as<decltype(iter)>;
                };
            };

            { instance.key_comp() } -> ::std::same_as<decltype(key_cmp)>;
            { instance.value_comp() } -> ::std::same_as<decltype(value_cmp)>;

            { instance.extract(const_iter) } -> ::std::same_as<decltype(node)>;
            { instance.extract(key) } -> ::std::same_as<decltype(node)>;

            { instance.erase(key) } -> ::std::same_as<decltype(size)>;
            { instance.find(key) } -> ::std::same_as<decltype(iter)>;
            { const_instance.find(key) } -> ::std::same_as<decltype(const_iter)>;
            { const_instance.count(key) } -> ::std::same_as<decltype(size)>;
            { const_instance.contains(key) } -> ::std::same_as<bool>;
            { instance.lower_bound(key) } -> ::std::same_as<decltype(iter)>;
            { const_instance.lower_bound(key) } -> ::std::same_as<decltype(const_iter)>;
            { instance.upper_bound(key) } -> ::std::same_as<decltype(iter)>;
            { const_instance.upper_bound(key) } -> ::std::same_as<decltype(const_iter)>;
            { instance.equal_range(key) } -> ::std::same_as<::std::pair<decltype(iter), decltype(iter)>>;
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

    template<typename Container>
    concept unique_associative_container =
        associative_container<Container> && details::unique_associative<Container>;

    template<typename Container>
    concept multikey_associative_container = associative_container<Container> &&
        requires(::std::decay_t<Container> instance, typename decltype(instance)::node_type node)
    { // clang-format off
        { instance.insert(::std::move(node)) } ->::std::same_as<typename decltype(instance)::iterator>;
    }; // clang-format on

    template<typename Container>
    concept unordered_associative_container = container<Container> && requires
    {
        typename ::std::decay_t<Container>;
        requires requires(
            ::std::decay_t<Container> instance,
            const decltype(instance)& const_instance,
            typename decltype(instance)::value_type value,
            typename decltype(instance)::key_type key,
            typename decltype(instance)::key_equal key_equal,
            typename decltype(instance)::hasher hasher,
            typename decltype(instance)::node_type node,
            allocator_of_t<decltype(instance)> alloc,
            typename decltype(instance)::reference ref,
            typename decltype(instance)::const_reference const_ref,
            typename decltype(instance)::iterator iter,
            typename decltype(instance)::const_iterator const_iter,
            typename decltype(instance)::local_iterator local_iter,
            typename decltype(instance)::const_local_iterator const_local_iter,
            typename decltype(instance)::difference_type diff,
            typename decltype(instance)::size_type size,
            ::std::initializer_list<decltype(value)> v_list //
        )
        {
            requires details::iterator_identical<decltype(iter), decltype(local_iter)> &&
                details::iterator_identical<decltype(const_iter), decltype(const_local_iter)> &&
                ::std::copyable<decltype(key_equal)> && //
                ::std::copyable<decltype(hasher)> && //
                ::std::predicate<decltype(key_equal), decltype(key), decltype(key)> &&
                concepts::invocable_r<decltype(hasher), ::std::size_t, decltype(key)> &&
                node_handle<decltype(node)> &&( //
                    ::std::default_initializable<decltype(key_equal)> ?
                        (!::std::default_initializable<decltype(hasher)> ||
                         ::std::constructible_from<decltype(instance), decltype(size)>) :
                        ::std::constructible_from<
                            decltype(instance),
                            decltype(size),
                            decltype(hasher)> //
                    ) && //
                ::std::constructible_from<
                    decltype(instance),
                    decltype(size),
                    decltype(hasher),
                    decltype(key_equal) // clang-format off
                > &&
                ::std::constructible_from< // clang-format on
                    decltype(instance),
                    decltype(const_iter),
                    decltype(const_iter),
                    decltype(size),
                    decltype(hasher),
                    decltype(key_equal) // clang-format off
                >; // clang-format on

            requires !container_emplace_constructible<decltype(instance), decltype(value)> ||
                ( //
                    ::std::default_initializable<decltype(key_equal)> ?
                        (!::std::default_initializable<decltype(hasher)> ||
                         ::std::constructible_from<decltype(instance), decltype(size)> &&
                             ::std::constructible_from<
                                 decltype(instance),
                                 decltype(const_iter),
                                 decltype(const_iter),
                                 decltype(size) // clang-format off
                          > && // clang-format on
                             ::std::constructible_from<
                                 decltype(instance),
                                 decltype(v_list),
                                 decltype(size)>) :
                        ::std::constructible_from<
                            decltype(instance),
                            decltype(size),
                            decltype(hasher) // clang-format off
                        > && // clang-format on
                            ::std::constructible_from<
                                decltype(instance),
                                decltype(const_iter),
                                decltype(const_iter),
                                decltype(size),
                                decltype(hasher) // clang-format off
                            > && // clang-format on
                            ::std::constructible_from<
                                decltype(instance),
                                decltype(v_list),
                                decltype(size),
                                decltype(hasher) // clang-format off
                            >
                    ) &&
                    requires
                    {
                        { instance.emplace_hint(const_iter, const_ref) } ->
                            ::std::same_as<decltype(iter)>;
                        { instance.insert(const_iter, const_iter) } ->
                            ::std::same_as<void>;
                        { instance.insert(v_list) } -> ::std::same_as<void>;
                    };

            requires !container_move_insertable<decltype(instance)> || requires
            {
                { instance.insert(const_iter, ::std::move(value)) } ->
                    ::std::same_as<decltype(iter)>;

                requires container_copy_insertable<decltype(instance)> || requires
                {
                    { instance.insert(const_iter, ::std::as_const(value)) } ->
                        ::std::same_as<decltype(iter)>;
                };
            }; //

            { instance.key_eq() } -> ::std::same_as<decltype(key_equal)>;
            { instance.hash_function() } -> ::std::same_as<decltype(hasher)>;

            { instance.extract(const_iter) } -> ::std::same_as<decltype(node)>;
            { instance.extract(key) } -> ::std::same_as<decltype(node)>;
            { instance.erase(key) } -> ::std::same_as<decltype(size)>;
            { instance.find(key) } -> ::std::same_as<decltype(iter)>;
            { const_instance.find(key) } -> ::std::same_as<decltype(const_iter)>;
            { const_instance.count(key) } -> ::std::same_as<decltype(size)>;
            { const_instance.bucket(key) } -> ::std::same_as<decltype(size)>;
            { const_instance.contains(key) } -> ::std::same_as<bool>;
            { instance.equal_range(key) } -> ::std::same_as<::std::pair<decltype(iter), decltype(iter)>>;
            { const_instance.equal_range(key) } ->
                ::std::same_as<::std::pair<decltype(const_iter), decltype(const_iter)>>;

            { instance.insert(const_iter, ::std::move(node)) } -> ::std::same_as<decltype(iter)>;
            { instance.erase(const_iter) } -> ::std::same_as<decltype(iter)>;
            { instance.erase(const_iter, const_iter) } -> ::std::same_as<decltype(iter)>;

            { instance.erase(iter) } -> ::std::same_as<decltype(iter)>;
            { instance.merge(instance) } -> ::std::same_as<void>;
            { instance.clear() } -> ::std::same_as<void>;

            { const_instance.bucket_count() } -> ::std::same_as<decltype(size)>;
            { const_instance.max_bucket_count() } -> ::std::same_as<decltype(size)>;
            { const_instance.load_factor() } -> ::std::same_as<float>;
            { const_instance.max_load_factor() } -> ::std::same_as<float>;

            { const_instance.bucket_size(size) } -> ::std::same_as<decltype(size)>;
            { instance.begin(size) } -> ::std::same_as<decltype(local_iter)>;
            { instance.end(size) } -> ::std::same_as<decltype(local_iter)>;
            { instance.rehash(size) } -> ::std::same_as<void>;
            { instance.reserve(size) } -> ::std::same_as<void>;
            { const_instance.begin(size) } -> ::std::same_as<decltype(const_local_iter)>;
            { const_instance.end(size) } -> ::std::same_as<decltype(const_local_iter)>;
            { const_instance.cbegin(size) } -> ::std::same_as<decltype(const_local_iter)>;
            { const_instance.cend(size) } -> ::std::same_as<decltype(const_local_iter)>;

            { instance.max_load_factor(float{}) } -> ::std::same_as<void>; // clang-format on
        };
    };

    template<typename Container>
    concept unique_unordered_associative_container =
        unordered_associative_container<Container> && details::unique_associative<Container>;

    template<typename Container>
    concept multikey_unordered_associative_container = unordered_associative_container<Container> &&
        requires(::std::decay_t<Container> instance, typename decltype(instance)::node_type node)
    { // clang-format off
        { instance.insert(::std::move(node)) } ->::std::same_as<typename decltype(instance)::iterator>;
    }; // clang-format on

    template<typename Predicate, typename Container>
    concept container_predicatable =
        ::std::predicate<Predicate, ranges::range_const_reference_t<Container>>;
}
