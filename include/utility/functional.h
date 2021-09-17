#pragma once

#include <functional>

#include <range/v3/functional/overload.hpp>

#include "type_traits.h"

namespace stdsharp::utility
{
    inline constexpr struct nodiscard_tag_t
    {
    } nodiscard_tag;

    namespace details
    {
        template<typename Invocable>
        struct invocable_obj_base : Invocable
        {
            using invocable_t = Invocable;
            using invocable_t::invocable_t;

            template<typename... T>
                requires ::std::constructible_from<Invocable, T...>
            constexpr explicit invocable_obj_base(T&&... t) //
                noexcept(::stdsharp::utility::nothrow_constructible_from<Invocable, T...>):
                invocable_t(::std::forward<T>(t)...)
            {
            }
        };

        template<::stdsharp::utility::final Invocable>
        class invocable_obj_base<Invocable>
        {
            Invocable invocable_;

        public:
            using invocable_t = Invocable;

            template<typename... T>
                requires ::std::constructible_from<Invocable, T...>
            constexpr explicit invocable_obj_base(T&&... t) //
                noexcept(::stdsharp::utility::nothrow_constructible_from<Invocable, T...>):
                invocable_(::std::forward<T>(t)...)
            {
            }

            template<typename... Args>
                requires ::std::invocable<const Invocable, Args...>
            constexpr decltype(auto) operator()(Args&&... args) const& //
                noexcept(::stdsharp::utility::nothrow_invocable<const Invocable, Args...>)
            {
                return ::std::invoke(invocable_, ::std::forward<Args>(args)...);
            }

            template<typename... Args>
                requires ::std::invocable<Invocable&, Args...>
            constexpr decltype(auto) operator()(Args&&... args) & //
                noexcept(::stdsharp::utility::nothrow_invocable<Invocable&, Args...>)
            {
                return ::std::invoke(invocable_, ::std::forward<Args>(args)...);
            }

            template<typename... Args>
                requires ::std::
                    invocable<::std::add_rvalue_reference_t<::std::decay_t<Invocable>>, Args...>
            constexpr decltype(auto) operator()(Args&&... args) && //
                noexcept( //
                    ::stdsharp::utility::nothrow_invocable<
                        ::std::add_rvalue_reference_t<::std::decay_t<Invocable>>,
                        Args... // clang-format off
                > // clang-format on
                )
            {
                return ::std::invoke(::std::move(invocable_), ::std::forward<Args>(args)...);
            }

            template<typename... Args>
                requires ::std::invocable<
                    const ::std::add_rvalue_reference_t<::std::decay_t<Invocable>>,
                    Args...>
            constexpr decltype(auto) operator()(Args&&... args) const&& //
                noexcept( //
                    ::stdsharp::utility::nothrow_invocable<
                        const ::std::add_rvalue_reference_t<::std::decay_t<Invocable>>,
                        Args... // clang-format off
                > // clang-format on
                )
            {
                return ::std::invoke(::std::move(invocable_), ::std::forward<Args>(args)...);
            }
        };

        template<typename Func, typename... T>
        class [[nodiscard]] bind_ref_front_invoker : ::std::tuple<Func, T...>
        {
            using base = ::std::tuple<Func, T...>;
            using base::base;

            template<typename... Args>
            static constexpr bool noexcept_v =
                ::stdsharp::utility::nothrow_invocable<Func, T..., Args...>;

