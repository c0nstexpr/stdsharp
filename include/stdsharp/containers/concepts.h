#pragma once

#include "../memory/allocator_traits.h"
#include "../ranges/ranges.h"
#include "../type_traits/indexed_traits.h"

#include <deque> // IWYU pragma: export
#include <forward_list>
#include <list> // IWYU pragma: export
#include <map> // IWYU pragma: export
#include <set>
#include <type_traits>
#include <unordered_map> // IWYU pragma: export
#include <unordered_set> // IWYU pragma: export
#include <vector> // IWYU pragma: export

namespace stdsharp
{
    template<typename T, auto Size>
    struct allocator_of<std::array<T, Size>>
    {
        using type = std::allocator<T>;
    };

    template<typename Rng, typename ValueType>
    concept compatible_range = ranges::input_range<Rng> &&
        std::convertible_to<ranges::range_reference_t<Rng>, ValueType>;

    template<typename ValueType, typename Allocator>
    concept erasable = requires(Allocator allocator_instance, ValueType* ptr) {
        requires allocator_req<Allocator>;
        requires std::same_as<
            Allocator,
            typename allocator_traits<Allocator>:: //
            template rebind_alloc<ValueType>>;
        requires std::destructible<ValueType>;
        allocator_traits<Allocator>::destroy(allocator_instance, ptr);
    };

    template<typename ValueType, typename Allocator>
    concept nothrow_erasable = requires(Allocator allocator_instance, ValueType* ptr) {
        requires erasable<ValueType, Allocator>;
        requires noexcept(allocator_traits<Allocator>::destroy(allocator_instance, ptr));
    };

    template<typename Container>
    concept container_erasable = requires {
        requires erasable<
            typename std::decay_t<Container>::value_type,
            allocator_of_t<std::decay_t<Container>>>;
    };

    template<typename Container>
    concept container_nothrow_erasable = requires {
        requires nothrow_erasable<
            typename std::decay_t<Container>::value_type,
            allocator_of_t<std::decay_t<Container>>>;
    };

    template<typename ValueType, typename Allocator>
    concept move_insertable =
        requires(Allocator allocator_instance, ValueType* ptr, ValueType&& rv) {
            requires std::same_as<
                Allocator,
                typename allocator_traits<Allocator>::template rebind_alloc<ValueType>>;
            requires std::move_constructible<ValueType>;
            allocator_traits<Allocator>::construct(allocator_instance, ptr, cpp_move(rv));
        };

    template<typename ValueType, typename Allocator>
    concept nothrow_move_insertable =
        requires(Allocator allocator_instance, ValueType* ptr, ValueType&& rv) {
            requires move_insertable<ValueType, Allocator>;
            requires nothrow_move_constructible<ValueType>;
            requires noexcept(allocator_traits<Allocator>::
                                  construct(allocator_instance, ptr, cpp_move(rv)));
        };

    template<typename Container>
    concept container_move_insertable = requires {
        requires move_insertable<
            typename std::decay_t<Container>::value_type,
            allocator_of_t<std::decay_t<Container>>>;
    };

    template<typename Container>
    concept container_nothrow_move_insertable = requires {
        requires nothrow_move_insertable<
            typename std::decay_t<Container>::value_type,
            allocator_of_t<std::decay_t<Container>>>;
    };

    template<typename ValueType, typename Allocator>
    concept copy_insertable = requires(Allocator allocator_instance, ValueType* ptr, ValueType v) {
        requires move_insertable<ValueType, Allocator> && std::copy_constructible<ValueType>;
        allocator_traits<Allocator>::construct(allocator_instance, ptr, v);
    };

    template<typename ValueType, typename Allocator>
    concept nothrow_copy_insertable =
        requires(Allocator allocator_instance, ValueType* ptr, ValueType v) {
            requires nothrow_move_insertable<ValueType, Allocator>;
            requires nothrow_copy_constructible<ValueType>;
            requires noexcept(allocator_traits<Allocator>::construct(allocator_instance, ptr, v));
        };

    template<typename Container>
    concept container_copy_insertable = requires {
        requires copy_insertable<
            typename std::decay_t<Container>::value_type,
            allocator_of_t<std::decay_t<Container>>>;
    };

    template<typename Container>
    concept container_nothrow_copy_insertable = requires {
        requires nothrow_copy_insertable<
            typename std::decay_t<Container>::value_type,
            allocator_of_t<std::decay_t<Container>>>;
    };

