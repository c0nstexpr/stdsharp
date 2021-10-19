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

namespace stdsharp::containers
{
    namespace details
    {
        template<typename ContainerType, typename DecayT = ::std::decay_t<ContainerType>>
        concept std_array = requires
        {
            ::std::tuple_size<DecayT>{};
            requires ::std::same_as<
                ::std::array< //
                    typename DecayT::value_type,
                    ::std::tuple_size_v<DecayT> // clang-format off
                >,
                DecayT
            >; // clang-format on
        };
    }

    template<typename>
    struct allocator_from_container;

    template<typename ContainerType>
        requires(!::stdsharp::containers::details::std_array<ContainerType>)
    struct allocator_from_container<ContainerType> :
        std::type_identity<std::allocator<typename ContainerType::value_type>>
    {
    };

    template<typename ContainerType>
        requires requires
        {
            requires ::stdsharp::memory::allocator_req<typename ContainerType::allocator_type>;
        }
    struct allocator_from_container<ContainerType> :
        std::type_identity<typename ContainerType::allocator_type>
    {
    };

    template<typename ContainerType>
    using allocator_from_container_t = typename allocator_from_container<ContainerType>::type;

    template<typename ValueType, typename Allocator>
    concept erasable = ::std::same_as<
        Allocator, // clang-format off
        typename ::std::allocator_traits<Allocator>::template rebind_alloc<ValueType>
    > &&
        ::std::destructible<ValueType> &&
        requires(Allocator allocator_instance, ValueType* ptr)
        {
            std::allocator_traits<Allocator>::destroy(allocator_instance, ptr);
        }; // clang-format on

    template<typename Container>
    concept container_erasable = requires
    {
        requires ::stdsharp::containers::erasable<
            typename ::std::decay_t<Container>::value_type,
            ::stdsharp::containers::allocator_from_container_t<
                ::std::decay_t<Container>> // clang-format off
        >; // clang-format on
    };

    template<typename ValueType, typename Allocator>
    concept move_insertable = ::std::same_as<
        Allocator, // clang-format off
        typename ::std::allocator_traits<Allocator>::template rebind_alloc<ValueType>
    > &&
        ::std::move_constructible<ValueType> &&
        requires(Allocator allocator_instance, ValueType* ptr, ValueType&& rv)
        {
            std::allocator_traits<Allocator>::construct(
                allocator_instance,
                ptr,
                ::std::move(rv) //
            );
        }; // clang-format on

    template<typename Container>
    concept container_move_insertable = requires
    {
        requires ::stdsharp::containers::move_insertable<
            typename ::std::decay_t<Container>::value_type,
            ::stdsharp::containers:: // clang-format off
                allocator_from_container_t<::std::decay_t<Container>>
        >; // clang-format on
    };

    template<typename ValueType, typename Allocator>
    concept copy_insertable =
        ::stdsharp::containers::move_insertable<ValueType, Allocator> && // clang-format off
        ::std::copy_constructible<ValueType> &&
        requires(Allocator allocator_instance, ValueType* ptr, ValueType v)
        {
            std::allocator_traits<Allocator>::construct(allocator_instance, ptr, v);
        }; // clang-format on

    template<typename Container>
    concept container_copy_insertable = requires
    {
        requires ::stdsharp::containers::copy_insertable<
            typename ::std::decay_t<Container>::value_type,
            ::stdsharp::containers::allocator_from_container_t<
                ::std::decay_t<Container>> // clang-format off
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
            std::allocator_traits<Allocator>::construct(
                allocator_instance,
                ptr,
                ::std::forward<Args>(args)...
            );
        }; // clang-format on

    template<typename Container, typename... Args>
    concept container_emplace_constructible = requires
    {
        requires ::stdsharp::containers::emplace_constructible<
            typename ::std::decay_t<Container>::value_type,
            ::stdsharp::containers::allocator_from_container_t<::std::decay_t<Container>>,
            Args... // clang-format off
        >; // clang-format on
    };

