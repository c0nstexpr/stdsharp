#pragma once

#include <functional>

#include <range/v3/functional/overload.hpp>

#include "type_traits.h"

namespace blurringshadow::utility
{
    inline constexpr auto invoke = []<typename... Args>(Args && ... args) // clang-format off
        noexcept(noexcept(::std::invoke(std::forward<Args>(args)...))) -> decltype(auto)
    { // clang-format on
        return ::std::invoke(std::forward<Args>(args)...); //
    };

    inline constexpr auto empty_invoke = [](auto&&...) noexcept
    {
        return empty{}; //
    };

    namespace details
    {
        template<typename ReturnT>
        struct invoke_r_fn
        {
            template<typename Func, typename... Args>
                requires ::blurringshadow::utility::invocable_r<ReturnT, Func, Args...>
            [[nodiscard]] constexpr ReturnT operator()(Func&& func, Args&&... args) const
                noexcept(::blurringshadow::utility::nothrow_invocable_r<ReturnT, Func, Args...>)
            {
                return static_cast<ReturnT>(
                    ::std::invoke(::std::forward<Func>(func), ::std::forward<Args>(args)...) //
                );
            }
        };

        template<typename T>
        struct constructor_fn
        {
            template<typename... Args>
                requires ::std::constructible_from<T, Args...>
            [[nodiscard]] constexpr T operator()(Args&&... args) const
                noexcept(::blurringshadow::utility::nothrow_constructible_from<T, Args...>)
            {
                return {::std::forward<Args>(args)...};
            };
        };
    }
    template<typename T>
    inline constexpr ::blurringshadow::utility::details::constructor_fn<T> constructor{};

    template<typename ReturnT>
    inline constexpr ::blurringshadow::utility::details::invoke_r_fn<ReturnT> invoke_r{};

    template<typename Invocable>
    class invocable_obj
    {
    protected:
        Invocable fn_{};

    public:
        template<typename... T>
            requires ::std::constructible_from<Invocable, T...> // clang-format off
        constexpr explicit invocable_obj(T&&... t) // clang-format on
            noexcept(::blurringshadow::utility::nothrow_constructible_from<Invocable, T...>):
            fn_(::std::forward<T>(t)...)
        {
        }

