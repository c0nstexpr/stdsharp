#pragma once

#include "core_traits.h"
#include "../utility/value_wrapper.h"
#include "../compilation_config_in.h"

namespace stdsharp
{
    template<typename T, std::size_t I>
    struct basic_indexed_type : type_constant<T>
    {
        static constexpr type_constant<T> get_type(const index_constant<I>) noexcept { return {}; }
    };

    namespace details
    {
        struct indexed_adl_accessor;

        template<typename T, std::size_t I>
        struct indexed_type
        {
            struct type : basic_indexed_type<T, I>
            {
                friend struct indexed_adl_accessor;

            private:
                using m_base = basic_indexed_type<T, I>;
            };
        };
    }

    template<typename T, std::size_t I>
    using indexed_type = ::meta::_t<details::indexed_type<T, I>>;

    template<std::size_t I>
    struct make_indexed_type_fn
    {
        template<typename T>
        [[nodiscard]] constexpr indexed_type<T, I> operator()(const std::type_identity<T>) //
            noexcept
        {
            return {};
        }
    };

    template<std::size_t I>
    inline constexpr make_indexed_type_fn<I> make_indexed_type{};

    template<typename T, std::size_t I>
    struct STDSHARP_EBO indexed_value : private value_wrapper<T>
    {
    private:
        using m_base = value_wrapper<T>;

    public:
        using m_base::m_base;
        using m_base::v;

#define STDSHARP_GET(const_, ref)                                                               \
    template<std::size_t Index>                                                                 \
    [[nodiscard]] constexpr decltype(auto) get(const index_constant<Index>) const_ ref noexcept \
    {                                                                                           \
        return static_cast<const_ T ref>(this->v);                                              \
    }

        STDSHARP_GET(, &)
        STDSHARP_GET(const, &)
        STDSHARP_GET(, &&)
        STDSHARP_GET(const, &&)

#undef STDSHARP_GET

        [[nodiscard]] constexpr operator indexed_type<T, I>() const noexcept { return {}; }
    };

    template<std::size_t I>
    struct make_indexed_value_fn
    {
        template<typename T>
            requires std::constructible_from<indexed_value<std::decay_t<T>, I>, T>
        [[nodiscard]] constexpr indexed_value<std::decay_t<T>, I> operator()(T&& t) //
            noexcept(nothrow_constructible_from<indexed_value<std::decay_t<T>, I>, T>)
        {
            return cpp_forward(t);
        }
    };

    template<std::size_t I>
    inline constexpr make_indexed_value_fn<I> make_indexed_value{};

    namespace details
    {
        template<typename... T>
        struct basic_indexed_types
        {
            template<typename = index_sequence_for<T...>>
            struct base;

            template<std::size_t... I>
            struct STDSHARP_EBO base<regular_value_sequence<I...>> :
                private stdsharp::indexed_type<T, I>...
            {
                using stdsharp::indexed_type<T, I>::get_type...;

                base() = default;

                template<typename... U>
                constexpr base(U&&... u) //
                    noexcept((nothrow_constructible_from<stdsharp::indexed_type<T, I>, U> && ...))
                    requires(std::constructible_from<stdsharp::indexed_type<T, I>, U> && ...)
                    : stdsharp::indexed_type<T, I>(cpp_forward(u))...
                {
                }
            };

            using impl = base<>;
        };
    }

    template<typename... T>
    struct basic_indexed_types : details::basic_indexed_types<T...>::impl
    {
    };

    template<typename... T>
    basic_indexed_types(std::type_identity<T>...) -> basic_indexed_types<T...>;

    template<typename... T>
    using indexed_types = adl_proof_t<basic_indexed_types, T...>;

    inline constexpr make_template_type_fn<indexed_types> make_indexed_types{};

    namespace details
    {
        template<typename... T>
        struct indexed_values
        {
            template<typename = index_sequence_for<T...>>
            struct impl;

            using indexed_types = stdsharp::indexed_types<T...>;

            template<std::size_t... I>
            struct STDSHARP_EBO impl<regular_value_sequence<I...>> :
                private stdsharp::indexed_value<T, I>...
            {
                using stdsharp::indexed_value<T, I>::get...;

                impl() = default;

                template<typename... U>
                    requires(std::constructible_from<stdsharp::indexed_value<T, I>, U> && ...)
                constexpr impl(U&&... u) //
                    noexcept((nothrow_constructible_from<stdsharp::indexed_value<T, I>, U> && ...)):
                    stdsharp::indexed_value<T, I>(cpp_forward(u))...
                {
                }

                [[nodiscard]] explicit constexpr operator indexed_types() const noexcept
                {
                    return {};
                }

#define STDSHARP_GET(const_, ref)                                                       \
    template<std::size_t Index>                                                         \
    [[nodiscard]] constexpr decltype(auto) get() const_ ref noexcept                    \
    {                                                                                   \
        using indexed =                                                                 \
            stdsharp::indexed_value<std::tuple_element_t<Index, indexed_types>, Index>; \
        return static_cast<const_ indexed ref>(*this).get(index_constant<Index>{});     \
    }

                STDSHARP_GET(, &)
                STDSHARP_GET(const, &)
                STDSHARP_GET(, &&)
                STDSHARP_GET(const, &&)

#undef STDSHARP_GET

                bool operator==(const impl&) const = default;
            };
        };
    }

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

namespace stdsharp::details
{
    struct indexed_adl_accessor
    {
        template<typename T, typename U, std::size_t I>
        static consteval auto invoke(const basic_indexed_type<U, I>)
        {
            return std::same_as<T, indexed_type<U, I>>;
        }

        template<typename T>
        static constexpr auto is_indexed_type =
            requires { requires invoke<T>(typename T::m_base{}); };
    };
}

namespace std
{
    template<typename... T>
    struct tuple_size<::stdsharp::basic_indexed_types<T...>> :
        tuple_size<::stdsharp::regular_type_sequence<T...>>
    {
    };

    template<::stdsharp::adl_proofed_for<::stdsharp::indexed_types> T>
    struct tuple_size<T> : tuple_size<typename T::basic_indexed_types>
    {
    };

    template<typename... T>
    struct tuple_size<::stdsharp::indexed_values<T...>> :
        tuple_size<::stdsharp::indexed_types<T...>>
    {
    };

    template<std::size_t I, typename T>
    struct tuple_element<I, ::stdsharp::basic_indexed_type<T, I>>
    {
        using type = T;
    };

    template<std::size_t I, typename T>
        requires ::stdsharp::details::indexed_adl_accessor::is_indexed_type<T>
    struct tuple_element<I, T> : tuple_element<I, typename T::basic_indexed_type>
    {
    };

    template<std::size_t I, typename T>
    struct tuple_element<I, ::stdsharp::indexed_value<T, I>>
    {
        using type = T;
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

    template<std::size_t I, ::stdsharp::adl_proofed_for<::stdsharp::indexed_types> T>
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