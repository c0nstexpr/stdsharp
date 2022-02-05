//
// Created by BlurringShadow on 2021-10-4.
//

#pragma once
#include <deque>
#include <forward_list>
#include <list>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <queue>

#include <range/v3/iterator.hpp>

#include "memory/memory.h"
#include "ranges/ranges.h"
#include "concepts/concepts.h"

namespace stdsharp::containers
{
    namespace details
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
            requires allocator_req<typename ContainerType::allocator_type>;
        };
    }

    template<typename>
    struct allocator_from_container;

    template<typename ContainerType>
        requires(
            !(details::std_array<ContainerType> ||
              details::has_allocator_type_alias<ContainerType>) //
        )
    struct allocator_from_container<ContainerType> :
        ::std::type_identity<::std::allocator<typename ContainerType::value_type>>
    {
    };

    template<details::has_allocator_type_alias ContainerType>
    struct allocator_from_container<ContainerType> :
        ::std::type_identity<typename ContainerType::allocator_type>
    {
    };

    template<typename ContainerType>
    using allocator_from_container_t = typename allocator_from_container<ContainerType>::type;

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
            allocator_from_container_t<::std::decay_t<Container>> // clang-format off
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
                allocator_from_container_t<::std::decay_t<Container>>
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
            allocator_from_container_t<::std::decay_t<Container>> // clang-format off
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
            allocator_from_container_t<::std::decay_t<Container>>,
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

        template<typename ContainerType>
        concept container_req = container_erasable<ContainerType> &&
            ::std::ranges::forward_range<ContainerType> && // clang-format off
            requires(
                ContainerType instance,
                const ContainerType const_instance,
                typename ContainerType::value_type value,
                allocator_from_container_t<ContainerType> alloc,
                typename ContainerType::reference ref,
                typename ContainerType::const_reference const_ref,
                typename ContainerType::iterator iter,
                typename ContainerType::const_iterator const_iter,
                typename ContainerType::difference_type diff,
                typename ContainerType::size_type size
            )
            {
                requires (!::std::equality_comparable<decltype(value)> ||
                    ::std::equality_comparable<ContainerType>)&& //
                    ::std::same_as<decltype(value), ::std::ranges::range_value_t<ContainerType>> && //
                    (::std::same_as<decltype(ref), ::std::ranges::range_reference_t<ContainerType>> ||
                    ::std::same_as<decltype(const_ref), ::std::ranges::range_reference_t<ContainerType>>)&& //
                    ::std::same_as<decltype(ref), ::std::add_lvalue_reference_t<decltype(value)>>&& //
                    ::std::same_as<decltype(const_ref), type_traits::add_const_lvalue_ref_t<decltype(value)>>&& //
                    ::std::same_as<decltype(iter), ::std::ranges::iterator_t<ContainerType>>&& //
                    ::std::same_as<decltype(const_iter), ranges::const_iterator_t<ContainerType>>&& //
                    ::std::signed_integral<decltype(diff)>&& //
                    ::std::same_as<decltype(diff), ::std::ranges::range_difference_t<ContainerType>> &&
                    ::std::unsigned_integral<decltype(size)> &&
                    ::std::same_as<decltype(size), ::std::ranges::range_size_t<ContainerType>> &&
                    (::std::numeric_limits<decltype(size)>::max() >
                    ::std::numeric_limits<decltype(diff)>::max());

                requires requires
                {
                    typename ContainerType::key_type;
                    requires details::container_special_member<
                        ContainerType,
                        decltype(alloc),
                        typename ContainerType::key_compare,
                        typename ContainerType::value_compare
                    >;
                } ||
                requires
                {
                    typename ContainerType::key_type;
                    requires details::container_special_member<
                        ContainerType,
                        decltype(alloc),
                        typename ContainerType::key_equal,
                        typename ContainerType::hasher
                    >;
                } ||
                details::container_special_member<ContainerType, decltype(alloc)>;

                { const_instance.cbegin() } -> ::std::same_as<decltype(const_iter)>;
                { const_instance.cend() } -> ::std::same_as<decltype(const_iter)>;
                { instance.begin() } -> ::std::same_as<decltype(iter)>;
                { instance.end() } -> ::std::same_as<decltype(iter)>;
                { instance.size() } -> ::std::same_as<decltype(size)>;
                { instance.max_size() } -> ::std::same_as<decltype(size)>;
                { instance.empty() } -> ::std::convertible_to<bool>;
            }; // clang-format on
    }

    template<typename Container>
    concept container = details::std_array<Container> || requires
    {
        requires details::container_req<::std::decay_t<Container>>;
    };

    namespace details
    {
        template<typename ContainerType>
        concept allocator_aware_container_req = container<ContainerType> && requires(
            typename ContainerType::value_type value,
            typename ContainerType::allocator_type alloc,
            ContainerType instance //
        )
        {
            requires ::std::constructible_from<ContainerType, decltype(alloc)> &&
                allocator_req<decltype(alloc)> &&
                (container_copy_insertable<ContainerType> &&
                     ::std::constructible_from<
                         ContainerType,
                         const ContainerType&,
                         decltype(alloc) // clang-format off
                     > || // clang-format on
                 container_move_insertable<ContainerType> &&
                     ::std::constructible_from<ContainerType, ContainerType, decltype(alloc)>);

            ::std::allocator_traits<decltype(alloc)>:: //
                propagate_on_container_move_assignment::value ||
                container_move_insertable<ContainerType>&&
                    concepts::move_assignable<decltype(value)>;
            requires ::std::same_as<decltype(value), typename decltype(alloc)::value_type>;

            // clang-format off
            { instance.get_allocator() } -> ::std::same_as<decltype(alloc)>; // clang-format on
        };

        template<typename ContainerType>
        concept sequence_container_req = container<ContainerType> && requires(
            ContainerType instance,
            const ContainerType& const_instance,
            typename ContainerType::value_type value,
            const decltype(value)& const_value,
            typename ContainerType::iterator iter,
            typename ContainerType::const_iterator const_iter,
            typename ContainerType::size_type size //
        )
        {
            requires !container_copy_insertable<ContainerType> ||
                ::std::constructible_from<ContainerType, decltype(size), decltype(value)>;

            requires !container_emplace_constructible<ContainerType, decltype(value)> ||
                ::std::constructible_from<
                    ContainerType,
                    decltype(const_iter),
                    decltype(const_iter)> &&
                    ::std::constructible_from<
                        ContainerType,
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

            requires !container_move_insertable<ContainerType> || requires
            { // clang-format off
                { instance.insert(const_iter, ::std::move(value)) } ->
                    ::std::same_as<decltype(iter)>;

                requires !container_copy_insertable<ContainerType> || requires
                {
                    { instance.insert(const_iter, const_value) } ->
                        ::std::same_as<decltype(iter)>; // clang-format on

                    requires !concepts::copy_assignable<decltype(value)> ||
                        requires(decltype(size) n)
                    {
                        requires ::std::assignable_from<
                            ContainerType&,
                            ::std::initializer_list<decltype(value)> // clang-format off
                        >;

                        { instance.insert(const_iter, n, const_value) } ->
                             ::std::same_as<decltype(iter)>;

                        { instance.assign(n, const_value) } -> ::std::same_as<void>;
                    };
                };
            };

            { instance.front() } -> ::std::same_as<typename ContainerType::reference>;
            { const_instance.front() } ->
                ::std::same_as<typename ContainerType::const_reference>; // clang-format on
        };

        template<typename ContainerType>
        concept reversible_container_req = container<ContainerType> && requires(
            ContainerType instance,
            const ContainerType& const_instance,
            typename ContainerType::reverse_iterator iter,
            typename ContainerType::const_reverse_iterator const_iter //
        )
        { // clang-format off
                { instance.rbegin() } -> ::std::same_as<decltype(iter)>;
                { instance.rend() } -> ::std::same_as<decltype(iter)>;
                { const_instance.rbegin() } -> ::std::same_as<decltype(const_iter)>;
                { const_instance.rend() } -> ::std::same_as<decltype(const_iter)>;
                { instance.crbegin() } -> ::std::same_as<decltype(const_iter)>;
                { instance.crend() } -> ::std::same_as<decltype(const_iter)>; // clang-format on
        };

        template<typename Handle>
        concept node_handle_req = ::std::constructible_from<Handle> && //
            ::std::destructible<Handle> && //
            concepts::nothrow_movable<Handle> &&
            // clang-format off
            requires(Handle handle)
            {
                requires noexcept(static_cast<bool>(handle));
                { ::std::as_const(handle).get_allocator() } ->
                    ::std::same_as<typename Handle::allocator_type>;
                requires noexcept(::std::as_const(handle).empty());
                { ::std::as_const(handle).empty() } -> ::std::same_as<bool>;
            }; // clang-format on

        template<typename T, typename Iterator, typename NodeType>
        concept associative_insert_return_type_req = concepts::aggregate<T> && //
            ::std::constructible_from<T, Iterator, bool, NodeType> && //
            ::std::is_standard_layout_v<T> && //
            requires(T instance) // clang-format off
            {
                { instance.position } -> ::std::same_as<Iterator&>;
                { instance.inserted } -> ::std::same_as<bool&>;
                { instance.node } -> ::std::same_as<NodeType&>;
            }; // clang-format on

        template<typename ContainerType>
        concept associative_container_req = container<ContainerType> && requires(
            ContainerType instance,
            const ContainerType& const_instance,
            typename ContainerType::value_type value,
            typename ContainerType::key_type key,
            typename ContainerType::key_compare key_cmp,
            typename ContainerType::value_compare value_cmp,
            typename ContainerType::node_type node,
            allocator_from_container_t<ContainerType> alloc,
            typename ContainerType::iterator iter,
            typename ContainerType::const_iterator const_iter,
            typename ContainerType::size_type size,
            ::std::initializer_list<decltype(value)> v_list //
        )
        {
            requires ::std::copyable<decltype(key_cmp)> && //
                ::std::copyable<decltype(value_cmp)> && //
                ::std::predicate<decltype(key_cmp), decltype(key), decltype(key)> && //
                ::std::predicate<decltype(value_cmp), decltype(value), decltype(value)> && //
                details::node_handle_req<decltype(node)>;

            requires !container_emplace_constructible<ContainerType, decltype(value)> ||
                ( //
                    !::std::default_initializable<decltype(key_cmp)> ||
                    ::std::constructible_from<
                        ContainerType,
                        decltype(const_iter),
                        decltype(const_iter) // clang-format off
                    > &&
                    ::std::constructible_from<ContainerType, decltype(v_list)> //
                ) && // clang-format on
                    ::std::constructible_from<
                        ContainerType,
                        decltype(const_iter),
                        decltype(const_iter),
                        decltype(key_cmp) // clang-format off
                    > &&
                    ::std::constructible_from<ContainerType, decltype(v_list), decltype(key_cmp)> &&
                    requires
                    {
                        { instance.insert(v_list) } -> ::std::same_as<void>;
                        { instance.emplace_hint(const_iter, ::std::as_const(value)) } ->
                            ::std::same_as<decltype(iter)>;

                        { instance.insert(const_iter, const_iter) } -> ::std::same_as<void>;
                    }; // clang-format on

            requires !container_move_insertable<ContainerType> || requires
            { // clang-format off

                { instance.insert(const_iter, ::std::move(value)) } -> ::std::same_as<decltype(iter)>;

                requires !container_copy_insertable<ContainerType> || requires
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
            { instance.clear() } -> ::std::same_as<void>;
        }; // clang-format on

        template<typename ContainerType>
        concept unordered_associative_container_req = container<ContainerType> && requires(
            ContainerType instance,
            const ContainerType& const_instance,
            typename ContainerType::value_type value,
            typename ContainerType::key_type key,
            typename ContainerType::key_equal key_equal,
            typename ContainerType::hasher hasher,
            typename ContainerType::node_type node,
            allocator_from_container_t<ContainerType> alloc,
            typename ContainerType::reference ref,
            typename ContainerType::const_reference const_ref,
            typename ContainerType::iterator iter,
            typename ContainerType::const_iterator const_iter,
            typename ContainerType::local_iterator local_iter,
            typename ContainerType::const_local_iterator const_local_iter,
            typename ContainerType::difference_type diff,
            typename ContainerType::size_type size,
            ::std::initializer_list<decltype(value)> v_list //
        )
        {
            requires details::iterator_identical<decltype(iter), decltype(local_iter)> &&
                details::iterator_identical<decltype(const_iter), decltype(const_local_iter)> &&
                ::std::copyable<decltype(key_equal)> && //
                ::std::copyable<decltype(hasher)> && //
                ::std::predicate<decltype(key_equal), decltype(key), decltype(key)> &&
                concepts::invocable_r<decltype(hasher), ::std::size_t, decltype(key)> &&
                details::node_handle_req<decltype(node)> &&( //
                    ::std::default_initializable<decltype(key_equal)> ?
                        (!::std::default_initializable<decltype(hasher)> ||
                         ::std::constructible_from<ContainerType, decltype(size)>) :
                        ::std::
                            constructible_from<ContainerType, decltype(size), decltype(hasher)> //
                    ) && //
                ::std::constructible_from<
                    ContainerType,
                    decltype(size),
                    decltype(hasher),
                    decltype(key_equal) // clang-format off
                > &&
                ::std::constructible_from< // clang-format on
                    ContainerType,
                    decltype(const_iter),
                    decltype(const_iter),
                    decltype(size),
                    decltype(hasher),
                    decltype(key_equal) // clang-format off
                >; // clang-format on

            requires !container_emplace_constructible<ContainerType, decltype(value)> ||
                ( //
                    ::std::default_initializable<decltype(key_equal)> ?
                        (!::std::default_initializable<decltype(hasher)> ||
                         ::std::constructible_from<ContainerType, decltype(size)> &&
                             ::std::constructible_from<
                                 ContainerType,
                                 decltype(const_iter),
                                 decltype(const_iter),
                                 decltype(size) // clang-format off
                          > && // clang-format on
                             ::std::constructible_from<
                                 ContainerType,
                                 decltype(v_list),
                                 decltype(size)>) :
                        ::std::constructible_from<
                            ContainerType,
                            decltype(size),
                            decltype(hasher) // clang-format off
                        > && // clang-format on
                            ::std::constructible_from<
                                ContainerType,
                                decltype(const_iter),
                                decltype(const_iter),
                                decltype(size),
                                decltype(hasher) // clang-format off
                            > && // clang-format on
                            ::std::constructible_from<
                                ContainerType,
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

            requires !container_move_insertable<ContainerType> || requires
            {
                { instance.insert(const_iter, ::std::move(value)) } ->
                    ::std::same_as<decltype(iter)>;

                requires container_copy_insertable<ContainerType> || requires
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

            { instance.max_load_factor(float{}) } -> ::std::same_as<void>;
        }; // clang-format on
    }

    template<typename Container>
    concept reversible_container = details::reversible_container_req<Container>;

    template<typename Container>
    concept allocator_aware_container = requires
    {
        requires details::allocator_aware_container_req<::std::decay_t<Container>>;
    };

    template<typename Container>
    concept sequence_container = details::std_array<Container> || requires
    {
        requires details::sequence_container_req<::std::decay_t<Container>>;
    };

    template<typename Container>
    concept contiguous_container =
        container<Container> && ::std::ranges::contiguous_range<::std::decay_t<Container>>;

    template<typename Container>
    concept associative_container = requires
    {
        requires details::associative_container_req<::std::decay_t<Container>>;
    };

    template<typename Container>
    concept unordered_associative_container = requires
    {
        requires details::unordered_associative_container_req<::std::decay_t<Container>>;
    };

    template<typename Predicate, typename Container>
    concept container_predicatable =
        ::std::predicate<Predicate, ::std::ranges::range_value_t<Container>>;
}
