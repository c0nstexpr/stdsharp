#pragma once

#include "../utility/value_wrapper.h"

#include "../compilation_config_in.h"

namespace stdsharp
{
    template<std::size_t I, typename T>
    struct indexed_value : value_wrapper<T>
    {
    };
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
        using base_at = indexed_value<J, T...[J]>;

        template<std::size_t J>
            requires(J < size())
        constexpr decltype(auto) get(this auto&& self) noexcept
        {
            return cpp_forward(self).base_at<J>::get();
        }

        template<std::size_t J>
            requires(J < size())
        constexpr decltype(auto) cget(this const auto&& self) noexcept
        {
            return cpp_forward(self).template get<J>();
        }

        template<std::size_t J>
            requires(J < size())
        constexpr decltype(auto) cget(this const auto& self) noexcept
        {
            return cpp_forward(self).template get<J>();
        }
    };
}

namespace stdsharp
{
    template<typename... T>
    class indexed_values : details::indexed_values<std::make_index_sequence<sizeof...(T)>, T...>
    {
        using m_base = details::indexed_values<std::make_index_sequence<sizeof...(T)>, T...>;

    public:
        using m_base::size;
        using m_base::get;
        using m_base::cget;

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
        using type = typename ::stdsharp::indexed_values<T...>::template type<I>;
    };
}

#include "../compilation_config_out.h"
