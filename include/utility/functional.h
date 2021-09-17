#pragma once

#include <functional>

#include <range/v3/functional/overload.hpp>

#include "type_traits.h"

namespace stdsharp::utility
{
    template<typename Invocable>
    class invocable_obj
    {
        Invocable invocable_;

    public:
        using invocable_t = Invocable;

        template<typename... T>
            requires ::std::constructible_from<Invocable, T...> // clang-format off
        constexpr explicit invocable_obj(T&&... t) // clang-format on
            noexcept(::stdsharp::utility::nothrow_constructible_from<Invocable, T...>):
            invocable_(::std::forward<T>(t)...)
        {
        }

        template<typename... Args>
            requires ::std::invocable<const Invocable, Args...>
        constexpr decltype(auto) operator()(Args&&... args) const& //
            noexcept(nothrow_invocable<const Invocable, Args...>)
        {
            return ::std::invoke(invocable_, ::std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires ::std::invocable<Invocable&, Args...>
        constexpr decltype(auto) operator()(Args&&... args) & //
            noexcept(nothrow_invocable<Invocable&, Args...>)
        {
            return ::std::invoke(invocable_, ::std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires ::std::
                invocable<::std::add_rvalue_reference_t<::std::decay_t<Invocable>>, Args...>
        constexpr decltype(auto) operator()(Args&&... args) && //
            noexcept( //
                nothrow_invocable<
                    ::std::add_rvalue_reference_t<::std::decay_t<Invocable>>,
                    Args... // clang-format off
                > // clang-format on
            )
        {
            return ::std::invoke(::std::move(invocable_), ::std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires ::std::
                invocable<const ::std::add_rvalue_reference_t<::std::decay_t<Invocable>>, Args...>
        constexpr decltype(auto) operator()(Args&&... args) const&& //
            noexcept( //
                nothrow_invocable<
                    const ::std::add_rvalue_reference_t<::std::decay_t<Invocable>>,
                    Args... // clang-format off
                > // clang-format on
            )
        {
            return ::std::invoke(::std::move(invocable_), ::std::forward<Args>(args)...);
        }
    };

    template<typename Invocable>
    invocable_obj(Invocable&&) -> invocable_obj<::std::decay_t<Invocable>>;

    template<typename Invocable>
    class nodiscard_invocable_obj : ::stdsharp::utility::invocable_obj<Invocable>
    {
        using base = ::stdsharp::utility::invocable_obj<Invocable>;

    public:
        using base::base;

        using typename base::invocable_t;

        template<typename... Args>
            requires ::std::invocable<const Invocable, Args...>
        [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args) const& //
            noexcept(::stdsharp::utility::nothrow_invocable<const Invocable, Args...>)
        {
            return base::operator()(::std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires ::std::invocable<Invocable, Args...>
        [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args) & //
            noexcept(::stdsharp::utility::nothrow_invocable<Invocable, Args...>)
        {
            return base::operator()(::std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires ::std::
                invocable<::std::add_rvalue_reference_t<::std::decay_t<Invocable>>, Args...>
        [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args) const&& //
            noexcept( //
                nothrow_invocable<
                    ::std::add_rvalue_reference_t<::std::decay_t<Invocable>>,
                    Args... // clang-format off
                > // clang-format on
            )
        {
            return static_cast<const base&&>(*this)(::std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires ::std::
                invocable<const ::std::add_rvalue_reference_t<::std::decay_t<Invocable>>, Args...>
        [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args) && //
            noexcept( //
                nothrow_invocable<
                    const ::std::add_rvalue_reference_t<::std::decay_t<Invocable>>,
                    Args... // clang-format off
                > // clang-format on
            )
        {
            return static_cast<base&&>(*this)(::std::forward<Args>(args)...);
        }
    };

    template<typename Invocable>
    nodiscard_invocable_obj(Invocable&&) -> nodiscard_invocable_obj<::std::decay_t<Invocable>>;

    inline constexpr auto empty_invoke = [](auto&&...) noexcept
    {
        return ::stdsharp::utility::empty{}; //
    };

    namespace details
    {
        template<typename ReturnT>
        struct invoke_r_fn
        {
            template<typename Func, typename... Args>
                requires ::stdsharp::utility::invocable_r<ReturnT, Func, Args...>
            [[nodiscard]] constexpr auto operator()(Func&& func, Args&&... args) const
                noexcept(::stdsharp::utility::nothrow_invocable_r<ReturnT, Func, Args...>)
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
                noexcept(::stdsharp::utility::nothrow_constructible_from<T, Args...>)
            {
                return {::std::forward<Args>(args)...};
            };
        };
    }

    template<typename ReturnT>
    inline constexpr ::stdsharp::utility::details::invoke_r_fn<ReturnT> invoke_r{};

    template<typename T>
    inline constexpr ::stdsharp::utility::details::constructor_fn<T> constructor{};

    inline constexpr struct
    {
        template<typename T>
        [[nodiscard]] constexpr ::std::remove_cvref_t<T> operator()(T&& t) const
            noexcept(nothrow_constructible_from<::std::remove_cvref_t<T>, T>)
        {
            return forward<T>(t);
        }
    } copy{};

    inline constexpr struct bind_ref_front_fn
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
                ::stdsharp::utility::nothrow_invocable<Func, T..., Args...>;

        public:
            template<typename... Args>
                requires ::std::invocable<const Func, const T..., Args...>
            constexpr decltype(auto)
                operator()(Args&&... args) const& noexcept(invoker::noexcept_v<Args...>)
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
                requires ::std::invocable<Func, T..., Args...>
            constexpr decltype(auto) operator()(Args&&... args) & //
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

            template<typename... Args>
                requires ::std::invocable<const Func, const T..., Args...>
            constexpr decltype(auto)
                operator()(Args&&... args) const&& noexcept(invoker::noexcept_v<Args...>)
            {
                return ::std::apply( // clang-format off
                        [&]<typename... U>(U&&... t)
                            noexcept(invoker::noexcept_v<Args...>)
                            ->decltype(auto)
                        {
                            return ::std::invoke(
                                ::std::forward<U>(t)...,
                                ::std::forward<Args>(args)...
                            );
                        }, // clang-format on
                    static_cast<const base&&>(*this) //
                );
            }

            template<typename... Args>
                requires ::std::invocable<Func, T..., Args...>
            constexpr decltype(auto) operator()(Args&&... args) && //
                noexcept(invoker::noexcept_v<Args...>)
            {
                return ::std::apply( // clang-format off
                        [&]<typename... U>(U&&... t)
                            noexcept(invoker::noexcept_v<Args...>)
                            ->decltype(auto)
                        {
                            return ::std::invoke(
                                ::std::forward<U>(t)...,
                                ::std::forward<Args>(args)...
                            );
                        }, // clang-format on
                    static_cast<base&&>(*this) //
                );
            }
        };

    public:
        template<typename... T>
            requires ::std::constructible_from<Invocable, T...>
        constexpr explicit invocable_obj(const ::stdsharp::utility::nodiscard_tag_t, T&&... t) //
            noexcept(::stdsharp::utility::nothrow_constructible_from<Invocable, T...>):
            base(::std::forward<T>(t)...)
        {
        }

        using typename base::invocable_t;

        template<typename... Args>
            requires ::std::invocable<const Invocable, Args...>
        [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args) const& //
            noexcept(::stdsharp::utility::nothrow_invocable<const Invocable, Args...>)
        {
            return base::operator()(::std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires ::std::invocable<Invocable, Args...>
        [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args) & //
            noexcept(::stdsharp::utility::nothrow_invocable<Invocable, Args...>)
        {
            return base::operator()(::std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires ::std::
                invocable<::std::add_rvalue_reference_t<::std::decay_t<Invocable>>, Args...>
        [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args) const&& //
            noexcept( //
                nothrow_invocable<
                    ::std::add_rvalue_reference_t<::std::decay_t<Invocable>>,
                    Args... // clang-format off
                > // clang-format on
            )
        {
            return static_cast<const base&&>(*this)(::std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires ::std::
                invocable<const ::std::add_rvalue_reference_t<::std::decay_t<Invocable>>, Args...>
        [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args) && //
            noexcept( //
                nothrow_invocable<
                    const ::std::add_rvalue_reference_t<::std::decay_t<Invocable>>,
                    Args... // clang-format off
                > // clang-format on
            )
        {
            return static_cast<base&&>(*this)(::std::forward<Args>(args)...);
        }
    };

    template<typename Invocable>
    invocable_obj(Invocable&&) -> invocable_obj<::std::decay_t<Invocable>>;

    template<typename Invocable>
    invocable_obj(::stdsharp::utility::nodiscard_tag_t, Invocable&&)
        -> invocable_obj<::std::decay_t<Invocable>, ::stdsharp::utility::nodiscard_tag_t>;

    inline constexpr auto empty_invoke = [](auto&&...) noexcept
    {
        return ::stdsharp::utility::empty; //
    };

    template<typename ReturnT>
    inline constexpr ::stdsharp::utility::invocable_obj invoke_r(
        ::stdsharp::utility::nodiscard_tag,
        []<typename Func, typename... Args>(Func&& func, Args&&... args) // clang-format off
            noexcept(::stdsharp::utility::nothrow_invocable_r<ReturnT, Func, Args...>)
            -> ::std::enable_if_t<::stdsharp::utility::invocable_r<ReturnT, Func, Args...>, ReturnT>
        {
            return static_cast<ReturnT>(
                ::std::invoke(::std::forward<Func>(func), ::std::forward<Args>(args)...)
            );
        } // clang-format on
    );

    template<typename T>
    inline constexpr ::stdsharp::utility::invocable_obj constructor(
        ::stdsharp::utility::nodiscard_tag,
        []<typename... Args>(Args&&... args) // clang-format off
            noexcept(::stdsharp::utility::nothrow_constructible_from<T, Args...>)
            -> ::std::enable_if_t<::std::constructible_from<T, Args...>, T> // clang-format on
        {
            return T{::std::forward<Args>(args)...}; //
        } //
    );

    inline constexpr ::stdsharp::utility::invocable_obj copy(
        ::stdsharp::utility::nodiscard_tag,
        []<typename T>(T&& t) noexcept( // clang-format off
            ::stdsharp::utility::nothrow_constructible_from<::std::remove_cvref_t<T>, T>
        ) -> ::std::remove_cvref_t<T> // clang-format on
        {
            return forward<T>(t); //
        } //
    );

    inline constexpr auto bind_ref_front = []<typename Func, typename... Args>(
        Func && func,
        Args&&... args // clang-format off
    ) noexcept( // clang-format on
        noexcept( //
            ::stdsharp::utility::details::bind_ref_front_invoker{
                ::std::forward<Func>(func),
                ::std::forward<Args>(args)... //
            } // clang-format off
        ) // clang-format on
    )
    {
        return ::stdsharp::utility::details::bind_ref_front_invoker{
            ::std::forward<Func>(func),
            ::std::forward<Args>(args)... //
        }; //
    };

    inline constexpr auto assign_v = ::ranges::overload(
        []<typename T, typename U> // clang-format off
            requires ::std::assignable_from<T, U>
        (T& left, U&& right) noexcept(noexcept(left = ::std::forward<U>(right)))
            ->decltype(auto) // clang-format on
        {
            return left = ::std::forward<U>(right); //
        },
        []<typename T, typename... U> // clang-format off
            requires ::std::constructible_from<::std::remove_cvref_t<T>, U...>
        (T& left, U&&... right)
            noexcept(noexcept(left = ::std::remove_cvref_t<T>{::std::forward<U>(right)...}))
            ->decltype(auto) // clang-format on
        {
            return left = ::std::remove_cvref_t<T>{::std::forward<U>(right)...}; //
        } //
    );

    using assign = decltype(::stdsharp::utility::assign_v);

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

#define BS_UTIL_SHIFT_OPERATE(direction, operate)                                       \
    inline constexpr ::stdsharp::utility::nodiscard_invocable_obj direction##_shift_v{ \
        []<typename T, typename U>(T&& left, U&& right) noexcept(                       \
            noexcept(::std::forward<T>(left) operate ::std::forward<U>(right)))         \
        {                                                                               \
            return ::std::forward<T>(left) operate ::std::forward<U>(right); /**/       \
        }};                                                                             \
                                                                                        \
    using direction##_shift = decltype(::stdsharp::utility::direction##_shift_v);

    BS_UTIL_SHIFT_OPERATE(left, <<)
    BS_UTIL_SHIFT_OPERATE(right, >>)

#undef BS_UTIL_SHIFT_OPERATE

#define BS_UTIL_ASSIGN_OPERATE(operator_type, op)                                         \
    inline constexpr auto operator_type##_assign_v = ::ranges::overload(                  \
        []<typename U>(auto& l, U&& u) noexcept(                                          \
            noexcept((l op## = ::std::forward<U>(u)))) -> decltype(auto)                  \
        {                                                                                 \
            return l op## = ::std::forward<U>(u); /**/                                    \
        },                                                                                \
        []<typename U>(auto& l, U&& u) noexcept(                                          \
            noexcept((l = operator_type##_v(l, ::std::forward<U>(u))))) -> decltype(auto) \
        {                                                                                 \
            return l = operator_type##_v(l, ::std::forward<U>(u)); /**/                   \
        });                                                                               \
                                                                                          \
    using operator_type##_assign = decltype(operator_type##_assign_v);

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

#define BS_UTIL_INCREMENT_DECREMENT_OPERATE(operator_prefix, op, al_op)                           \
    inline constexpr auto pre_##operator_prefix##crease_v = ::ranges::overload(                   \
        []<typename T> requires ::std::invocable<al_op##_assign, T&, ::std::size_t>(              \
            T & v,                                                                                \
            const ::std::size_t i =                                                               \
                1) noexcept(::stdsharp::utility::                                                \
                                nothrow_invocable<al_op##_assign, T&, const ::std::size_t>) {     \
            return al_op##_assign_v(v, i); /**/                                                   \
        },                                                                                        \
        []<typename T>(T& v, ::std::size_t i = 1) noexcept(noexcept(op##op v))                    \
        {                                                                                         \
            for(; i > 0; --i) op##op v;                                                           \
            return v;                                                                             \
        });                                                                                       \
                                                                                                  \
    using pre_##operator_prefix##crease =                                                         \
        decltype(::stdsharp::utility::pre_##operator_prefix##crease_v);                          \
                                                                                                  \
    struct post_##operator_prefix##crease                                                         \
    {                                                                                             \
        template<typename T>                                                                      \
            requires ::std::invocable<::stdsharp::utility::al_op##_assign, T, ::std::size_t>     \
        [[nodiscard]] constexpr auto operator()(T& v, const ::std::size_t i = 1) const noexcept(  \
            ::stdsharp::utility::                                                                \
                nothrow_invocable<::stdsharp::utility::al_op##_assign, T&, const ::std::size_t>) \
        {                                                                                         \
            const auto old = v;                                                                   \
            ::stdsharp::utility::al_op##_assign_v(v, i);                                         \
            return old;                                                                           \
        }                                                                                         \
                                                                                                  \
        template<typename T>                                                                      \
        [[nodiscard]] constexpr auto operator()(T& v, ::std::size_t i = 1) const                  \
            noexcept(noexcept(v op##op) && noexcept(op##op v))                                    \
        {                                                                                         \
            if(i == 0) return v op##op;                                                           \
                                                                                                  \
            const auto old = v;                                                                   \
            for(; i > 0; --i) op##op v;                                                           \
            return old;                                                                           \
        }                                                                                         \
    };                                                                                            \
                                                                                                  \
    inline constexpr ::stdsharp::utility::post_##operator_prefix##crease                         \
        post_##operator_prefix##crease_v{};

    BS_UTIL_INCREMENT_DECREMENT_OPERATE(in, +, plus)
    BS_UTIL_INCREMENT_DECREMENT_OPERATE(de, -, minus)

#undef BS_UTIL_INCREMENT_DECREMENT_OPERATE

    inline constexpr ::stdsharp::utility::nodiscard_invocable_obj advance_v{
        ::ranges::overload(
            []<typename T, typename Distance> // clang-format off
                requires ::std::invocable<plus_assign, T, Distance>
            (T & v, Distance&& distance)
                noexcept(::stdsharp::utility::nothrow_invocable<plus_assign, T&, Distance>)
                ->decltype(auto) // clang-format on
            {
                return ::stdsharp::utility::plus_assign_v(
                    v,
                    ::std::forward<Distance>(distance) //
                );
            },
            []<typename T, typename Distance>(T& v, const Distance& distance) noexcept( //
                noexcept(
                    ::stdsharp::utility::pre_increase_v(v, distance),
                    ::stdsharp::utility::pre_decrease_v(v, distance) // clang-format off
                )
            ) -> decltype(auto) // clang-format on
            {
                return distance > 0 ? //
                    ::stdsharp::utility::pre_increase_v(v, distance) :
                    ::stdsharp::utility::pre_decrease_v(v, -distance);
            } // clang-format off
        ) // clang-format on
    };

    inline constexpr ::stdsharp::utility::invocable_obj returnable_invoke(
        nodiscard_tag,
        []<typename Func, typename... Args>(Func&& func, Args&&... args) //
        noexcept(::stdsharp::utility::nothrow_invocable<Func, Args...>)
            ->decltype(auto)
        {
            const auto invoker = [&]() //
                noexcept(::stdsharp::utility::nothrow_invocable<Func, Args...>)
            {
                return ::std::invoke(::std::forward<Func>(func), ::std::forward<Args>(args)...); //
            };
            if constexpr(::std::same_as<::std::invoke_result_t<decltype(invoker)>, void>)
            {
                invoker();
                return ::stdsharp::utility::empty;
            } // clang-format off
            else return invoker(); // clang-format on
        } //
    );

    inline constexpr auto merge_invoke = []<::std::invocable... Func>(Func&& ... func)
        noexcept(
            noexcept(
                ::std::tuple<
                    ::std::invoke_result_t<
                        decltype(::stdsharp::utility::returnable_invoke),
                        Func
                    >...
                >{
                    ::stdsharp::utility::returnable_invoke(::std::forward<Func>(func))...
                }
            )
        ) // clang-format on
    {
        return ::std::tuple< //
            ::std::invoke_result_t<
                decltype(::stdsharp::utility::returnable_invoke),
                Func // clang-format off
            >...
        >{
            // clang-format on
            ::stdsharp::utility::returnable_invoke(::std::forward<Func>(func))... //
        };
    };

    inline constexpr auto make_returnable = []<typename Func>(Func&& func) //
        noexcept( //
            noexcept( //
                ::std::bind_front(
                    ::stdsharp::utility::returnable_invoke,
                    ::std::forward<Func>(func) // clang-format off
                )
            ) // clang-format on
        )
    {
        return ::std::bind_front(
            ::stdsharp::utility::returnable_invoke,
            ::std::forward<Func>(func) //
        );
        } //
    );

    template<typename, typename...>
    struct cpo_invoke_fn;

    namespace details
    {
        template<typename Proj>
        class projector : invocable_obj<Proj>
        {
            using base = invocable_obj<Proj>;

        public:
            using base::base;

#define BS_UTILITY_PROJECTOR_OPERATOR_DEF(const_)                                              \
    template<typename Func, typename... Args>                                                  \
        requires(::std::invocable<const_ base, Args> && ...)                                   \
    &&::std::                                                                                  \
        invocable<Func, ::std::invoke_result_t<const_ base, Args>...> constexpr decltype(auto) \
            operator()(Func&& func, Args&&... args) const_ noexcept(                           \
                ::stdsharp::utility::                                                         \
                    nothrow_invocable<Func, ::std::invoke_result_t<const base, Args>...>)      \
    {                                                                                          \
        return ::std::invoke(                                                                  \
            ::std::forward<Func>(func), base::operator()(::std::forward<Args>(args))...);      \
    }

            BS_UTILITY_PROJECTOR_OPERATOR_DEF(const)
            BS_UTILITY_PROJECTOR_OPERATOR_DEF()
#undef BS_UTILITY_PROJECTOR_OPERATOR_DEF
        };

        template<typename CPOTag>
        struct cpo_fn
        {
            template<typename... T>
            constexpr decltype(auto) operator()(T&&... t) const noexcept
            {
                return ::stdsharp::utility::cpo_invoke_fn<CPOTag, ::std::remove_const_t<T>...>::
                    invoke(::std::forward<T>(t)... // clang-format off
                ); // clang-format on
            }
        };
    }

    // clang-format off
    inline constexpr ::stdsharp::utility::nodiscard_invocable_obj make_projector{
        []<typename Func>(Func&& func)
        {
            return ::stdsharp::utility::details::projector<Func>{::std::forward<Func>(func)};
        } // clang-format on
    };

    template<typename CPOTag>
    inline constexpr ::stdsharp::utility::details::cpo_fn<CPOTag> cpo{};
}