    namespace details
    {
        template<
            typename T,
            typename U,
            typename TTraits = ::std::iterator_traits<T>,
            typename UTraits = ::std::iterator_traits<U> // clang-format off
        > // clang-format on
        concept iterator_identical = requires
        {
            requires ::std::same_as<typename TTraits::value_type, typename UTraits::value_type>;
            requires ::std::
                same_as<typename TTraits::difference_type, typename UTraits::difference_type>;
            requires ::std::same_as<typename TTraits::pointer, typename UTraits::pointer>;
            requires ::std::same_as<typename TTraits::reference, typename UTraits::reference>;
        };

        template<
            typename ContainerType,
            typename ValueType,
            typename Allocator,
            typename... OtherMemberType // clang-format off
        > // clang-format on
        concept container_special_member = //
            (!(::std::default_initializable<OtherMemberType> && ...) ||
             ::std::default_initializable<ContainerType>)&& //
            ::std::destructible<ContainerType> &&
            (::stdsharp::containers::container_copy_insertable<ContainerType> &&
                 (::std::copyable<OtherMemberType> && ...) && //
                 ::std::copyable<ContainerType> ||
             (::std::movable<OtherMemberType> && ...) && //
                 ::std::movable<ContainerType> &&
                 ::stdsharp::concepts::copy_assignable<ContainerType>);

        template<typename ContainerType, typename ValueType, typename Allocator>
        struct container_special_member_req :
            ::std::bool_constant<::stdsharp::containers::details::
                                     container_special_member<ContainerType, ValueType, Allocator>>
        {
        };

        template<typename ContainerType, typename ValueType, typename Allocator>
            requires requires
            {
                typename ContainerType::key_compare;
                typename ContainerType::value_compare;
                typename ContainerType::key_type;
            }
        struct container_special_member_req<ContainerType, ValueType, Allocator> :
            ::std::bool_constant< //
                ::stdsharp::containers::details::container_special_member<
                    ContainerType,
                    ValueType,
                    Allocator,
                    typename ContainerType::key_compare,
                    typename ContainerType::value_compare // clang-format off
                >
            > // clang-format on
        {
        };

        template<typename ContainerType, typename ValueType, typename Allocator>
            requires requires
            {
                typename ContainerType::key_equal;
                typename ContainerType::hasher;
                typename ContainerType::key_type;
            }
        struct container_special_member_req<ContainerType, ValueType, Allocator> :
            ::std::bool_constant< //
                ::stdsharp::containers::details::container_special_member<
                    ContainerType,
                    ValueType,
                    Allocator,
                    typename ContainerType::key_equal,
                    typename ContainerType::hasher // clang-format off
                >
            > // clang-format on
        {
        };

        template<
            typename ContainerType,
            typename ValueType = typename ContainerType::value_type,
            typename Allocator = ::stdsharp::containers::allocator_from_container_t<ContainerType>,
            typename RefType = typename ContainerType::reference,
            typename ConstRefType = typename ContainerType::const_reference,
            typename Iter = typename ContainerType::iterator,
            typename ConstIter = typename ContainerType::const_iterator,
            typename DifferenceType = typename ContainerType::difference_type,
            typename SizeType = typename ContainerType::size_type // clang-format off
        > // clang-format on
        concept container_req =
            ::stdsharp::containers::container_erasable<ContainerType> &&
            ::std::ranges::forward_range<ContainerType> && //
            ::stdsharp::containers::details::
                container_special_member_req<ContainerType, ValueType, Allocator>::value &&
            (!::std::equality_comparable<ValueType> ||
             ::std::equality_comparable<ContainerType>)&& //
            ::std::same_as<ValueType, ::std::ranges::range_value_t<ContainerType>> && //
            (::std::same_as<RefType, ::std::ranges::range_reference_t<ContainerType>> ||
             ::std::same_as<ConstRefType, ::std::ranges::range_reference_t<ContainerType>>)&& //
            ::std::same_as<RefType, ::std::add_lvalue_reference_t<ValueType>>&& //
            ::std::same_as<
                ConstRefType,
                ::stdsharp::type_traits::add_const_lvalue_ref_t<ValueType> // clang-format off
            > && // clang-format on
            ::std::same_as<Iter, ::std::ranges::iterator_t<ContainerType>>&& //
            ::std::same_as<ConstIter, ::stdsharp::ranges::const_iterator_t<ContainerType>>&& //
            ::std::numeric_limits<DifferenceType>::is_signed&& //
            ::std::same_as<DifferenceType, ::std::ranges::range_difference_t<ContainerType>> &&
            !::std::numeric_limits<SizeType>::is_signed &&
            ::std::same_as<SizeType, ::std::ranges::range_size_t<ContainerType>> &&
            (::std::numeric_limits<SizeType>::max() >
             ::std::numeric_limits<DifferenceType>::max()) && // clang-format off
            requires(const ContainerType instance)
            {
                { instance.cbegin() } -> ::std::same_as<ConstIter>;
                { instance.cend() } -> ::std::same_as<ConstIter>;
            } &&
            requires(ContainerType instance)
            {
                { instance.begin() } -> ::std::same_as<Iter>;
                { instance.end() } -> ::std::same_as<Iter>;
                { instance.size() } -> ::std::same_as<SizeType>;
                { instance.max_size() } -> ::std::same_as<SizeType>;
                { instance.empty() } -> ::std::convertible_to<bool>;
            }; // clang-format on
    }