    template<typename ValueType, typename Allocator, typename... Args>
    concept emplace_constructible =
        requires(Allocator allocator_instance, ValueType* ptr, Args&&... args) {
            requires std::same_as<
                Allocator,
                typename allocator_traits<Allocator>::template rebind_alloc<ValueType>>;
            requires std::constructible_from<ValueType, Args...>;
            allocator_traits<Allocator>::construct(allocator_instance, ptr, cpp_forward(args)...);
        };

    template<typename ValueType, typename Allocator, typename... Args>
    concept nothrow_emplace_constructible =
        requires(Allocator allocator_instance, ValueType* ptr, Args&&... args) {
            requires nothrow_constructible_from<ValueType, Args...>;
            requires noexcept( //
                allocator_traits<Allocator>::
                    construct(allocator_instance, ptr, cpp_forward(args)...)
            );
        };

    template<typename Container, typename... Args>
    concept container_emplace_constructible = requires {
        requires emplace_constructible<
            typename std::decay_t<Container>::value_type,
            allocator_of_t<std::decay_t<Container>>,
            Args...>;
    };

    template<typename Container, typename... Args>
    concept container_nothrow_emplace_constructible = requires {
        requires nothrow_emplace_constructible<
            typename std::decay_t<Container>::value_type,
            allocator_of_t<std::decay_t<Container>>,
            Args...>;
    };
}

namespace stdsharp::details
{
    template<typename T, typename U>
    concept iterator_identical = requires(
        std::iterator_traits<T> t_traits,
        std::iterator_traits<U> u_traits
    ) {
        requires std::same_as<
            typename decltype(t_traits)::value_type,
            typename decltype(u_traits)::value_type>;
        requires std::same_as<
            typename decltype(t_traits)::difference_type,
            typename decltype(u_traits)::difference_type>;
        requires std::
            same_as<typename decltype(t_traits)::pointer, typename decltype(u_traits)::pointer>;
        requires std::
            same_as<typename decltype(t_traits)::reference, typename decltype(u_traits)::reference>;
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
        ContainerType::value_type value,
        const decltype(value)& const_value,
        ContainerType::iterator iter,
        ContainerType::const_iterator const_iter
    ) //
    {
        requires !container_move_insertable<ContainerType> || requires {
            { instance.insert(const_iter, cpp_move(value)) } -> std::same_as<decltype(iter)>;

            requires !container_copy_insertable<decltype(instance)> || requires {
                { instance.insert(const_iter, const_value) } -> std::same_as<decltype(iter)>;
            };
        };
    };
}

