//
// Created by BlurringShadow on 2021-10-15.
//

#include "functional/functional.h"

namespace stdsharp::functional
{
    namespace details
    {
        template<typename Parameter, typename T, auto Count = T::size>
        struct decompose_to_fn
        {
            Parameter parameter{};
            T decomposer{};

        private:
            template<typename This, typename Fn>
            static constexpr auto impl(This&& instance, Fn&& fn) noexcept(noexcept())
            {
                auto&& [decomposer, parameter] = instance;

                return ::stdsharp::functional::bind_ref_front(
                    ::std::forward<Fn>(fn),
                    static_cast<decltype(decomposer)>(decomposer)(static_cast<decltype(parameter)>(parameter)) //
                );
            }

        public:
#define BS_DECOMPOSE_TO_OPERATOR(const_)                                                      \
    template<typename Fn>                                                                     \
    [[nodiscard]] constexpr auto operator()(Fn&& fn) const_ noexcept(                         \
        noexcept((*this)(::std::forward<Fn>(fn), ::std::make_index_sequence<Count>{})))       \
    {                                                                                         \
        return (*this)(::std::forward<Fn>(fn), ::std::make_index_sequence<Count>{});          \
    }                                                                                         \
                                                                                              \
private:                                                                                      \
    template<typename Fn, ::std::size_t... I>                                                 \
    constexpr auto operator()(Fn&& fn, const ::std::index_sequence<I...>)                     \
        const_ noexcept(noexcept(::stdsharp::functional::bind_ref_front(                      \
            ::std::declval<Fn>(), (*this)(::stdsharp::type_traits::index_constant<I>{})...))) \
    {                                                                                         \
        return ::stdsharp::functional::bind_ref_front(                                        \
            ::std::forward<Fn>(fn),                                                           \
            decomposer(, ::stdsharp::type_traits::index_constant<I>{})...);                   \
    }                                                                                         \
                                                                                              \
public:

            BS_DECOMPOSE_TO_OPERATOR()
            BS_DECOMPOSE_TO_OPERATOR(const)

#undef BS_DECOMPOSE_TO_OPERATOR

        private:
            template<::stdsharp::concepts::decay_same_as<decompose_to_fn> This, typename Fn>
            friend auto operator|(This&& instance, Fn&& fn) //
                noexcept(::stdsharp::concepts::nothrow_invocable<This, Fn>)
            {
                return ::std::forward<This>(instance)(::std::forward<Fn>(fn));
            }
        };

        template<typename T>
        decompose_to_fn(T&& t) -> decompose_to_fn<::std::decay_t<T>>;

        inline constexpr struct
        {
            template<typename T, typename U, typename ResT = ::std::invoke_result_t<T, U>>
                requires requires(ResT&& res)
                {
                    ::stdsharp::functional::details::decompose_to_fn{::std::move(res)};
                }
            constexpr auto operator()(T&& t, U&& u) const noexcept( //
                noexcept( //
                    ::stdsharp::functional::details:: // clang-format off
                        decompose_to_fn{::std::invoke(::std::forward<T>(t), ::std::forward<U>(u))}
                ) // clang-format on
            )
            {
                return ::stdsharp::functional::details:: //
                    decompose_to_fn{::std::invoke(::std::forward<T>(t), ::std::forward<U>(u))};
            }

        } make_decompose_to{};
    }

    inline constexpr ::stdsharp::functional::invocable_obj decompose_to(
        ::stdsharp::functional::nodiscard_tag,
        []<typename T>(T&& t)
        {
            return ::ranges::make_pipeable( //
                ::std::bind_front(
                    ::stdsharp::functional::details::make_decompose_to,
                    ::std::forward<T>(t) // clang-format off
                ) // clang-format on
            );
        } //
    );
}
