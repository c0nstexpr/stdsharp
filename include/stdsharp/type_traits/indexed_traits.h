#pragma once

#include "core_traits.h"
#include "../utility/value_wrapper.h"
#include "../compilation_config_in.h"

namespace stdsharp::details
{
    template<typename... T>
    struct indexed_types
    {
    private:
        template<typename U, std::size_t I>
        struct indexed_type : type_constant<U>
        {
            static constexpr type_constant<U> get_type(const index_constant<I>) noexcept
            {
                return {};
            }
        };

    public:
        template<typename = index_sequence_for<T...>>
        struct base;

        template<std::size_t... I>
        struct STDSHARP_EBO base<regular_value_sequence<I...>> : indexed_type<T, I>...
        {
            using indexed_type<T, I>::get_type...;

            base() = default;
        };

        using impl = base<>;
    };
}

namespace stdsharp
{
    template<typename... T>
    struct basic_indexed_types : details::indexed_types<T...>::impl
    {
    };

    template<typename... T>
    basic_indexed_types(std::type_identity<T>...) -> basic_indexed_types<T...>;

    template<typename... T>
    using indexed_types = adl_proof_t<basic_indexed_types, T...>;

    inline constexpr make_template_type_fn<indexed_types> make_indexed_types{};
}

namespace stdsharp::details
{
    template<typename... T>
    struct indexed_values
    {
    private:
        template<typename U, std::size_t I>
        struct indexed_value : value_wrapper<U>
        {
        private:
            using m_base = value_wrapper<U>;

        public:
            using m_base::m_base;
        };

    public:
        template<typename = index_sequence_for<T...>>
        struct impl;

        using indexed_types = stdsharp::indexed_types<T...>;

        template<std::size_t... I>
        struct STDSHARP_EBO impl<regular_value_sequence<I...>> :
            indexed_value<T, I>...,
            indexed_types
        {
            impl() = default;

            template<typename... U>
                requires(std::constructible_from<indexed_value<T, I>, U> && ...)
            constexpr impl(U&&... u) //
                noexcept((nothrow_constructible_from<indexed_value<T, I>, U> && ...)):
                indexed_value<T, I>(cpp_forward(u))...
            {
            }

        private:
            template<std::size_t Index>
            using indexed_value_type = std::tuple_element_t<Index, indexed_types>;

        public:
#define STDSHARP_GET(const_, ref)                                                          \
    template<std::size_t Index>                                                            \
    [[nodiscard]] constexpr const_ indexed_value_type<Index> ref get() const_ ref noexcept \
    {                                                                                      \
        using indexed = indexed_value<indexed_value_type<Index>, Index>;                   \
        return static_cast<const_ indexed ref>(*this).v;                                   \
    }

            STDSHARP_GET(, &)
            STDSHARP_GET(const, &)
            STDSHARP_GET(, &&)
            STDSHARP_GET(const, &&)

#undef STDSHARP_GET
        };
    };
}

namespace stdsharp
{

    template<typename... T>
    struct indexed_values : details::indexed_values<T...>::template impl<>
    {
        using details::indexed_values<T...>::template impl<>::impl;
    };

    template<typename... T>
    indexed_values(T&&...) -> indexed_values<std::decay_t<T>...>;

    template<typename Indexed>
    using index_sequence_by = std::make_index_sequence<std::tuple_size_v<std::decay_t<Indexed>>>;

    inline constexpr struct indexed_apply_fn
    {
    private:
        template<
            std::size_t... I,
            typename Indexed,
            std::invocable<get_element_t<I, Indexed>...> Fn>
        static constexpr decltype(auto) impl(
            Fn&& fn,
            Indexed&& indexed,
            const std::index_sequence<I...> //
        ) noexcept(nothrow_invocable<Fn, get_element_t<I, Indexed>...>)
        {
            return std::invoke(cpp_forward(fn), cpo::get_element<I>(cpp_forward(indexed))...);
        }

    public:
        template<typename Indexed, typename Fn>
            requires requires {
                impl(std::declval<Fn>(), std::declval<Indexed>(), index_sequence_by<Indexed>{});
            }
        constexpr decltype(auto) operator()(Fn && fn, Indexed && indexed) const noexcept( //
            noexcept( //
                impl(std::declval<Fn>(), std::declval<Indexed>(), index_sequence_by<Indexed>{})
            )
        )
        {
            return impl(cpp_forward(fn), cpp_forward(indexed), index_sequence_by<Indexed>{});
        }
    } indexed_apply{};
}

namespace std
{
    template<typename... T>
    struct tuple_size<::stdsharp::basic_indexed_types<T...>> :
        tuple_size<::stdsharp::regular_type_sequence<T...>>
    {
    };

    template<::stdsharp::adl_proofed_for<::stdsharp::basic_indexed_types> T>
    struct tuple_size<T> : tuple_size<typename T::basic_indexed_types>
    {
    };

    template<typename... T>
    struct tuple_size<::stdsharp::indexed_values<T...>> :
        tuple_size<::stdsharp::indexed_types<T...>>
    {
    };

    template<std::size_t I, typename... T>
    struct tuple_element<I, ::stdsharp::basic_indexed_types<T...>>
    {
        using type = ::meta::_t< //
            decltype( //
                ::stdsharp::basic_indexed_types<T...>{}.get_type(::stdsharp::index_constant<I>{}) //
            ) // clang-format off
        >; // clang-format on
    };

    template<std::size_t I, ::stdsharp::adl_proofed_for<::stdsharp::basic_indexed_types> T>
    struct tuple_element<I, T> : tuple_element<I, typename T::basic_indexed_types>
    {
    };

    template<std::size_t I, typename... T>
    struct tuple_element<I, ::stdsharp::indexed_values<T...>> :
        tuple_element<I, ::stdsharp::indexed_types<T...>>
    {
    };
}

#include "../compilation_config_out.h"