    template<typename Container>
    concept container = requires
    {
        requires ::stdsharp::containers::details::container_req<::std::decay_t<Container>>;
    };

    namespace details
    {
        template<
            typename ContainerType,
            typename ValueType = typename ContainerType::value_type,
            typename Allocator = typename ContainerType::allocator_type // clang-format off
        > // clang-format on
        concept allocator_aware_container_req =
            ::stdsharp::containers::details::container_req<ContainerType, ValueType, Allocator> &&
            ::std::constructible_from<ContainerType, Allocator> && //
            (::stdsharp::containers::container_copy_insertable<ContainerType> &&
                 ::std::constructible_from<ContainerType, const ContainerType&, Allocator> ||
             ::stdsharp::containers::container_move_insertable<ContainerType> &&
                 ::std::constructible_from<ContainerType, ContainerType, Allocator>)&&
            // clang-format off
            requires
            {
                std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value ||
                    ::stdsharp::containers::container_move_insertable<ContainerType> &&
                    ::stdsharp::concepts::move_assignable<ValueType>;
                requires ::std::same_as<ValueType, typename Allocator::value_type>;
            } &&
            requires(ContainerType instance)
            {
                { instance.get_allocator() } -> ::std::same_as<Allocator>;
            }; // clang-format on

        template<
            typename ContainerType,
            typename ValueType = typename ContainerType::value_type,
            typename RefType = typename ContainerType::reference,
            typename ConstRefType = typename ContainerType::const_reference,
            typename Allocator = ::stdsharp::containers::allocator_from_container_t<ContainerType>,
            typename Iter = typename ContainerType::iterator,
            typename ConstIter = typename ContainerType::const_iterator,
            typename SizeType = typename ContainerType::size_type // clang-format off
        > // clang-format on
        concept sequence_container_req = ::stdsharp::containers::container<ContainerType> &&
            (!::stdsharp::containers::container_copy_insertable<ContainerType> ||
             ::std::constructible_from<
                 ContainerType,
                 SizeType,
                 ValueType>)&& // clang-format off
            requires(ContainerType instance, ConstIter const_iter, ValueType value)
            {
                requires !::stdsharp::containers::container_emplace_constructible<ContainerType, ValueType> ||
                    ::std::constructible_from<ContainerType, ConstIter, ConstIter> &&
                    ::std::constructible_from<ContainerType, ::std::initializer_list<ValueType>> &&
                    requires(::std::initializer_list<ValueType> v_list)
                    {
                        { instance.insert(const_iter, v_list) } -> ::std::same_as<Iter>;
                        { instance.assign(v_list) } -> ::std::same_as<void>;
                    } &&
                    requires
                    {
                        { instance.emplace(const_iter, ::std::as_const(value)) } -> ::std::same_as<Iter>;

                        { instance.insert(const_iter, const_iter, const_iter) } -> ::std::same_as<Iter>;
                        { instance.assign(const_iter, const_iter) } -> ::std::same_as<void>;

                        { instance.erase(const_iter) } -> ::std::same_as<Iter>;
                        { instance.erase(const_iter, const_iter) } -> ::std::same_as<Iter>;

                        { instance.clear() } -> ::std::same_as<void>;
                    };

                requires !::stdsharp::containers::container_move_insertable<ContainerType> ||
                    requires
                    {
                        { instance.insert(const_iter, ::std::move(value)) } -> ::std::same_as<Iter>;

                        requires !::stdsharp::containers::copy_insertable<
                            ValueType,
                            Allocator
                        > || requires
                        {
                            { instance.insert(const_iter, ::std::as_const(value)) } ->
                                ::std::same_as<Iter>;

                            requires !::stdsharp::concepts::copy_assignable<ValueType> || requires(SizeType n)
                            {
                                requires ::std::assignable_from<
                                    ContainerType&,
                                    ::std::initializer_list<ValueType>
                                >;

                                { instance.insert(const_iter, n, ::std::as_const(value)) } ->
                                    ::std::same_as<Iter>;

                                { instance.assign(n, ::std::as_const(value)) } ->
                                    ::std::same_as<void>;
                            };
                        };
                    };

                { instance.front() } -> ::std::same_as<RefType>;
                { ::std::as_const(instance).front() } -> ::std::same_as<ConstRefType>;
            }; // clang-format on

