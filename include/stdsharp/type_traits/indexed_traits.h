#pragma once

#include "core_traits.h"
#include "../utility/value_wrapper.h"
#include "../compilation_config_in.h"

namespace stdsharp
{
    namespace details
    {
        struct stdsharp_indexed_tag
        {
        };
    }

    template<typename T, ::std::size_t I>
    struct basic_indexed_type : type_constant<T>
    {
        static constexpr type_constant<T> get_type(const index_constant<I>) noexcept { return {}; }

        static constexpr basic_indexed_type get_indexed_type(const index_constant<I>) noexcept
        {
            return {};
        }
    };

    template<::std::size_t Index, typename T>
    [[nodiscard]] constexpr auto get_type(const basic_indexed_type<T, Index>& v) //
        noexcept
    {
        return v.get_type(index_constant<Index>{});
    }

    namespace details
    {
        template<typename T, ::std::size_t I>
        struct indexed_type
        {
            struct type : basic_indexed_type<T, I>
            {
            };
        };
    }

    template<typename T, ::std::size_t I>
    using indexed_type = ::meta::_t<details::indexed_type<T, I>>;

    template<::std::size_t I>
    struct make_indexed_type_fn
    {
        template<typename T>
        [[nodiscard]] constexpr indexed_type<T, I> operator()(const ::std::type_identity<T>) //
            noexcept
        {
            return {};
        }
    };

    template<::std::size_t I>
    inline constexpr make_indexed_type_fn<I> make_indexed_type{};

    template<typename T, ::std::size_t I>
    struct STDSHARP_EBO basic_indexed_value : value_wrapper<T>, indexed_type<T, I>
    {
        using value_wrapper<T>::value_wrapper;

#define STDSHARP_GET(const_, ref_)                                                               \
    template<::std::size_t Index>                                                                \
    [[nodiscard]] constexpr decltype(auto) get(const index_constant<Index>) const_ ref_ noexcept \
    {                                                                                            \
        return value_wrapper<T>::get();                                                          \
    }

        STDSHARP_GET(, &)
        STDSHARP_GET(const, &)
        STDSHARP_GET(, &&)
        STDSHARP_GET(const, &&)

#undef STDSHARP_GET
    };

#define STDSHARP_GET(const_, ref_)                                                         \
    template<::std::size_t Index, typename T>                                              \
    [[nodiscard]] constexpr decltype(auto) get(const_ basic_indexed_value<T, Index> ref_ v \
    ) noexcept                                                                             \
    {                                                                                      \
        return v.get(index_constant<Index>{});                                             \
    }

    STDSHARP_GET(, &)
    STDSHARP_GET(const, &)
    STDSHARP_GET(, &&)
    STDSHARP_GET(const, &&)

#undef STDSHARP_GET

    namespace details
    {
        template<typename T, ::std::size_t I>
        struct indexed_value
        {
            struct type : basic_indexed_value<T, I>
            {
                using basic_indexed_value<T, I>::basic_indexed_value;
            };
        };
    }

    template<typename T, ::std::size_t I>
    using indexed_value = ::meta::_t<details::indexed_value<T, I>>;

    template<::std::size_t I>
    struct make_indexed_value_fn
    {
        template<typename T>
            requires ::std::constructible_from<indexed_value<::std::decay_t<T>, I>, T>
        [[nodiscard]] constexpr indexed_value<::std::decay_t<T>, I> operator()(T&& t) //
            noexcept(nothrow_constructible_from<indexed_value<::std::decay_t<T>, I>, T>)
        {
            return cpp_forward(t);
        }
    };

    template<::std::size_t I>
    inline constexpr make_indexed_value_fn<I> make_indexed_value{};

    namespace details
    {
        template<typename... T>
        struct basic_indexed_types
        {
            template<typename = index_sequence_for<T...>>
            struct base;

            template<::std::size_t... I>
            struct STDSHARP_EBO base<regular_value_sequence<I...>> :
                stdsharp::indexed_type<T, I>...,
                regular_type_sequence<T...>
            {
                base() = default;

                template<typename... U>
                constexpr base(U&&... u) //
                    noexcept((nothrow_constructible_from<stdsharp::indexed_type<T, I>, U> && ...))
                    requires(::std::constructible_from<stdsharp::indexed_type<T, I>, U> && ...)
                    : stdsharp::indexed_type<T, I>(cpp_forward(u))...
                {
                }

                static constexpr auto size() noexcept { return sizeof...(T); }
            };

