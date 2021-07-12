// Created by BlurringShadow at 2021-03-03-下午 4:35

#pragma once
#include "value_sequence.h"

namespace blurringshadow::utility::traits
{
    namespace details
    {
        template<std::size_t I, typename Type>
        struct indexed_type
        {
            // ReSharper disable once CppFunctionIsNotImplemented
            template<std::size_t J>
                requires(I == J)
            static constexpr Type get_from_index() noexcept;
        };

        template<typename... T>
        struct type_list
        {
            template<std::size_t... I>
            struct indexed : private indexed_type<I, T>...
            {
                using indexed_type<I, T>::get_from_index...;

                template<typename Find>
                static constexpr std::array<bool, sizeof...(T)> match()
                {
                    return {std::same_as<Find, T>...};
                }

                static constexpr auto size() noexcept { return sizeof...(T); }
            };
        };
    }

    // clang-format off
    template<typename... Types>
    struct type_container : private from_std_sequence_t<std::index_sequence_for<Types...>>::
        template apply_t<details::type_list<Types...>::template indexed>
    {
    private:
        using base = typename details::type_list<Types...>::template indexed<>;

        // clang-format on
    public:
        using base::size;

        template<std::size_t Index>
        using indexed_t = decltype(base::template get_from_index<Index>());

        template<typename Find>
        static constexpr auto find() noexcept
        {
            constexpr auto query = base::template match<Find>();
            return std::ranges::find(query, true) - query.cbegin();
        }

        template<typename Find>
        static constexpr auto count() noexcept
        {
            constexpr auto query = base::template match<Find>();
            return std::ranges::count(query, true) - query.cbegin();
        }

        template<typename Find>
        static constexpr auto contains() noexcept
        {
            return count<Find> != 0;
        }

        template<template<typename...> typename Type>
        using apply_t = Type<Types...>;

        template<std::size_t... OtherInt>
        using sequence_indexed_t = type_container<typename base::template indexed_t<OtherInt>...>;

        template<typename... Other>
        using append_t = type_container<Types..., Other...>;

        template<typename... Other>
        using append_front_t = type_container<Other..., Types...>;

        template<typename Container>
        using append_container_t = typename Container::template apply_t<append_t>;

        template<typename Container>
        using append_container_front_t = typename Container::template apply_t<append_front_t>;

        template<std::size_t From, std::size_t Size>
        using select_range_indexed_t =
            typename make_sequence<From, Size>::template apply_t<sequence_indexed_t>;

        // clang-format off
        template<std::size_t Index, typename... Other>
        using insert_t = typename select_range_indexed_t<0, Index>::
            template append_t<Other...>::
            template append_container_t<select_range_indexed_t<Index, size() - Index>>;
        // clang-format on

    private:
        template<typename... Other>
        struct insert
        {
            template<std::size_t Index>
            using t = insert_t<Index, Other...>;
        };

    public:
        template<std::size_t Index, typename Container>
        using insert_container_t = typename Container::template apply_t<insert>::template t<Index>;

        using rest_t = select_range_indexed_t<1, size() - 1>;

        // clang-format off
        template<std::size_t Index>
        using except_t = typename select_range_indexed_t<0, Index>::
            template append_container_t<select_range_indexed_t<Index + 1, size() - Index - 1>>;
        // clang-format on
    };

    /*
    namespace details
    {
        template<std::size_t I, typename Type>
        struct unique_indexed_type : indexed_type<I, Type>
        {
            // ReSharper disable once CppFunctionIsNotImplemented
            template<std::same_as<Type> U>
            static constexpr auto get_index() noexcept
            {
                return I;
            }
        };

        template<typename... Types>
        struct type_set_builder : type_container<Types...>
        {
            using array_t = std::array<std::size_t, type_set_builder::size()>;

            static constexpr std::pair<array_t, std::size_t> filtered_result()
            {
                std::array indices{type_set_builder::template find<Types>()...};
                std::size_t valid_size = 1;

                for(std::size_t i = 1; i < indices.size(); ++i)
                    if(const auto index = indices[i]; index == i)
                    {
                        indices[valid_size] = index;
                        ++valid_size;
                    }

                return std::pair{indices, valid_size};
            }

            // clang-format off
            template<std::size_t... I>
            using selector = type_container<typename type_set_builder::
                template indexed_t<filtered_result().first[I]>...>;

            using filtered_t = typename traits::make_sequence<std::size_t{},
    filtered_result().second>:: template apply_t<selector>;
            // clang-format on
        };
    }

    template<typename... Types>
    struct type_set
    {
    private:
        using impl = typename details::type_set_builder<Types...>::filtered_t;

        template<typename... Other>
        struct append : type_container<Other...>
        {
            using array_t = std::array<std::size_t, append::size()>;

            static constexpr std::pair<array_t, std::size_t> filtered_result()
            {
                std::array contains{impl::template contains<Other...>};
                std::array<std::size_t, type_container<Other...>::size()> indices{};
                std::size_t valid_size = 0;

                for(std::size_t i = 0; i < contains.size(); ++i)
                    if(contains[i])
                    {
                        indices[valid_size] = i;
                        ++valid_size;
                    }

                return std::pair{indices, valid_size};
            }

            // clang-format off
            template<std::size_t... I>
            using selector = type_set<typename append::template
    indexed_t<filtered_result().first[I]>...>;

            using filtered_t = typename make_sequence<std::size_t{}, filtered_result().second>::
                template apply_t<selector>;
            // clang-format on
        };

    public:
        template<typename... Other>
        using append_t =
            std::conditional_t<impl::template contains<Other>, type_set, type_set<Types..., Other>>;
    };
*/
}