        template<
            typename ContainerType,
            typename RIter = typename ContainerType::reverse_iterator,
            typename ConstRIter = typename ContainerType::const_reverse_iterator
            // clang-format off
        >
        concept reversible_container_req = ::stdsharp::containers::container<ContainerType> &&
            requires(ContainerType instance)
            {
                { instance.rbegin() } -> ::std::same_as<RIter>;
                { instance.rend() } -> ::std::same_as<RIter>;
                { ::std::as_const(instance).rbegin() } -> ::std::same_as<ConstRIter>;
                { ::std::as_const(instance).rend() } -> ::std::same_as<ConstRIter>;
                { instance.crbegin() } -> ::std::same_as<ConstRIter>;
                { instance.crend() } -> ::std::same_as<ConstRIter>;
            }; // clang-format on

        template<typename Handle>
        concept node_handle_req = ::std::constructible_from<Handle> && //
            ::std::destructible<Handle> && ::stdsharp::concepts::nothrow_movable<Handle> &&
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
        concept associative_insert_return_type_req = ::stdsharp::concepts::aggregate<T> && //
            ::std::constructible_from<T, Iterator, bool, NodeType> && //
            ::std::is_standard_layout_v<T> && //
            requires(T instance) // clang-format off
            {
                { instance.position } -> std::same_as<Iterator&>;
                { instance.inserted } -> std::same_as<bool&>;
                { instance.node } -> std::same_as<NodeType&>;
            }; // clang-format on
        template<
            typename ContainerType,
            typename ValueType = typename ContainerType::value_type,
            typename KeyType = typename ContainerType::key_type,
            typename KeyCmp = typename ContainerType::key_compare,
            typename ValueCmp = typename ContainerType::value_compare,
            typename NodeType = typename ContainerType::node_type,
            typename Allocator = ::stdsharp::containers::allocator_from_container_t<ContainerType>,
            typename Iter = typename ContainerType::iterator,
            typename ConstIter = typename ContainerType::const_iterator,
            typename SizeType = typename ContainerType::size_type // clang-format off
        > // clang-format on
        concept associative_container_req = ::stdsharp::containers::container<ContainerType> && //
            ::std::copyable<KeyCmp> && //
            ::std::copyable<ValueCmp> && //
            ::std::predicate<KeyCmp, KeyType, KeyType> && //
            ::std::predicate<ValueCmp, ValueType, ValueType> && //
            ::stdsharp::containers::details::node_handle_req<NodeType> && //
            requires(
                ContainerType instance,
                ConstIter const_iter,
                ValueType value // clang-format off
            )
            {
                requires !::stdsharp::containers::container_emplace_constructible<ContainerType, ValueType> ||
                    (!::std::default_initializable<KeyCmp> ||
                        ::std::constructible_from<ContainerType, ConstIter, ConstIter> &&
                    ::std::constructible_from<ContainerType, ::std::initializer_list<ValueType>>) &&
                    ::std::constructible_from<ContainerType, ConstIter, ConstIter, KeyCmp> &&
                    ::std::constructible_from<
                        ContainerType,
                        ::std::initializer_list<ValueType>,
                        KeyCmp
                    > &&
                    requires(::std::initializer_list<ValueType> v_list)
                    {
                        { instance.insert(v_list) } -> ::std::same_as<void>;
                    } &&
                    requires
                    {
                        { instance.emplace_hint(const_iter, ::std::as_const(value)) } ->
                            ::std::same_as<Iter>;

                        { instance.insert(const_iter, const_iter) } -> ::std::same_as<void>;
                    };

                requires !::stdsharp::containers::container_move_insertable<ContainerType> ||
                    requires
                    {
                        { instance.insert(const_iter, ::std::move(value)) } -> ::std::same_as<Iter>;

                        requires !::stdsharp::containers::copy_insertable<
                            ValueType,
                            Allocator
                        > || requires
                        {
                            { instance.insert(const_iter, ::std::as_const(value)) } ->
                                ::std::same_as<Iter>;
                        };
                    };

                { instance.key_comp() } -> ::std::same_as<KeyCmp>;
                { instance.value_comp() } -> ::std::same_as<ValueCmp>;

                { instance.extract(const_iter) } -> ::std::same_as<NodeType>;

                requires requires(KeyType key)
                {
                    { instance.extract(key) } -> ::std::same_as<NodeType>;
                    { instance.erase(key) } -> ::std::same_as<SizeType>;
                    { instance.find(key) } -> ::std::same_as<Iter>;
                    { ::std::as_const(instance).find(key) } -> ::std::same_as<ConstIter>;
                    { ::std::as_const(instance).count(key) } -> ::std::same_as<SizeType>;
                    { ::std::as_const(instance).contains(key) } -> ::std::same_as<bool>;
                    { instance.lower_bound(key) } -> ::std::same_as<Iter>;
                    { ::std::as_const(instance).lower_bound(key) } -> ::std::same_as<ConstIter>;
                    { instance.upper_bound(key) } -> ::std::same_as<Iter>;
                    { ::std::as_const(instance).upper_bound(key) } -> ::std::same_as<ConstIter>;
                    { instance.equal_range(key) } -> ::std::same_as<::std::pair<Iter, Iter>>;
                    { ::std::as_const(instance).equal_range(key) } ->
                        ::std::same_as<::std::pair<ConstIter, ConstIter>>;
                };

                requires requires(NodeType&& node)
                {
                    { instance.insert(const_iter, ::std::move(node)) } -> ::std::same_as<Iter>;
                };

                { instance.erase(const_iter) } -> ::std::same_as<Iter>;
                { instance.erase(const_iter, const_iter) } -> ::std::same_as<Iter>;

                requires requires(Iter iter)
                {
                    { instance.erase(iter) } -> ::std::same_as<Iter>;
                };

                { instance.merge(instance) } -> ::std::same_as<void>;
                { instance.clear() } -> ::std::same_as<void>;
            }; // clang-format on

