#pragma once // TODO: __cpp_pack_indexing >= 202311L

#include "../utility/value_wrapper.h"
#include "object.h"

#include <tuple>

#include "../compilation_config_in.h"

namespace stdsharp::details
{
    struct type_indexer
    {
        template<typename T>
        struct filter
        {
            consteval type_constant<T> get() const noexcept { return {}; }
        };

        struct invalid
        {
        };

        template<>
        struct filter<invalid>
        {
        };

    public:
        template<std::size_t I, std::size_t... J, typename... T>
        static consteval auto impl(
            const std::index_sequence<J...> /*unused*/,
            const basic_type_sequence<T...> /*unused*/
        )
        {
            constexpr struct : filter<std::conditional_t<I == J, T, invalid>>...
            {
            } f{};

            return f.get();
        }

        template<std::size_t I, typename... T>
        using type =
            decltype(impl<I>(std::index_sequence_for<T...>{}, basic_type_sequence<T...>{}))::type;
    };
}

namespace stdsharp
{
    template<std::size_t I, typename T>
    struct indexed_value : value_wrapper<T>
    {
    };

    template<std::size_t I, typename... T>
    using type_at = details::type_indexer::type<I, T...>;
}

namespace stdsharp::details
{
    template<typename...>
    struct indexed_values;

    template<typename... T, std::size_t... I>
    struct STDSHARP_EBO indexed_values<std::index_sequence<I...>, T...> : indexed_value<I, T>...
    {
        static constexpr auto size() noexcept { return sizeof...(T); }

        template<std::size_t J>
        using type = type_at<J, T...>;

        template<std::size_t J, typename Self, typename U = indexed_value<J, type<J>>>
        constexpr forward_cast_t<Self, type<J>> get(this Self&& self) noexcept
        {
            return forward_cast<Self, indexed_values, U>(self).get();
        }

        template<
            std::size_t J,
            typename Self,
            typename SelfT = const Self,
            typename U = indexed_value<J, type<J>>>
        constexpr forward_cast_t<SelfT, type<J>> cget(this const Self&& self) noexcept
        {
            return forward_cast<SelfT, indexed_values, U>(self).get();
        }

        template<
            std::size_t J,
            typename Self,
            typename SelfT = const Self&,
            typename U = indexed_value<J, type<J>>>
        constexpr forward_cast_t<SelfT, type<J>> cget(this const Self& self) noexcept
        {
            return forward_cast<SelfT, indexed_values, U>(self).get();
        }
    };
}

namespace stdsharp
{
    template<typename... T>
    struct indexed_values : details::indexed_values<std::make_index_sequence<sizeof...(T)>, T...>
    {
    private:
        using m_base = details::indexed_values<std::make_index_sequence<sizeof...(T)>, T...>;

    public:
        indexed_values() = default;

        template<typename... U>
            requires list_initializable_from<m_base, U...>
        constexpr indexed_values(U&&... u) noexcept(nothrow_list_initializable_from<m_base, U...>):
            m_base{cpp_forward(u)...}
        {
        }
    };

    template<typename... T>
    indexed_values(T&&...) -> indexed_values<std::decay_t<T>...>;
}

namespace std
{
    template<typename... T>
    struct tuple_size<::stdsharp::indexed_values<T...>>
    {
        static constexpr auto value = ::stdsharp::indexed_values<T...>::size();
    };

    template<std::size_t I, typename... T>
    struct tuple_element<I, ::stdsharp::indexed_values<T...>>
    {
        using type = ::stdsharp::indexed_values<T...>::template type<I>;
    };
}

#include "../compilation_config_out.h"