namespace stdsharp
{
    template<typename Iterator, typename Node>
    using insert_return_type =
        details::insert_return_type_of<std::set<int>::insert_return_type>::type<Iterator, Node>;
    template<typename Container, typename... Members>
    concept container = requires {
        typename std::decay_t<Container>;
        requires requires(
            std::decay_t<Container> instance,
            const decltype(instance) const_instance,
            decltype(instance)::value_type value,
            std::ranges::range_reference_t<decltype(instance)> rng_ref,
            decltype(instance)::reference ref,
            decltype(instance)::const_reference const_ref,
            decltype(instance)::iterator iter,
            decltype(instance)::const_iterator const_iter,
            decltype(instance)::difference_type diff,
            decltype(instance)::size_type size
        ) {
            requires std::ranges::forward_range<decltype(instance)>;

            requires std::destructible<decltype(instance)>;

            requires !(nothrow_default_initializable<Members> && ...) ||
                nothrow_default_initializable<decltype(instance)> ||
                !(std::default_initializable<Members> && ...) ||
                std::default_initializable<decltype(instance)>;

            requires !((nothrow_copy_constructible<Members> && ...) &&
                       container_nothrow_copy_insertable<decltype(instance)>) ||
                nothrow_copy_constructible<decltype(instance)> ||
                !(std::copy_constructible<Members> && ...) &&
                    container_copy_insertable<decltype(instance)> ||
                std::copy_constructible<decltype(instance)>;

            requires !((nothrow_copy_assignable<Members> && ...) &&
                       nothrow_copy_assignable<decltype(value)> &&
                       container_nothrow_copy_insertable<decltype(instance)>) ||
                nothrow_copy_assignable<decltype(instance)> ||
                !((copy_assignable<Members> && ...) && copy_assignable<decltype(value)> &&
                  container_copy_insertable<decltype(instance)>) ||
                copy_assignable<decltype(instance)>;

            requires !(std::move_constructible<Members> && ...) ||
                std::move_constructible<decltype(instance)> ||
                !(nothrow_move_constructible<Members> && ...) ||
                nothrow_move_constructible<decltype(instance)>;

            requires !std::equality_comparable<decltype(value)> ||
                std::equality_comparable<decltype(instance)>;

            requires requires(
                allocator_of_t<decltype(instance)> alloc,
                allocator_traits<decltype(alloc)> traits,
                std::bool_constant<decltype(traits)::propagate_on_container_move_assignment::value>
                    propagate
            ) {
                requires container_erasable<decltype(instance)>;

                requires !(decltype(propagate)::value ||
                           container_nothrow_move_insertable<decltype(instance)> &&
                               (nothrow_move_assignable<Members> && ...)) ||
                    nothrow_move_assignable<decltype(instance)> ||
                    !(decltype(propagate)::value ||
                      container_move_insertable<decltype(instance)> &&
                          (move_assignable<Members> && ...)) ||
                    move_assignable<decltype(instance)>;
            };

            requires std::same_as<decltype(value), std::ranges::range_value_t<decltype(instance)>>;
            requires std::same_as<decltype(ref), std::add_lvalue_reference_t<decltype(value)>>;
            requires std::same_as<decltype(const_ref), add_const_lvalue_ref_t<decltype(value)>>;
            requires std::same_as<decltype(iter), std::ranges::iterator_t<decltype(instance)>>;
            requires std::same_as<decltype(iter), std::ranges::sentinel_t<decltype(instance)>>;

            { const_instance.cbegin() } -> std::same_as<decltype(const_iter)>;

            { const_instance.cend() } -> std::same_as<decltype(const_iter)>;

            requires std::same_as<decltype(ref), decltype(rng_ref)> ||
                std::same_as<decltype(const_ref), decltype(rng_ref)>;
            requires std::same_as<decltype(const_ref), range_const_reference_t<decltype(instance)>>;
            requires std::signed_integral<decltype(diff)>;
            requires std::
                same_as<decltype(diff), std::ranges::range_difference_t<decltype(instance)>>;
            requires std::unsigned_integral<decltype(size)>;
            requires requires {
                []<typename T, typename U>(std::forward_list<T, U>*) {}(&instance);
            } || requires {
                requires std::
                    same_as<decltype(size), std::ranges::range_size_t<decltype(instance)>>;
            };
            requires(
                std::numeric_limits<decltype(size)>::max() >
                std::numeric_limits<decltype(diff)>::max()
            );

            { instance.max_size() } -> std::same_as<decltype(size)>;
            { instance.empty() } -> std::convertible_to<bool>;
        };
    };

    template<typename Container, typename... Members>
    concept reversible_container = container<Container, Members...> &&
        requires //
    {
        typename std::decay_t<Container>;
        requires requires(
            std::decay_t<Container> instance,
            const decltype(instance)& const_instance,
            decltype(instance)::reverse_iterator iter,
            decltype(instance)::const_reverse_iterator const_iter
        ) {
            { instance.rbegin() } -> std::same_as<decltype(iter)>;
            { instance.rend() } -> std::same_as<decltype(iter)>;
            { const_instance.rbegin() } -> std::same_as<decltype(const_iter)>;
            { const_instance.rend() } -> std::same_as<decltype(const_iter)>;
            { instance.crbegin() } -> std::same_as<decltype(const_iter)>;
            { instance.crend() } -> std::same_as<decltype(const_iter)>;
        };
    };

    template<typename Container, typename... Members>
    concept allocator_aware_container = requires {
        requires requires(
            std::decay_t<Container> instance,
            decltype(instance)::value_type value,
            decltype(instance)::allocator_type alloc
        ) {
            requires allocator_req<decltype(alloc)>;
            requires container<Container, decltype(alloc), Members...>;
            requires std::constructible_from<decltype(instance), decltype(alloc)>;

            requires !container_copy_insertable<decltype(instance)> ||
                std::constructible_from<
                    decltype(instance),
                    const decltype(instance)&,
                    decltype(alloc)>;
            requires !container_move_insertable<decltype(instance)> ||
                std::constructible_from<decltype(instance), decltype(instance), decltype(alloc)>;

            requires std::same_as<decltype(value), typename decltype(alloc)::value_type>;

            { instance.get_allocator() } -> std::same_as<decltype(alloc)>;
        };
    };