        template<
            typename ContainerType,
            typename ValueType = typename ContainerType::value_type,
            typename KeyType = typename ContainerType::key_type,
            typename KeyEqual = typename ContainerType::key_equal,
            typename Hasher = typename ContainerType::hasher,
            typename NodeType = typename ContainerType::node_type,
            typename Allocator = ::stdsharp::containers::allocator_from_container_t<ContainerType>,
            typename RefType = typename ContainerType::reference,
            typename ConstRefType = typename ContainerType::const_reference,
            typename Iter = typename ContainerType::iterator,
            typename ConstIter = typename ContainerType::const_iterator,
            typename LocalIter = typename ContainerType::local_iterator,
            typename ConstLocalIter = typename ContainerType::const_local_iterator,
            typename DifferenceType = typename ContainerType::difference_type,
            typename SizeType = typename ContainerType::size_type // clang-format off
        > // clang-format on
        concept unordered_associative_container_req =
            ::stdsharp::containers::container<ContainerType> &&
            ::stdsharp::containers::details::iterator_identical<Iter, LocalIter> &&
            ::stdsharp::containers::details::iterator_identical<ConstIter, ConstLocalIter> &&
            ::std::copyable<KeyEqual> && //
            ::std::copyable<Hasher> && //
            ::std::predicate<KeyEqual, KeyType, KeyType> &&
            ::stdsharp::concepts::invocable_r<Hasher, ::std::size_t, KeyType> &&
            ::stdsharp::containers::details::node_handle_req<NodeType> &&
            (::std::default_initializable<KeyEqual> ?
                 (!::std::default_initializable<Hasher> ||
                  ::std::constructible_from<ContainerType, SizeType>) :
                 ::std::constructible_from<ContainerType, SizeType, Hasher>)&& //
            ::std::constructible_from<ContainerType, SizeType, Hasher, KeyEqual>&& //
            ::std::constructible_from< //
                ContainerType,
                ConstIter,
                ConstIter,
                SizeType,
                Hasher,
                KeyEqual // clang-format off
            > && // clang-format on
            requires(ContainerType instance, ConstIter const_iter, ValueType value)
        // clang-format off
            {
                requires !::stdsharp::containers::container_emplace_constructible<ContainerType, ValueType> ||
                (::std::default_initializable<KeyEqual> ?
                 (!::std::default_initializable<Hasher> ||
                  ::std::constructible_from<ContainerType, SizeType> &&
                      ::std::constructible_from<ContainerType, ConstIter, ConstIter, SizeType> &&
                      ::std::constructible_from<
                          ContainerType,
                          ::std::initializer_list<ValueType>,
                          SizeType
                      >) :
                 ::std::constructible_from<ContainerType, SizeType, Hasher> &&
                     ::std::constructible_from<
                         ContainerType,
                         ConstIter,
                         ConstIter,
                         SizeType,
                         Hasher
                     > &&
                     ::std::constructible_from<
                         ContainerType,
                         ::std::initializer_list<ValueType>,
                         SizeType,
                         Hasher
                     >)&&
                requires
                {
                    { instance.emplace_hint(const_iter, ::std::as_const(value)) } ->
                        ::std::same_as<Iter>;

                    { instance.insert(const_iter, const_iter) } -> ::std::same_as<void>;
                    requires requires(::std::initializer_list<ValueType> v_list)
                    {
                        { instance.insert(v_list) } -> ::std::same_as<void>;
                    };
                };

                requires !::stdsharp::containers::container_move_insertable<ContainerType> ||
                    requires
                    {
                        { instance.insert(const_iter, ::std::move(value)) } -> ::std::same_as<Iter>;

                        !::stdsharp::containers::copy_insertable<
                            ValueType,
                            Allocator
                        > || requires
                        {
                            { instance.insert(const_iter, ::std::as_const(value)) } ->
                                ::std::same_as<Iter>;
                        };
                    };

                { instance.key_eq() } -> ::std::same_as<KeyEqual>;
                { instance.hash_function() } -> ::std::same_as<Hasher>;

                { instance.extract(const_iter) } -> ::std::same_as<NodeType>;
                requires requires(KeyType key)
                {
                    { instance.extract(key) } -> ::std::same_as<NodeType>;
                    { instance.erase(key) } -> ::std::same_as<SizeType>;
                    { instance.find(key) } -> ::std::same_as<Iter>;
                    { ::std::as_const(instance).find(key) } -> ::std::same_as<ConstIter>;
                    { ::std::as_const(instance).count(key) } -> ::std::same_as<SizeType>;
                    { ::std::as_const(instance).bucket(key) } -> ::std::same_as<SizeType>;
                    { ::std::as_const(instance).contains(key) } -> ::std::same_as<bool>;
                    { instance.equal_range(key) } -> ::std::same_as<::std::pair<Iter, Iter>>;
                    { ::std::as_const(instance).equal_range(key) } ->
                        ::std::same_as<::std::pair<ConstIter, ConstIter>>;
                };

                requires requires(NodeType&& node)
                {
                    { instance.insert(const_iter, ::std::move(node)) } -> ::std::same_as<Iter>;
                };

                { instance.erase(const_iter) } -> ::std::same_as<Iter>;
                { instance.erase(const_iter, const_iter) } -> ::std::same_as<Iter>;
                requires requires(Iter iter)
                {
                    { instance.erase(iter) } -> ::std::same_as<Iter>;
                };

                { instance.merge(instance) } -> ::std::same_as<void>;
                { instance.clear() } -> ::std::same_as<void>;

                { ::std::as_const(instance).bucket_count() } -> ::std::same_as<SizeType>;
                { ::std::as_const(instance).max_bucket_count() } -> ::std::same_as<SizeType>;
                { ::std::as_const(instance).load_factor() } -> ::std::same_as<float>;
                { ::std::as_const(instance).max_load_factor() } -> ::std::same_as<float>;

                requires requires(SizeType size_n)
                {
                    { ::std::as_const(instance).bucket_size(size_n) } -> ::std::same_as<SizeType>;
                    { instance.begin(size_n) } -> ::std::same_as<LocalIter>;
                    { instance.end(size_n) } -> ::std::same_as<LocalIter>;
                    { instance.rehash(size_n) } -> ::std::same_as<void>;
                    { instance.reserve(size_n) } -> ::std::same_as<void>;
                    { ::std::as_const(instance).begin(size_n) } -> ::std::same_as<ConstLocalIter>;
                    { ::std::as_const(instance).end(size_n) } -> ::std::same_as<ConstLocalIter>;
                    { ::std::as_const(instance).cbegin(size_n) } -> ::std::same_as<ConstLocalIter>;
                    { ::std::as_const(instance).cend(size_n) } -> ::std::same_as<ConstLocalIter>;
                };

                requires requires(float factor_n)
                {
                    { instance.max_load_factor(factor_n) } -> ::std::same_as<void>;
                };
            }; // clang-format on
    }