            struct impl : base<>
            {
                template<::std::size_t J>
                using get_t = ::meta::_t<decltype(get_type<J>(base{}))>;
            };
        };
    }

    template<typename... T>
    struct basic_indexed_types :
        details::basic_indexed_types<T...>::impl,
        details::stdsharp_indexed_tag
    {
    };

    template<typename... T>
    basic_indexed_types(::std::type_identity<T>...) -> basic_indexed_types<T...>;

    template<typename... T>
    using indexed_types = adl_proof_t<basic_indexed_types, T...>;

    template<typename... T>
        requires requires //
    {
        requires(
            ::std::convertible_to<
                template_rebind<::std::decay_t<T>, void>,
                ::std::type_identity<void>> &&
            ...
        );
    }
    struct deduction<indexed_types, T...>
    {
        using type = indexed_types<::meta::_t<T>...>;
    };

    inline constexpr make_template_type_fn<indexed_types> make_indexed_types{};

    namespace details
    {
        template<typename... T>
        struct indexed_values
        {
            template<typename = index_sequence_for<T...>>
            struct impl;

            template<::std::size_t... I>
            struct STDSHARP_EBO impl<regular_value_sequence<I...>> :
                stdsharp::indexed_value<T, I>...,
                stdsharp::indexed_types<T...>
            {
                impl() = default;

                template<typename... U>
                    requires(::std::constructible_from<stdsharp::indexed_value<T, I>, U> && ...)
                constexpr impl(U&&... u) //
                    noexcept((nothrow_constructible_from<stdsharp::indexed_value<T, I>, U> && ...)):
                    stdsharp::indexed_value<T, I>(cpp_forward(u))...
                {
                }
            };
        };
    }

    template<typename... T>
    struct basic_indexed_values : details::indexed_values<T...>::template impl<>
    {
        using details::indexed_values<T...>::template impl<>::impl;
    };

    template<typename... T>
    basic_indexed_values(T&&...) -> basic_indexed_values<::std::decay_t<T>...>;

    template<typename... T>
    using indexed_values = adl_proof_t<basic_indexed_values, T...>;

    inline constexpr make_template_type_fn<indexed_values> make_indexed_values{};

    template<typename Indexed>
    using index_sequence_by =
        ::std::make_index_sequence<::std::tuple_size_v<::std::decay_t<Indexed>>>;

    inline constexpr struct indexed_apply_fn
    {
    private:
        template<
            ::std::size_t... I,
            typename Indexed,
            ::std::invocable<get_element_t<I, Indexed>...> Fn>
        static constexpr decltype(auto) impl(
            Fn&& fn,
            Indexed&& indexed,
            const ::std::index_sequence<I...> //
        ) noexcept(nothrow_invocable<Fn, get_element_t<I, Indexed>...>)
        {
            return ::std::invoke(cpp_forward(fn), cpo::get_element<I>(cpp_forward(indexed))...);
        }

    public:
        template<typename Indexed, typename Fn>
            requires requires //
        { impl(::std::declval<Fn>(), ::std::declval<Indexed>(), index_sequence_by<Indexed>{}); }
        constexpr decltype(auto) operator()(Fn&& fn, Indexed&& indexed) const noexcept( //
            noexcept( //
                impl(::std::declval<Fn>(), ::std::declval<Indexed>(), index_sequence_by<Indexed>{})
            )
        )
        {
            return impl(
                cpp_forward(fn),
                cpp_forward(indexed),
                ::std::make_index_sequence<::std::tuple_size_v<Indexed>>{}
            );
        }
    } indexed_apply{};
}

namespace std
{
    template<::std::size_t I, derived_from<::stdsharp::details::stdsharp_indexed_tag> Indexed>
    struct tuple_element<I, Indexed>
    {
        using type = Indexed::template get_t<I>;
    };

    template<derived_from<::stdsharp::details::stdsharp_indexed_tag> Indexed>
    struct tuple_size<Indexed>
    {
        static constexpr auto value = Indexed::size();
    };
}

#include "../compilation_config_out.h"