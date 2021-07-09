// Created by BlurringShadow at 2021-06-11-ÏÂÎç 6:52

#pragma once

#include "sequence.h"

namespace blurringshadow::utility::traits
{
    template<auto... Values>
    struct sequence_set;

    namespace details
    {
        template<auto... Values, auto... Others>
        struct sequence_set_insert<sequence_set<Values...>, Others...>
        {
            using array_t = std::array<std::size_t, sizeof...(Others)>;

            static constexpr std::pair<array_t, std::size_t> filtered_indices()
            {
                array_t indices{};

                std::size_t valid_size = 0;

                (
                    [&, i = std::size_t{}]<auto V>(constant<V>) mutable
                    {
                        if(!sequence_set<Values...>::template contains<V>())
                            if constexpr(constexpr auto matches{equal_to_v(V, Others)...};
                                         std::ranges::find(matches, true) - matches.cbegin() ==
                                         valid_size)
                                indices[valid_size++] = i;
                        ++i;
                    }(constant<Others>{}),
                    ...);

                return {indices, valid_size};
            }

            // clang-format off
            template<std::size_t... I>
            using filtered_seq = sequence_set<Values..., sequence<Others...>::
                template get_by_index<filtered_indices().first[I]>()...>;

            using type = traits::make_sequence<std::size_t{}, filtered_indices().second>::
                template apply_t<filtered_seq>;
            // clang-format on
        };

        template<typename, auto...>
        struct sequence_set_exclude;

        template<auto... Values, auto... Others>
        struct sequence_set_exclude<sequence_set<Values...>, Others...>
        {
            using array_t = std::array<std::size_t, sizeof...(Values)>;

            static constexpr std::pair<array_t, std::size_t> filtered_indices()
            {
                array_t indices{};
                std::size_t valid_size = 0;
                std::array matches{{Values, false}...};

                (
                    [&, i = std::size_t{}]<auto V>(constant<V>) mutable
                    {
                        if(!sequence_set<Values...>::template contains<V>())
                            indices[valid_size++] = i;
                        ++i;
                    }(constant<Others>{}),
                    ...);

                return {indices, valid_size};
            }

            // clang-format off
            template<std::size_t... I>
            using filtered_seq = sequence_set<
                sequence<Values...>::template get_by_index<filtered_indices().first[I]>()...
            >;

            using type = traits::make_sequence<std::size_t{}, filtered_indices().second>::
                template apply_t<filtered_seq>;
            // clang-format on
        };
    }

    template<auto... Values>
    struct sequence_set : private details::keyed_value<Values>...
    {
        using as_container = sequence<Values...>;

    private:
        using details::keyed_value<Values>::get_value...;

    public:
        template<auto Value>
        static constexpr auto contains()
        {
            return details::seq_set_contains<sequence_set, Value>;
        }

        template<auto... Others>
        using insert_t = typename details::sequence_set_insert<sequence_set, Others...>::type;

        template<typename Seq>
        using insert_seq_t = typename Seq::template apply_t<insert_t>;

        template<auto... Others>
        using exclude_t = typename details::sequence_set_exclude<sequence_set, Others...>::type;

        template<typename Seq>
        using exclude_seq_t = typename Seq::template apply_t<exclude_t>;
    };
}