    template<typename Container>
    concept reversible_aware_container = requires
    {
        requires ::stdsharp::containers::details::reversible_container_req<
            ::std::decay_t<Container>>;
    };

    template<typename Container>
    concept allocator_aware_container = requires
    {
        requires ::stdsharp::containers::details::allocator_aware_container_req<
            ::std::decay_t<Container>>;
    };

    template<typename Container>
    concept sequence_container = !::stdsharp::containers::details::std_array<Container> || requires
    {
        ::stdsharp::containers::details::sequence_container_req<::std::decay_t<Container>>;
    };

    template<typename Container>
    concept contiguous_container = ::stdsharp::containers::container<Container> &&
        ::std::ranges::contiguous_range<::std::decay_t<Container>>;

    template<typename Container>
    concept associative_container = requires
    {
        requires ::stdsharp::containers::details:: //
            associative_container_req<::std::decay_t<Container>>;
    };

    template<typename Container>
    concept unordered_associative_container = requires
    {
        requires ::stdsharp::containers::details:: //
            unordered_associative_container_req<::std::decay_t<Container>>;
    };

    template<::stdsharp::containers::container T>
    inline constexpr ::stdsharp::functional::invocable_obj forward_container(
        ::stdsharp::functional::nodiscard_tag,
        ::ranges::overload(
            [](::std::decay_t<T>&& t) noexcept(noexcept(t | ::ranges::views::move))
            {
                return t | ::ranges::views::move; //
            },
            ::std::identity{} // clang-format off
        ) // clang-format on
    );

    template<typename T>
    using forward_container_t =
        decltype(::stdsharp::containers::forward_container<T>(::std::declval<T>()));
}
