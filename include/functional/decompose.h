//
// Created by BlurringShadow on 2021-10-15.
//
#pragma once
#include "functional/cpo.h"
#include "functional/functional.h"
#include "type_traits/type_traits.h"
#include "utility/utility.h"

namespace stdsharp::functional
{
    template<::std::size_t I>
    struct decompose_by_fn
    {
    };

    template<::std::size_t I>
    inline constexpr tagged_cpo_t<decompose_by_fn<I>> decompose_by{};

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
                typename ForwardDecomposer = decltype(::std::declval<This>().decomposer),
                typename ForwardParam = decltype(::std::declval<This>().parameter),
                ::std::invocable< //
                    ::std::invoke_result_t<
                        decltype(decompose_by<I>),
                        ForwardDecomposer,
                        ForwardParam // clang-format off
                    >...
                > Fn
            > // clang-format on
            static constexpr auto impl(This&& instance, Fn&& fn) noexcept( //
                concepts::nothrow_invocable<
                    Fn, //
                    ::std::invoke_result_t<
                        decltype(decompose_by<I>),
                        ForwardDecomposer,
                        ForwardParam // clang-format off
                    >...
                > // clang-format on
            )
            {
                return ::std::invoke(
                    ::std::forward<Fn>(fn),
                    decompose_by<I>(
                        ::std::forward<This>(instance).decomposer,
                        ::std::forward<This>(instance).parameter // clang-format off
                    )... // clang-format on
                );
            }

        public:
#define BS_DECOMPOSE_TO_OPERATOR(const_, ref)                                                    \
    template<typename Fn>                                                                        \
        requires requires                                                                        \
        {                                                                                        \
            decompose_fn::impl(::std::declval<const_ decompose_fn ref>(), ::std::declval<Fn>()); \
        }                                                                                        \
    [[nodiscard]] constexpr auto operator()(Fn&& fn) const_ ref noexcept(noexcept(               \
        decompose_fn::impl(::std::declval<const_ decompose_fn ref>(), ::std::forward<Fn>(fn))))  \
    {                                                                                            \
        return decompose_fn::impl(                                                               \
            static_cast<const_ decompose_fn ref>(*this), ::std::forward<Fn>(fn));                \
    }

            BS_DECOMPOSE_TO_OPERATOR(, &)
            BS_DECOMPOSE_TO_OPERATOR(const, &)
            BS_DECOMPOSE_TO_OPERATOR(, &&)
            BS_DECOMPOSE_TO_OPERATOR(const, &&)

#undef BS_DECOMPOSE_TO_OPERATOR
        };
    }

    template<typename Decomposer, typename Parameter, ::std::size_t... I>
    struct decompose_fn :
        ::std::conditional_t<
            sizeof...(I) == 0,
            details::decompose_fn<Decomposer, Parameter>,
            details::
                decompose_fn<Decomposer, Parameter, ::std::index_sequence<I...>> // clang-format off
        > // clang-format on
    {
    private:
        template<concepts::decay_same_as<decompose_fn> This, typename Fn>
            requires ::std::invocable<This, Fn>
        friend auto operator|(This&& instance, Fn&& fn) //
            noexcept(concepts::nothrow_invocable<This, Fn>)
        {
            return ::std::forward<This>(instance)(::std::forward<Fn>(fn));
        }
    };

    template<typename Decomposer, typename Parameter>
    decompose_fn(Decomposer&&, Parameter&&)
        -> decompose_fn<::std::decay_t<Decomposer>, type_traits::coerce_t<Parameter>>;

    template<::std::size_t... I>
    inline constexpr invocable_obj make_decompose(
        nodiscard_tag,
        []<typename Decomposer, typename Parameter> // clang-format off
            requires requires
            {
                decompose_fn{
                    ::std::declval<Decomposer>(),
                    ::std::declval<Parameter>()
                };
            } // clang-format on
        (Decomposer&& decomposer, Parameter&& parameter) noexcept( //
            noexcept( //
                decompose_fn{
                    ::std::declval<Decomposer>(),
                    ::std::declval<Parameter>() //
                } // clang-format off
            ) // clang-format off
        )
        {
            return decompose_fn{
                ::std::forward<Decomposer>(decomposer),
                ::std::forward<Parameter>(parameter) //
            };
        } //
    );

    template<::std::size_t... I>
    inline constexpr invocable_obj decompose_to(
        nodiscard_tag,
        []<::std::copy_constructible Decomposer>(Decomposer&& decomposer) noexcept( //
            noexcept( //
                ::ranges::make_pipeable( //
                    bind_ref_front(
                        make_decompose<I...>,
                        ::std::forward<Decomposer>(decomposer) // clang-format off
                    )
                )
            ) // clang-format on
        )
        {
            return ::ranges::make_pipeable( //
                bind_ref_front(
                    make_decompose<I...>,
                    ::std::forward<Decomposer>(decomposer) // clang-format off
                ) // clang-format on
            );
        } //
    );
}