    template<typename Container, typename... Members>
    concept sequence_container = container<Container, Members...> && requires {
        requires requires(
            std::decay_t<Container> instance,
            const decltype(instance)& const_instance,
            decltype(instance)::value_type value,
            const decltype(value)& const_value,
            decltype(instance)::iterator iter,
            decltype(instance)::const_iterator const_iter,
            decltype(instance)::size_type size
        ) {
            requires !container_copy_insertable<decltype(instance)> ||
                std::constructible_from<decltype(instance), decltype(size), decltype(value)>;

            requires !container_emplace_constructible<decltype(instance), decltype(value)> ||
                requires(std::initializer_list<decltype(value)> v_list) {
                    requires std::constructible_from<
                        decltype(instance),
                        decltype(const_iter),
                        decltype(const_iter)>;
                    requires std::constructible_from<decltype(instance), decltype(v_list)>;
                    { instance.insert(const_iter, v_list) } -> std::same_as<decltype(iter)>;
                    { instance.assign(v_list) } -> std::same_as<void>;

                    { instance.emplace(const_iter, const_value) } -> std::same_as<decltype(iter)>;

                    {
                        instance.insert(const_iter, const_iter, const_iter)
                    } -> std::same_as<decltype(iter)>;
                    { instance.assign(const_iter, const_iter) } -> std::same_as<void>;

                    { instance.erase(const_iter) } -> std::same_as<decltype(iter)>;
                    { instance.erase(const_iter, const_iter) } -> std::same_as<decltype(iter)>;

                    { instance.clear() } -> std::same_as<void>;
                };

            requires details::container_insertable<decltype(instance)>;

            requires !(container_copy_insertable<decltype(instance)> &&
                       copy_assignable<decltype(value)>) ||
                requires(decltype(size) n) {
                    requires std::assignable_from<
                        decltype(instance)&,
                        std::initializer_list<decltype(value)>>;

                    { instance.insert(const_iter, n, const_value) } -> std::same_as<decltype(iter)>;

                    { instance.assign(n, const_value) } -> std::same_as<void>;
                };

            { instance.front() } -> std::same_as<typename decltype(instance)::reference>;
            {
                const_instance.front()
            } -> std::same_as<typename decltype(instance)::const_reference>;
        };
    };

    template<typename Container>
    concept contiguous_container = container<Container> &&
        std::ranges::contiguous_range<std::decay_t<Container>>;
}

namespace stdsharp::details
{
    template<typename Container>
    concept unique_associative = requires(
        std::decay_t<Container> instance,
        decltype(instance)::node_type node,
        decltype(instance)::insert_return_type insert_return_v
    ) {
        { instance.insert(cpp_move(node)) } -> std::same_as<decltype(insert_return_v)>;
        requires std::same_as<
            decltype(insert_return_v),
            insert_return_type<decltype(instance.begin()), decltype(node)>>;
    };

    template<typename Container>
    concept multikey_associative =
        requires(std::decay_t<Container> instance, decltype(instance)::node_type node) {
            {
                instance.insert(cpp_move(node))
            } -> std::same_as<typename decltype(instance)::iterator>;
        };

    template<typename Container, typename... Args>
    struct container_optional_constructible
    {
        using size_t = std::size_t;

        template<typename... Optional>
        static constexpr bool value =
            []<size_t... I>(this const auto self, const std::index_sequence<I...>) consteval
        {
            constexpr auto count = sizeof...(I);
            auto res = std::constructible_from<Container, Args..., type_at<I, Optional...>...>;

            if constexpr(count > 0) res = res && self(std::make_index_sequence<count - 1>{});

            return res;
        }(std::make_index_sequence<sizeof...(Optional)>{});
    };
}