        template<typename... Args>
            requires ::std::invocable<const Invocable, Args...>
        constexpr decltype(auto) operator()(Args&&... args) const
            noexcept(::blurringshadow::utility::nothrow_invocable<const Invocable, Args...>)
        {
            return ::std::invoke(fn_, ::std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires ::std::invocable<Invocable, Args...>
        constexpr decltype(auto) operator()(Args&&... args) //
            noexcept(::blurringshadow::utility::nothrow_invocable<Invocable, Args...>)
        {
            return ::std::invoke(fn_, ::std::forward<Args>(args)...);
        }
    };

    namespace details
    {
        struct bind_ref_front_fn
        {
        private:
            template<typename Func, typename... T>
            class invoker : ::std::tuple<Func, T...>
            {
                friend bind_ref_front_fn;

                using base = ::std::tuple<Func, T...>;
                using base::base;

                template<typename... Args>
                static constexpr bool noexcept_v =
                    ::blurringshadow::utility::nothrow_invocable<Func, T..., Args...>;

            public:
                template<typename... Args>
                constexpr decltype(auto) operator()(Args&&... args) const
                    noexcept(invoker::noexcept_v<Args...>)
                {
                    return ::std::apply(
                        [&args...](auto&... t) noexcept(invoker::noexcept_v<Args...>) //
                        -> decltype(auto)
                        {
                            return ::std::invoke(t..., ::std::forward<Args>(args)...); //
                        },
                        static_cast<const base&>(*this) //
                    );
                }

                template<typename... Args>
                constexpr decltype(auto) operator()(Args&&... args) //
                    noexcept(invoker::noexcept_v<Args...>)
                {
                    return ::std::apply(
                        [&args...](auto&... t) noexcept(invoker::noexcept_v<Args...>) //
                        -> decltype(auto)
                        {
                            return ::std::invoke(t..., ::std::forward<Args>(args)...); //
                        },
                        static_cast<base&>(*this) //
                    );
                }
            };

        public:
            template<typename Func, typename... Args>
            [[nodiscard]] constexpr auto operator()(Func&& func, Args&&... args) const
                noexcept( // clang-format on
                    ::blurringshadow::utility::nothrow_constructible_from<
                        invoker<
                            ::blurringshadow::utility::to_lvalue_t<Func>,
                            ::blurringshadow::utility::to_lvalue_t<Args>... // clang-format off
                        >,
                        Func,
                        Args...
                    >
                ) // clang-format on
            {
                return invoker<
                    ::blurringshadow::utility::to_lvalue_t<Func>,
                    ::blurringshadow::utility::to_lvalue_t<Args>... // clang-format off
                >{::std::forward<Func>(func), ::std::forward<Args>(args)...}; // clang-format on
            }
        };
    }

    inline constexpr ::blurringshadow::utility::details::bind_ref_front_fn bind_ref_front{};

    struct assign
    {
        template<typename T, typename U>
            requires ::std::assignable_from<T, U>
        constexpr decltype(auto) operator()(T& left, U&& right) const
            noexcept(noexcept(left = ::std::forward<U>(right)))
        {
            return left = ::std::forward<U>(right);
        }

        template<typename T, typename... U>
        constexpr decltype(auto) operator()(T& left, U&&... right) const
            noexcept(noexcept(left = ::std::remove_cvref_t<T>{::std::forward<U>(right)...}))
        {
            return left = ::std::remove_cvref_t<T>{::std::forward<U>(right)...};
        }
    };

    inline constexpr ::blurringshadow::utility::assign assign_v{};

    inline constexpr ::std::ranges::equal_to equal_to_v{};
    inline constexpr ::std::ranges::not_equal_to not_equal_to_v{};
    inline constexpr ::std::ranges::less less_v{};
    inline constexpr ::std::ranges::greater greater_v{};
    inline constexpr ::std::ranges::less_equal less_equal_v{};
    inline constexpr ::std::ranges::greater_equal greater_equal_v{};
    inline constexpr ::std::compare_three_way compare_three_way_v{};

    inline constexpr ::std::plus<> plus_v{};
    inline constexpr ::std::minus<> minus_v{};
    inline constexpr ::std::divides<> divides_v{};
    inline constexpr ::std::multiplies<> multiplies_v{};
    inline constexpr ::std::modulus<> modulus_v{};
    inline constexpr ::std::negate<> negate_v{};

    inline constexpr ::std::logical_and<> logical_and_v{};
    inline constexpr ::std::logical_not<> logical_not_v{};
    inline constexpr ::std::logical_or<> logical_or_v{};

    inline constexpr ::std::bit_and<> bit_and_v{};
    inline constexpr ::std::bit_not<> bit_not_v{};
    inline constexpr ::std::bit_or<> bit_or_v{};
    inline constexpr ::std::bit_xor<> bit_xor_v{};

    struct left_shift
    {
        template<typename T, typename U>
        [[nodiscard]] constexpr auto operator()(T&& left, U&& right) const
            noexcept(noexcept(::std::forward<T>(left) << ::std::forward<U>(right)))
        {
            return ::std::forward<T>(left) << ::std::forward<U>(right);
        }
    };

    struct right_shift
    {
        template<typename T, typename U>
        [[nodiscard]] constexpr auto operator()(T&& left, U&& right) const
            noexcept(noexcept(::std::forward<T>(left) >> ::std::forward<U>(right)))
        {
            return ::std::forward<T>(left) >> ::std::forward<U>(right);
        }
    };

    inline constexpr ::blurringshadow::utility::left_shift left_shift_v{};

    inline constexpr ::blurringshadow::utility::right_shift right_shift_v{};

#define BS_UTIL_ASSIGN_OPERATE(operator_type, op)                                              \
    struct operator_type##_assign                                                              \
    {                                                                                          \
    private:                                                                                   \
        static constexpr auto operator_assign_fn = ::ranges::overload(                         \
            []<typename U>(auto& l, U&& u) noexcept(noexcept((l op## = ::std::forward<U>(u)))) \
            { return l op## = ::std::forward<U>(u); },                                         \
            []<typename U>(auto& l, U&& u) noexcept(                                           \
                noexcept((l = operator_type##_v(l, ::std::forward<U>(u)))))                    \
            { return l = operator_type##_v(l, ::std::forward<U>(u)); });                       \
                                                                                               \
    public:                                                                                    \
        template<typename T, typename U>                                                       \
        [[nodiscard]] constexpr auto operator()(T& left, U&& right) const noexcept(            \
            noexcept(operator_type##_assign::operator_assign_fn(left, forward<U>(right))))     \
        {                                                                                      \
            return operator_type##_assign::operator_assign_fn(left, forward<U>(right));        \
        }                                                                                      \
    };                                                                                         \
                                                                                               \
    inline constexpr ::blurringshadow::utility::operator_type##_assign operator_type##_assign_v;

    BS_UTIL_ASSIGN_OPERATE(plus, +)
    BS_UTIL_ASSIGN_OPERATE(minus, -)
    BS_UTIL_ASSIGN_OPERATE(divides, /)
    BS_UTIL_ASSIGN_OPERATE(multiplies, *)
    BS_UTIL_ASSIGN_OPERATE(modulus, %)
    BS_UTIL_ASSIGN_OPERATE(bit_and, &)
    BS_UTIL_ASSIGN_OPERATE(bit_or, |)
    BS_UTIL_ASSIGN_OPERATE(left_shift, <<)
    BS_UTIL_ASSIGN_OPERATE(right_shift, >>)

#undef BS_UTIL_ASSIGN_OPERATE

    inline constexpr ::std::identity identity_v{};

#define BS_UTIL_INCREMENT_DECREMENT_OPERATE(operator_prefix, op, al_op)                            \
    struct pre_##operator_prefix##crease                                                           \
    {                                                                                              \
        template<typename T>                                                                       \
            requires ::std::invocable<al_op##_assign, T&, ::std::size_t>                           \
        [[nodiscard]] constexpr auto operator()(T& v, const ::std::size_t i = 1) const noexcept(   \
            ::blurringshadow::utility::nothrow_invocable<al_op##_assign, T&, const ::std::size_t>) \
        {                                                                                          \
            return al_op##_assign_v(v, i);                                                         \
        }                                                                                          \
                                                                                                   \
        template<typename T>                                                                       \
        [[nodiscard]] constexpr auto operator()(T& v, ::std::size_t i = 1) const                   \
            noexcept(noexcept(op##op v))                                                           \
        {                                                                                          \
            for(; i > 0; --i) op##op v;                                                            \
            return v;                                                                              \
        }                                                                                          \
    };                                                                                             \
                                                                                                   \
    inline constexpr ::blurringshadow::utility::pre_##operator_prefix##crease                      \
        pre_##operator_prefix##crease_v{};                                                         \
                                                                                                   \
    struct post_##operator_prefix##crease                                                          \
    {                                                                                              \
        template<typename T>                                                                       \
            requires ::std::invocable<::blurringshadow::utility::al_op##_assign, T, ::std::size_t> \
        [[nodiscard]] constexpr auto operator()(T& v, const ::std::size_t i = 1) const             \
            noexcept(::blurringshadow::utility::nothrow_invocable<                                 \
                     ::blurringshadow::utility::al_op##_assign,                                    \
                     T&,                                                                           \
                     const ::std::size_t>)                                                         \
        {                                                                                          \
            const auto old = v;                                                                    \
            ::blurringshadow::utility::al_op##_assign_v(v, i);                                     \
            return old;                                                                            \
        }                                                                                          \
        template<typename T>                                                                       \
        [[nodiscard]] constexpr auto operator()(T& v, ::std::size_t i = 1) const                   \
            noexcept(noexcept(v op##op) && noexcept(op##op v))                                     \
        {                                                                                          \
            if(i == 0) return v op##op;                                                            \
                                                                                                   \
            const auto old = v;                                                                    \
            for(; i > 0; --i) op##op v;                                                            \
            return old;                                                                            \
        }                                                                                          \
    };                                                                                             \
                                                                                                   \
    inline constexpr ::blurringshadow::utility::post_##operator_prefix##crease                     \
        post_##operator_prefix##crease_v{};

    BS_UTIL_INCREMENT_DECREMENT_OPERATE(in, +, plus)
    BS_UTIL_INCREMENT_DECREMENT_OPERATE(de, -, minus)

#undef BS_UTIL_INCREMENT_DECREMENT_OPERATE
#undef BS_DUPLICATE

    struct advance
    {
        template<typename T, typename Distance>
            requires ::std::invocable<plus_assign, T, Distance>
        [[nodiscard]] constexpr auto operator()(T& v, Distance&& distance) const
            noexcept(::blurringshadow::utility::nothrow_invocable<plus_assign, T&, Distance>)
        {
            return ::blurringshadow::utility::plus_assign_v(v, ::std::forward<Distance>(distance));
        }

        template<typename T, typename Distance>
        [[nodiscard]] constexpr auto operator()(T& v, const Distance& distance) const noexcept( //
            noexcept(
                ::blurringshadow::utility::pre_increase_v(v, distance),
                ::blurringshadow::utility::pre_decrease_v(v, distance) // clang-format off
            )
        ) // clang-format on
        {
            if(distance > 0) return ::blurringshadow::utility::pre_increase_v(v, distance);
            return ::blurringshadow::utility::pre_decrease_v(v, -distance);
        }
    };

    inline constexpr ::blurringshadow::utility::advance advance_v{}; // clang-format off

    inline constexpr auto returnable_invoke = []<typename Func, typename... Args>(
        Func&& func,
        Args&&... args 
    ) noexcept(::blurringshadow::utility::nothrow_invocable<Func, Args...>) // clang-format on
    {
        const auto invoker =
            [&]() noexcept(::blurringshadow::utility::nothrow_invocable<Func, Args...>)
        {
            return ::std::invoke(func, ::std::forward<Args>(args)...); //
        };
        if constexpr(::std::same_as<::std::invoke_result_t<decltype(invoker)>, void>)
        {
            invoker();
            return empty{};
        } // clang-format off
        else return invoker();  
    };

    inline constexpr auto merge_invoke = []<::std::invocable... Func>(Func&& ... func)
        noexcept(
            noexcept(
                ::std::tuple{
                    ::blurringshadow::utility::returnable_invoke(::std::forward<Func>(func))...
                }
            )
        ) // clang-format on
    {
        return ::std::tuple{
            ::blurringshadow::utility::returnable_invoke(::std::forward<Func>(func))... //
        };
    };

    namespace details
    {
        struct make_returnable_fn
        {
            template<typename Func>
            constexpr auto operator()(Func&& func) const
            {
                return ::std::bind_front(::blurringshadow::utility::returnable_invoke, func);
            };
        };

        struct clone_fn
        {
            template<typename T>
            [[nodiscard]] constexpr ::std::remove_cvref_t<T> operator()(T&& t) const
                noexcept(nothrow_constructible_from<::std::remove_cvref_t<T>, T>)
            {
                return forward<T>(t);
            }
        };
    }

    inline constexpr ::blurringshadow::utility::details::make_returnable_fn make_returnable{};

    inline constexpr ::blurringshadow::utility::details::clone_fn clone{};
}
