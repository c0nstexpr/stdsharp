#pragma once

#include "core_traits.h"

namespace stdsharp
{
    template<typename T, ::std::size_t I>
    struct indexed_value : value_wrapper<T>
    {
        using value_wrapper<T>::value;

    private:
#define STDSHARP_GET(const_, ref_)                                                              \
    template<::std::size_t Index = I>                                                           \
        requires(I == Index)                                                                    \
    [[nodiscard]] friend constexpr decltype(auto) get(const_ indexed_value ref_ this_) noexcept \
    {                                                                                           \
        return static_cast<const_ T ref_>(this_.value);                                         \
    }

        STDSHARP_GET(, &)
        STDSHARP_GET(const, &)
        STDSHARP_GET(, &&)
        STDSHARP_GET(const, &&)

#undef STDSHARP_GET
    };

    namespace details
    {
        template<typename T, ::std::size_t I>
        struct indexed_type
        {
            struct t
            {
                using type = T;

            private:
                template<::std::size_t Index = I>
                    requires(I == Index)
                [[nodiscard]] friend constexpr stdsharp::type_constant<T> get(const t) noexcept
                {
                    return {};
                }
            };
        };
    }

    template<typename T, ::std::size_t I>
    using indexed_type = typename details::indexed_type<T, I>::t;

    namespace details
    {
        template<typename... T>
        struct indexed_values
        {
            template<::std::size_t... Index>
            struct inherited : stdsharp::indexed_value<T, Index>...
            {
            };

            template<typename = ::std::index_sequence_for<T...>>
            struct t;

            template<::std::size_t... Index>
            struct t<::std::index_sequence<Index...>> : inherited<Index...>
            {
                template<::std::size_t I>
                using type =
                    ::std::remove_cvref_t<decltype(get<I>(::std::declval<inherited<Index...>>()))>;
            };
        };

        template<typename... T>
        struct indexed_types
        {
            template<::std::size_t... Index>
            struct inherited : stdsharp::indexed_type<T, Index>...
            {
            };

            template<typename = ::std::index_sequence_for<T...>>
            struct t;

            template<::std::size_t... Index>
            struct t<::std::index_sequence<Index...>> : inherited<Index...>
            {
                template<::std::size_t I>
                using type = typename decltype(get<I>(inherited<Index...>{}))::type;
            };
        };
    }

    template<typename... T>
    struct indexed_values : details::indexed_values<T...>::template t<>
    {
    };

    template<typename... T>
    indexed_values(T&&...) -> indexed_values<::std::decay_t<T>...>;

    template<typename... T>
    using indexed_types = typename details::indexed_types<T...>::template t<>;
}