namespace stdsharp
{
    template<typename Container, typename... Members>
    concept associative_like_container = allocator_aware_container<Container, Members...> &&
        requires //
    {
        requires requires(
            std::decay_t<Container> instance,
            const decltype(instance)& const_instance,
            decltype(instance)::key_type key,
            decltype(instance)::const_reference const_ref,
            decltype(instance)::value_type value,
            decltype(instance)::node_type node,
            decltype(instance)::iterator iter,
            decltype(instance)::const_iterator const_iter,
            decltype(instance)::size_type size,
            std::initializer_list<decltype(value)> v_list
        ) {
            requires details::container_insertable<decltype(instance)>;

            requires !container_emplace_constructible<decltype(instance), decltype(value)> ||
                requires {
                    {
                        instance.emplace_hint(const_iter, const_ref)
                    } -> std::same_as<decltype(iter)>;
                    { instance.insert(const_iter, const_iter) } -> std::same_as<void>;
                    { instance.insert(v_list) } -> std::same_as<void>;
                };

            { instance.extract(const_iter) } -> std::same_as<decltype(node)>;
            { instance.extract(key) } -> std::same_as<decltype(node)>;

            { instance.erase(key) } -> std::same_as<decltype(size)>;

            { instance.find(key) } -> std::same_as<decltype(iter)>;
            { const_instance.find(key) } -> std::same_as<decltype(const_iter)>;

            { const_instance.count(key) } -> std::same_as<decltype(size)>;

            { const_instance.contains(key) } -> std::same_as<bool>;

            {
                instance.equal_range(key)
            } -> std::same_as<std::pair<decltype(iter), decltype(iter)>>;
            {
                const_instance.equal_range(key)
            } -> std::same_as<std::pair<decltype(const_iter), decltype(const_iter)>>;

            { instance.insert(const_iter, cpp_move(node)) } -> std::same_as<decltype(iter)>;

            { instance.erase(const_iter) } -> std::same_as<decltype(iter)>;
            { instance.erase(const_iter, const_iter) } -> std::same_as<decltype(iter)>;
            { instance.erase(iter) } -> std::same_as<decltype(iter)>;

            { instance.merge(instance) } -> std::same_as<void>;

            { instance.clear() } -> std::same_as<void>;
        };
    };

    template<typename Container, typename... Members>
    concept associative_container = requires {
        requires requires(
            std::decay_t<Container> instance,
            const decltype(instance)& const_instance,
            decltype(instance)::value_type value,
            decltype(instance)::key_type key,
            allocator_of_t<decltype(instance)> alloc,
            decltype(instance)::key_compare key_cmp,
            decltype(instance)::value_compare value_cmp,
            decltype(instance)::iterator iter,
            decltype(instance)::const_iterator const_iter,
            decltype(instance)::size_type size,
            std::initializer_list<decltype(value)> v_list
        ) {
            requires associative_like_container<decltype(instance), decltype(key_cmp), Members...>;

            requires std::copyable<decltype(key_cmp)>;
            requires std::predicate<decltype(key_cmp), decltype(key), decltype(key)>;
            requires std::constructible_from<decltype(instance), const decltype(key_cmp)>;

            requires std::copyable<decltype(value_cmp)>;
            requires std::predicate<decltype(value_cmp), decltype(value), decltype(value)>;

            requires !container_emplace_constructible<decltype(instance), decltype(value)> ||
                details::container_optional_constructible<
                    decltype(instance),
                    decltype(const_iter),
                    decltype(const_iter)>::template value<decltype(key_cmp), decltype(alloc)> &&
                    details::
                        container_optional_constructible<decltype(instance), decltype(v_list)>::
                            template value<decltype(key_cmp), decltype(alloc)>;

            requires !compatible_range<decltype(v_list), decltype(value)> ||
                details::container_optional_constructible<
                    decltype(instance),
                    std::from_range_t,
                    decltype(v_list)>::template value<decltype(key_cmp), decltype(alloc)>;

            { instance.key_comp() } -> std::same_as<decltype(key_cmp)>;
            { instance.value_comp() } -> std::same_as<decltype(value_cmp)>;

            { instance.lower_bound(key) } -> std::same_as<decltype(iter)>;
            { const_instance.lower_bound(key) } -> std::same_as<decltype(const_iter)>;
            { instance.upper_bound(key) } -> std::same_as<decltype(iter)>;
            { const_instance.upper_bound(key) } -> std::same_as<decltype(const_iter)>;
        };
    };

    template<typename Container, typename... Members>
    concept unique_associative_container = associative_container<Container, Members...> &&
        details::unique_associative<Container>;

    template<typename Container, typename... Members>
    concept multikey_associative_container = associative_container<Container, Members...> &&
        details::multikey_associative<Container>;

