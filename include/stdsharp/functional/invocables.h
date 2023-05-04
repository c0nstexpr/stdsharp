//

#pragma once

#include <algorithm>
#include <functional>

#include "../type_traits/indexed_traits.h"
#include "../utility/auto_cast.h"

namespace stdsharp
{
    namespace details
    {
        template<typename T, ::std::size_t I>
        struct indexed_invocable : stdsharp::indexed_value<T, I>
        {
            using base = stdsharp::indexed_value<T, I>;

            using base::base;

#define STDSHARP_OPERATOR(const_, ref)                                      \
    template<typename... Args, ::std::invocable<Args...> Fn = const_ T ref> \
    constexpr decltype(auto) operator()(Args&&... args)                     \
        const_ ref noexcept(nothrow_invocable<Fn, Args...>)                 \
    {                                                                       \
        return base::value()(cpp_forward(args)...);                         \
    }

            STDSHARP_OPERATOR(, &)
            STDSHARP_OPERATOR(const, &)
            STDSHARP_OPERATOR(, &&)
            STDSHARP_OPERATOR(const, &&)

#undef STDSHARP_OPERATOR
        };

        template<typename... Func>
        struct invocables
        {
            template<typename = ::std::index_sequence_for<Func...>>
            struct impl;

            using type = impl<>;

            template<::std::size_t... I>
            struct impl<::std::index_sequence<I...>> :
                indexed_invocable<Func, I>...,
                indexed_types<Func...>
            {
                impl() = default;

                template<typename... Args>
                    requires(::std::constructible_from<Func, Args> && ...)
                constexpr impl(Args&&... args) //
                    noexcept((nothrow_constructible_from<Func, Args> && ...)):
                    indexed_invocable<Func, I>(cpp_forward(args))...
                {
                }
            };
        };
    }

    template<typename... Func>
    using invocables = ::meta::_t<details::invocables<Func...>>;

    using make_invocables_fn = make_template_type_fn<invocables>;

    inline constexpr make_invocables_fn make_invocables{};

    template<::std::size_t Index>
    struct invoke_at_fn
    {
        template<
            typename T,
            typename... Args,
            ::std::invocable<Args...> Invocable = get_element_t<Index, T> // clang-format off
        > // clang-format on
        constexpr decltype(auto) operator()(T&& t, Args&&... args) const
            noexcept(nothrow_invocable<Invocable, Args...>)
        {
            return cpo::get_element<Index>(cpp_forward(args)...);
        }
    };

    template<auto Index>
    inline constexpr invoke_at_fn<Index> invoke_at{};

    template<constant_value PredicateConstant>
        requires ::std::predicate<decltype(PredicateConstant::value), expr_req>
    struct invoke_first_fn
    {
    private:
        template<typename T, typename... Args, ::std::size_t I = 0>
        static constexpr ::std::size_t find_first(const index_constant<I> = {}) noexcept
        {
            if constexpr(requires { typename get_element_t<I, T>; })
            {
                if constexpr(::std::
                                 invoke(PredicateConstant::value, invocable_test<get_element_t<I, T>, Args...>))
                    return I;
                else return find_first<T, Args...>(index_constant<I + 1>{});
            }
            else return -1;
        }

        template<typename T, typename... Args>
        static constexpr auto index = find_first<T, Args...>();

        template<typename T, typename... Args>
            requires(index<T, Args...> != -1)
        using invoke_at_t = invoke_at_fn<index<T, Args...>>;

    public:
        template<typename T, typename... Args>
            requires requires { typename invoke_at_t<T, Args...>; }
        constexpr decltype(auto) operator()(T&& t, Args&&... args) const
            noexcept(nothrow_invocable<invoke_at_t<T, Args...>, T, Args...>)
        {
            return invoke_at_t<T, Args...>{}(cpp_forward(args)...);
        }
    };

    template<constant_value PredicateConstant>
    inline constexpr invoke_first_fn<PredicateConstant> invoke_first{};

    namespace details
    {
        struct sequenced_invocables_predicate
        {
            static constexpr auto value =
                ::std::bind_front(::std::ranges::less_equal{}, expr_req::well_formed);
        };

        inline constexpr auto sequenced_invoke =
            invoke_first<details::sequenced_invocables_predicate>;
    }

    template<typename... Invocable>
    struct basic_sequenced_invocables : invocables<Invocable...>
    {
        using base = invocables<Invocable...>;

        using base::base;

#define STDSHARP_OPERATOR(const_, ref)                                            \
    template<                                                                     \
        typename... Args,                                                         \
        typename Base = const_ base ref,                                          \
        ::std::invocable<Base, Args...> Fn = decltype(details::sequenced_invoke)> \
    constexpr decltype(auto) operator()(Args&&... args)                           \
        const_ ref noexcept(nothrow_invocable<Fn, Base, Args...>)                 \
    {                                                                             \
        return details::sequenced_invoke(                                         \
            static_cast<const_ base ref>(*this),                                  \
            cpp_forward(args)...                                                  \
        );                                                                        \
    }

        STDSHARP_OPERATOR(, &)
        STDSHARP_OPERATOR(const, &)
        STDSHARP_OPERATOR(, &&)
        STDSHARP_OPERATOR(const, &&)

#undef STDSHARP_OPERATOR
    };

    template<typename... T>
    basic_sequenced_invocables(T&&...) -> basic_sequenced_invocables<::std::decay_t<T>...>;

    namespace details
    {
    }

    template<typename... Invocable>
    using sequenced_invocables = adl_proof_t<basic_sequenced_invocables, Invocable...>;

    inline constexpr make_template_type_fn<sequenced_invocables> make_sequenced_invocables{};
}