        public:
            template<typename... Args>
                requires ::std::invocable<const Func, const T..., Args...>
            constexpr decltype(auto) operator()(Args&&... args) const& //
                noexcept(bind_ref_front_invoker::noexcept_v<Args...>)
            {
                return ::std::apply(
                    [&args...](auto&... t) noexcept(bind_ref_front_invoker::noexcept_v<Args...>) //
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
                noexcept(bind_ref_front_invoker::noexcept_v<Args...>)
            {
                return ::std::apply(
                    [&args...](auto&... t) noexcept(bind_ref_front_invoker::noexcept_v<Args...>) //
                    -> decltype(auto)
                    {
                        return ::std::invoke(t..., ::std::forward<Args>(args)...); //
                    },
                    static_cast<base&>(*this) //
                );
            }

            template<typename... Args>
                requires ::std::invocable<const Func, const T..., Args...>
            constexpr decltype(auto) operator()(Args&&... args) const&& //
                noexcept(bind_ref_front_invoker::noexcept_v<Args...>)
            {
                return ::std::apply( // clang-format off
                        [&]<typename... U>(U&&... t)
                            noexcept(bind_ref_front_invoker::noexcept_v<Args...>)
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
                noexcept(bind_ref_front_invoker::noexcept_v<Args...>)
            {
                return ::std::apply( // clang-format off
                        [&]<typename... U>(U&&... t)
                            noexcept(bind_ref_front_invoker::noexcept_v<Args...>)
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

        template<typename Func, typename... T>
        bind_ref_front_invoker(Func&&, T&&...) -> bind_ref_front_invoker<
            ::stdsharp::utility::coerce_t<Func>,
            ::stdsharp::utility::coerce_t<T>... // clang-format off
        >; // clang-format on
    }

    template<typename Invocable, typename = void>
    class invocable_obj : public ::stdsharp::utility::details::invocable_obj_base<Invocable>
    {
        using base = ::stdsharp::utility::details::invocable_obj_base<Invocable>;

    public:
        using base::base;
        using typename base::invocable_t;
    };

    template<typename Invocable>
    class invocable_obj<Invocable, ::stdsharp::utility::nodiscard_tag_t> :
        ::stdsharp::utility::details::invocable_obj_base<Invocable>
    {
        using base = ::stdsharp::utility::details::invocable_obj_base<Invocable>;

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

    inline constexpr struct assign
    {
        template<typename T, typename U>
            requires ::std::assignable_from<T, U>
        constexpr decltype(auto) operator()(T& left, U&& right) const
            noexcept(noexcept(left = ::std::forward<U>(right)))
        {
            return left = ::std::forward<U>(right);
        }

        template<typename T, typename... U>
            requires ::std::constructible_from<::std::remove_cvref_t<T>, U...>
        constexpr decltype(auto) operator()(T& left, U&&... right) const
            noexcept(noexcept(left = ::std::remove_cvref_t<T>{::std::forward<U>(right)...}))
        {
            return left = ::std::remove_cvref_t<T>{::std::forward<U>(right)...};
        }
    } assign_v{};

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

#define BS_UTIL_SHIFT_OPERATE(direction, operate)                                        \
    inline constexpr struct direction##_shift                                            \
    {                                                                                    \
        template<typename T, typename U>                                                 \
        [[nodiscard]] constexpr decltype(auto) operator()(T&& left, U&& right) const     \
            noexcept(noexcept(::std::forward<T>(left) operate ::std::forward<U>(right))) \
        {                                                                                \
            return ::std::forward<T>(left) operate ::std::forward<U>(right);             \
        }                                                                                \
    } direction##_shift_v{};

    BS_UTIL_SHIFT_OPERATE(left, <<)
    BS_UTIL_SHIFT_OPERATE(right, >>)

#undef BS_UTIL_SHIFT_OPERATE

#define BS_UTIL_ASSIGN_OPERATE(operator_type, op)                                \
    inline constexpr struct operator_type##_assign                               \
    {                                                                            \
        template<typename T, typename U>                                         \
            requires requires(T & l, U u) { l op## = u; }                        \
        constexpr decltype(auto) operator()(T& l, U&& u) const                   \
            noexcept(noexcept((l op## = ::std::forward<U>(u))))                  \
        {                                                                        \
            return l op## = ::std::forward<U>(u);                                \
        }                                                                        \
                                                                                 \
        template<typename U>                                                     \
        constexpr decltype(auto) operator()(auto& l, U&& u) const                \
            noexcept(noexcept((l = operator_type##_v(l, ::std::forward<U>(u))))) \
        {                                                                        \
            return l = operator_type##_v(l, ::std::forward<U>(u));               \
        }                                                                        \
    } operator_type##_assign_v{};

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

#define BS_UTIL_INCREMENT_DECREMENT_OPERATE(operator_prefix, op, al_op)                          \
    inline constexpr struct pre_##operator_prefix##crease                                        \
    {                                                                                            \
        template<typename T>                                                                     \
            requires ::std::invocable<al_op##_assign, T&, ::std::size_t>                         \
        constexpr decltype(auto) operator()(T& v, const ::std::size_t i = 1) const /**/          \
            noexcept(::stdsharp::utility::/**/                                                   \
                     nothrow_invocable<al_op##_assign, T&, const ::std::size_t>)                 \
        {                                                                                        \
            return al_op##_assign_v(v, i);                                                       \
        }                                                                                        \
        template<typename T>                                                                     \
        constexpr decltype(auto) operator()(T& v, ::std::size_t i = 1) const                     \
            noexcept(noexcept(op##op v))                                                         \
        {                                                                                        \
            for(; i > 0; --i) op##op v;                                                          \
            return v;                                                                            \
        }                                                                                        \
    } pre_##operator_prefix##crease_v{};                                                         \
                                                                                                 \
    inline constexpr struct post_##operator_prefix##crease                                       \
    {                                                                                            \
        template<typename T>                                                                     \
            requires ::std::invocable<::stdsharp::utility::al_op##_assign, T, ::std::size_t>     \
        [[nodiscard]] constexpr auto operator()(T& v, const ::std::size_t i = 1) const /**/      \
            noexcept(                                                                            \
                ::stdsharp::utility::/**/                                                        \
                nothrow_invocable<::stdsharp::utility::al_op##_assign, T&, const ::std::size_t>) \
        {                                                                                        \
            const auto old = v;                                                                  \
            ::stdsharp::utility::al_op##_assign_v(v, i);                                         \
            return old;                                                                          \
        }                                                                                        \
                                                                                                 \
        template<typename T>                                                                     \
        [[nodiscard]] constexpr auto operator()(T& v, ::std::size_t i = 1) const                 \
            noexcept(noexcept(v op##op) && noexcept(op##op v))                                   \
        {                                                                                        \
            if(i == 0) return v op##op;                                                          \
                                                                                                 \
            const auto old = v;                                                                  \
            for(; i > 0; --i) op##op v;                                                          \
            return old;                                                                          \
        }                                                                                        \
    } post_##operator_prefix##crease_v{};

    BS_UTIL_INCREMENT_DECREMENT_OPERATE(in, +, plus)
    BS_UTIL_INCREMENT_DECREMENT_OPERATE(de, -, minus)

#undef BS_UTIL_INCREMENT_DECREMENT_OPERATE

    inline constexpr struct advance
    {
        template<typename T, typename Distance>
            requires ::std::invocable<plus_assign, T, Distance>
        constexpr decltype(auto) operator()(T& v, Distance&& distance) const
            noexcept(::stdsharp::utility::nothrow_invocable<plus_assign, T&, Distance>)
        {
            return ::stdsharp::utility::plus_assign_v(
                v,
                ::std::forward<Distance>(distance) //
            );
        }

        template<typename T, typename Distance>
        constexpr decltype(auto) operator()(T& v, const Distance& distance) const //
            noexcept( //
                noexcept(
                    ::stdsharp::utility::pre_increase_v(v, distance),
                    ::stdsharp::utility::pre_decrease_v(v, distance) // clang-format off
                ) // clang-format on
            )
        {
            return distance > 0 ? //
                ::stdsharp::utility::pre_increase_v(v, distance) :
                ::stdsharp::utility::pre_decrease_v(v, -distance);
        }
    } advance_v{};

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

    inline constexpr ::stdsharp::utility::invocable_obj merge_invoke(
        ::stdsharp::utility::nodiscard_tag,
        []<::std::invocable... Func>(Func&&... func) noexcept( //
            noexcept( // clang-format off
                ::std::tuple<::std::invoke_result_t<
                    decltype(::stdsharp::utility::returnable_invoke),
                    Func>...
                 >{::stdsharp::utility::returnable_invoke(::std::forward<Func>(func))...}
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
        } //
    );

    inline constexpr ::stdsharp::utility::invocable_obj make_returnable(
        ::stdsharp::utility::nodiscard_tag,
        []<typename Func>(Func&& func) noexcept( //
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
        class [[nodiscard]] projector : invocable_obj<Proj>
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
                ::stdsharp::utility::                                                          \
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

    inline constexpr auto make_projector = []<typename Func>(Func&& func)
    {
        return ::stdsharp::utility::details::projector<Func>{::std::forward<Func>(func)}; //
    };

    template<typename CPOTag>
    inline constexpr ::stdsharp::utility::details::cpo_fn<CPOTag> cpo{};
}