    template<typename Container, typename... Members>
    concept unordered_associative_container = requires {
        requires requires(
            std::decay_t<Container> instance,
            const decltype(instance)& const_instance,
            decltype(instance)::value_type value,
            decltype(instance)::key_type key,
            decltype(instance)::key_equal key_equal,
            decltype(instance)::hasher hasher,
            allocator_of_t<decltype(instance)> alloc,
            decltype(instance)::reference ref,
            decltype(instance)::const_reference const_ref,
            decltype(instance)::iterator iter,
            decltype(instance)::const_iterator const_iter,
            decltype(instance)::local_iterator local_iter,
            decltype(instance)::const_local_iterator const_local_iter,
            decltype(instance)::difference_type diff,
            decltype(instance)::size_type size,
            std::initializer_list<decltype(value)> v_list
        ) {
            requires associative_like_container<decltype(instance), decltype(hasher), Members...>;

            requires std::copyable<decltype(key_equal)>;
            requires std::predicate<decltype(key_equal), decltype(key), decltype(key)>;

            requires std::copyable<decltype(hasher)>;
            requires invocable_r<decltype(hasher), std::size_t, decltype(key)>;

            requires details::iterator_identical<decltype(iter), decltype(local_iter)>;
            requires details::iterator_identical<decltype(const_iter), decltype(const_local_iter)>;

            requires details::container_optional_constructible<decltype(instance), decltype(size)>::
                template value<decltype(hasher), decltype(key_equal), decltype(alloc)>;

            requires !container_emplace_constructible<decltype(instance), decltype(value)> ||
                requires {
                    requires details::container_optional_constructible<
                        decltype(instance),
                        decltype(const_iter),
                        decltype(const_iter)>::
                        template value<
                            decltype(size),
                            decltype(hasher),
                            decltype(key_equal),
                            decltype(alloc)>;

                    requires details::container_optional_constructible<
                        decltype(instance),
                        decltype(v_list)>:: //
                        template value<
                            decltype(size),
                            decltype(hasher),
                            decltype(key_equal),
                            decltype(alloc)>;

                    requires !compatible_range<decltype(v_list), decltype(value)> ||
                        details::container_optional_constructible<
                            decltype(instance),
                            std::from_range_t,
                            decltype(v_list)>:: //
                        template value<
                            decltype(size),
                            decltype(hasher),
                            decltype(key_equal),
                            decltype(alloc)>;
                };

            requires !container_copy_insertable<decltype(instance)> &&
                    copy_assignable<decltype(value)> &&
                    std::default_initializable<decltype(hasher)> &&
                    std::default_initializable<decltype(key_equal)> &&
                    std::default_initializable<decltype(alloc)> ||
                    std::constructible_from<decltype(instance), decltype(v_list)>;

            { instance.key_eq() } -> std::same_as<decltype(key_equal)>;
            { instance.hash_function() } -> std::same_as<decltype(hasher)>;

            { const_instance.bucket(key) } -> std::same_as<decltype(size)>;
            { const_instance.bucket_count() } -> std::same_as<decltype(size)>;
            { const_instance.max_bucket_count() } -> std::same_as<decltype(size)>;

            { instance.max_load_factor(0.0f) } -> std::same_as<void>;

            { const_instance.load_factor() } -> std::same_as<float>;
            { const_instance.max_load_factor() } -> std::same_as<float>;

            { const_instance.bucket_size(size) } -> std::same_as<decltype(size)>;

            { instance.rehash(size) } -> std::same_as<void>;

            { instance.reserve(size) } -> std::same_as<void>;

            { instance.begin(size) } -> std::same_as<decltype(local_iter)>;
            { instance.end(size) } -> std::same_as<decltype(local_iter)>;

            { const_instance.begin(size) } -> std::same_as<decltype(const_local_iter)>;
            { const_instance.end(size) } -> std::same_as<decltype(const_local_iter)>;
            { const_instance.cbegin(size) } -> std::same_as<decltype(const_local_iter)>;
            { const_instance.cend(size) } -> std::same_as<decltype(const_local_iter)>;
        };
    };

    template<typename Container, typename... Members>
    concept unique_unordered_associative_container =
        unordered_associative_container<Container, Members...> &&
        details::unique_associative<Container>;

    template<typename Container, typename... Members>
    concept multikey_unordered_associative_container =
        unordered_associative_container<Container, Members...> &&
        details::multikey_associative<Container>;

    template<typename Predicate, typename Container>
    concept container_predicate = std::predicate<Predicate, range_const_reference_t<Container>>;
}