//

#pragma once

#include <algorithm>

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

#define STDSHARP_OPERATOR(const_, ref)                       \
    template<typename... Args, typename Fn = const_ T ref>   \
        requires ::std::invocable<Fn, Args...>               \
    constexpr decltype(auto) operator()(Args&&... args)      \
        const_ ref noexcept(nothrow_invocable<Fn, Args...>)  \
    {                                                        \
        return base::value()(::std::forward<Args>(args)...); \
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
                    indexed_invocable<Func, I>(::std::forward<Args>(args))...
                {
                }
            };
        };
    }

    template<typename... Func>
    using invocables = ::meta::_t<details::invocables<Func...>>;

    using make_invocables_fn = make_template_type_fn<invocables>;

    inline constexpr make_invocables_fn make_invocables{};

    template<typename T, typename... Args>
    inline constexpr auto invocables_test = []<::std::size_t... I>
        requires requires(get_element_t<I, T>... v) { (v, ...); } //
    (const ::std::index_sequence<I...>)
    {
        return ::std::array{invocable_test<get_element_t<I, T>, Args...>...};
    }
    (index_sequence_by<T>{});

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
            return cpo::get_element<Index>(::std::forward<T>(t))(::std ::forward<Args>(args)...);
        }
    };

    template<auto Index>
    inline constexpr invoke_at_fn<Index> invoke_at{};

    template<constant_value PredicateConstant>
    struct invoke_first_fn
    {
    private:
        template<typename T, typename... Args>
        struct invoke_at
        {
            static constexpr auto test_res = invocables_test<T, Args...>;
            using fn = invoke_at_fn<auto_cast(
                ::std::ranges::find_if(test_res, PredicateConstant::value) - test_res.cbegin()
            )>;
        };

    public:
        template<
            typename T,
            typename... Args,
            ::std::invocable<T, Args...> InvokeAt = typename invoke_at<T, Args...>::fn>
        constexpr decltype(auto) operator()(T&& t, Args&&... args) const
            noexcept(nothrow_invocable<InvokeAt, T, Args...>)
        {
            return InvokeAt{}(::std::forward<T>(t), ::std ::forward<Args>(args)...);
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
            ::std::forward<Args>(args)...                                         \
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