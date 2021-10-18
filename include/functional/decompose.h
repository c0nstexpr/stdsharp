//
// Created by BlurringShadow on 2021-10-15.
//
#pragma once
#include "functional/cpo.h"
#include "type_traits/type_traits.h"

namespace stdsharp::functional
{
    template<::std::size_t I>
    struct decompose_by_fn
    {
        template<typename Decomposer, typename Parameter>
            requires requires { ::std::declval<Decomposer>().get<I>(::std::declval<Parameter>()); }
        constexpr decltype(auto) operator()(Decomposer&& decomposer, Parameter&& param) const
            noexcept(noexcept(::std::declval<Decomposer>().get<I>(::std::declval<Parameter>())))
        {
            return ::std::forward<Decomposer>(decomposer).get<I>(::std::forward<Parameter>(param));
        }
    };

    template<::std::size_t I>
    inline constexpr auto decompose_by =
        ::stdsharp::functional::tagged_cpo<::stdsharp::functional::decompose_by_fn<I>>;

    namespace details
    {
        template<
            typename Decomposer,
            typename,
            typename = ::std::make_index_sequence<Decomposer::size> // clang-format off
        > // clang-format on
        struct decompose_fn;

        template<typename Decomposer, typename Parameter, ::std::size_t... I>
        struct decompose_fn<Decomposer, Parameter, ::std::index_sequence<I...>>
        {
            Decomposer decomposer{};
            Parameter parameter{};

        private:
            template<
                typename This,
                typename AlignedDecomposer =
                    ::stdsharp::type_traits::const_ref_align_t<This&&, Decomposer>,
                typename AlignedParam =
                    ::stdsharp::type_traits::const_ref_align_t<This&&, Parameter>,
                ::std::invocable< //
                    ::std::invoke_result_t<
                        decltype(::stdsharp::functional::decompose_by<I>),
                        AlignedDecomposer,
                        AlignedParam // clang-format off
                    >...
                > Fn> // clang-format on
            static constexpr auto impl(This&& instance, Fn&& fn) noexcept( //
                ::stdsharp::concepts::nothrow_invocable<
                    Fn, //
                    ::std::invoke_result_t<
                        decltype(::stdsharp::functional::decompose_by<I>),
                        AlignedDecomposer,
                        AlignedParam // clang-format off
                    >...
                > // clang-format on
            )
            {
                return ::std::invoke(
                    ::std::forward<Fn>(fn),
                    ::stdsharp::functional::decompose_by<I>(
                        ::std::forward<This>(instance).decomposer,
                        ::std::forward<This>(instance).parameter // clang-format off
                    )... // clang-format on
                );
            }

            template<::stdsharp::concepts::decay_same_as<decompose_fn> This, typename Fn>
            friend auto operator|(This&& instance, Fn&& fn) //
                noexcept(::stdsharp::concepts::nothrow_invocable<This, Fn>)
            {
                return ::std::forward<This>(instance)(::std::forward<Fn>(fn));
            }

        public:
#define BS_DECOMPOSE_TO_OPERATOR(const_, ref)                                                   \
    template<typename Fn>                                                                       \
        requires requires                                                                       \
        {                                                                                       \
            decompose_fn::impl(::std::declval<const_ decompose_fn ref>(), std::declval<Fn>());  \
        }                                                                                       \
    [[nodiscard]] constexpr auto operator()(Fn&& fn) const_ ref noexcept(noexcept(              \
        decompose_fn::impl(::std::declval<const_ decompose_fn ref>(), ::std::forward<Fn>(fn)))) \
    {                                                                                           \
        return decompose_fn::impl(                                                              \
            static_cast<const_ decompose_fn ref>(*this), ::std::forward<Fn>(fn));               \
    }

            BS_DECOMPOSE_TO_OPERATOR(, &)
            BS_DECOMPOSE_TO_OPERATOR(const, &)
            BS_DECOMPOSE_TO_OPERATOR(, &&)
            BS_DECOMPOSE_TO_OPERATOR(const, &&)

#undef BS_DECOMPOSE_TO_OPERATOR
        };
    }

    template<typename Decomposer, typename Parameter, ::std::size_t... I>
    using decompose_fn = ::std::conditional_t<
        sizeof...(I) == 0,
        ::stdsharp::functional::details::decompose_fn<Decomposer, Parameter>,
        ::stdsharp::functional::details::
            decompose_fn<Decomposer, Parameter, ::std::index_sequence<I...>> // clang-format off
    >; // clang-format on

    template<::std::size_t... I>
    inline constexpr ::stdsharp::functional::invocable_obj make_decompose(
        ::stdsharp::functional::nodiscard_tag,
        []< //
            typename Decomposer,
            typename Parameter,
            ::std::constructible_from<Decomposer, Parameter> DecomposeFn =
                ::stdsharp::functional::decompose_fn<
                    ::std::decay_t<Decomposer>,
                    ::stdsharp::type_traits::coerce_t<Parameter>,
                    I... // clang-format off
                >
        > // clang-format on
        (Decomposer&& decomposer, Parameter&& parameter) noexcept( //
            ::stdsharp::concepts::nothrow_constructible_from<DecomposeFn, Decomposer, Parameter> //
        )
        {
            return DecomposeFn{
                ::std::forward<Decomposer>(decomposer),
                ::std::forward<Parameter>(parameter) //
            };
        } //
    );

    template<::std::size_t... I>
    inline constexpr ::stdsharp::functional::invocable_obj decompose_to(
        ::stdsharp::functional::nodiscard_tag,
        []<::std::copy_constructible Decomposer>(Decomposer&& decomposer) noexcept( //
            noexcept( //
                ::ranges::make_pipeable( //
                    ::std::bind_front(
                        ::stdsharp::functional::make_decompose<I...>,
                        ::std::forward<Decomposer>(decomposer) // clang-format off
                    )
                )
            ) // clang-format on
        )
        {
            return ::ranges::make_pipeable( //
                ::std::bind_front(
                    ::stdsharp::functional::make_decompose<I...>,
                    ::std::forward<Decomposer>(decomposer) // clang-format off
                ) // clang-format on
            );
        } //
    );
}
