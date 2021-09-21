//
// Created by BlurringShadow on 2021-9-17.
//

#pragma once

#include "concepts/concepts.h"

namespace stdsharp::functional
{
    inline constexpr struct nodiscard_tag_t
    {
    } nodiscard_tag;

    namespace details
    {
        template<typename Invocable>
        class invocable_obj_base
        {
            Invocable invocable_;

        public:
            using invocable_t = Invocable;

            template<typename... T>
                requires ::std::constructible_from<Invocable, T...>
            constexpr explicit invocable_obj_base(T&&... t) //
                noexcept(::stdsharp::concepts::nothrow_constructible_from<Invocable, T...>):
                invocable_(::std::forward<T>(t)...)
            {
            }

            template<typename... Args>
                requires ::std::invocable<const Invocable, Args...>
            constexpr decltype(auto) operator()(Args&&... args) const& //
                noexcept(::stdsharp::concepts::nothrow_invocable<const Invocable, Args...>)
            {
                return ::std::invoke(invocable_, ::std::forward<Args>(args)...);
            }

            template<typename... Args>
                requires ::std::invocable<Invocable&, Args...>
            constexpr decltype(auto) operator()(Args&&... args) & //
                noexcept(::stdsharp::concepts::nothrow_invocable<Invocable&, Args...>)
            {
                return ::std::invoke(invocable_, ::std::forward<Args>(args)...);
            }

            template<typename... Args>
                requires ::std::
                    invocable<::std::add_rvalue_reference_t<::std::decay_t<Invocable>>, Args...>
            constexpr decltype(auto) operator()(Args&&... args) && //
                noexcept( //
                    ::stdsharp::concepts::nothrow_invocable<
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
                    ::stdsharp::concepts::nothrow_invocable<
                        const ::std::add_rvalue_reference_t<::std::decay_t<Invocable>>,
                        Args... // clang-format off
                > // clang-format on
                )
            {
                return ::std::invoke(::std::move(invocable_), ::std::forward<Args>(args)...);
            }
        };

        template<::stdsharp::concepts::class_ Invocable>
            requires(!::stdsharp::concepts::final<Invocable>)
        struct invocable_obj_base<Invocable> : Invocable
        {
            using invocable_t = Invocable;
            using invocable_t::invocable_t;

            template<typename... T>
                requires ::std::constructible_from<Invocable, T...>
            constexpr explicit invocable_obj_base(T&&... t) //
                noexcept(::stdsharp::concepts::nothrow_constructible_from<Invocable, T...>):
                invocable_t(::std::forward<T>(t)...)
            {
            }
        };
    }

    template<typename Invocable, typename = void>
    class invocable_obj : public ::stdsharp::functional::details::invocable_obj_base<Invocable>
    {
        using base = ::stdsharp::functional::details::invocable_obj_base<Invocable>;

    public:
        using base::base;
        using typename base::invocable_t;
    };

    template<typename Invocable>
    class invocable_obj<Invocable, ::stdsharp::functional::nodiscard_tag_t> :
        ::stdsharp::functional::details::invocable_obj_base<Invocable>
    {
        using base = ::stdsharp::functional::details::invocable_obj_base<Invocable>;

    public:
        template<typename... T>
            requires ::std::constructible_from<Invocable, T...>
        constexpr explicit invocable_obj(const ::stdsharp::functional::nodiscard_tag_t, T&&... t) //
            noexcept(::stdsharp::concepts::nothrow_constructible_from<Invocable, T...>):
            base(::std::forward<T>(t)...)
        {
        }

        using typename base::invocable_t;

        template<typename... Args>
            requires ::std::invocable<const Invocable, Args...>
        [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args) const& //
            noexcept(::stdsharp::concepts::nothrow_invocable<const Invocable, Args...>)
        {
            return base::operator()(::std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires ::std::invocable<Invocable, Args...>
        [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args) & //
            noexcept(::stdsharp::concepts::nothrow_invocable<Invocable, Args...>)
        {
            return base::operator()(::std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires ::std::
                invocable<::std::add_rvalue_reference_t<::std::decay_t<Invocable>>, Args...>
        [[nodiscard]] constexpr decltype(auto) operator()(Args&&... args) const&& //
            noexcept( //
                ::stdsharp::concepts::nothrow_invocable<
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
                ::stdsharp::concepts::nothrow_invocable<
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
    invocable_obj(::stdsharp::functional::nodiscard_tag_t, Invocable&&)
        -> invocable_obj<::std::decay_t<Invocable>, ::stdsharp::functional::nodiscard_tag_t>;

    inline constexpr ::stdsharp::functional::invocable_obj make_invocable_ref(
        ::stdsharp::functional::nodiscard_tag,
        ::ranges::overload(
            []<typename Invocable>(Invocable& invocable) noexcept
            {
                return ::stdsharp::functional::invocable_obj<Invocable&>{invocable}; //
            },
            []<typename Invocable>(nodiscard_tag_t, Invocable& invocable) noexcept
            {
                return ::stdsharp::functional::invocable_obj<Invocable&>{
                    ::stdsharp::functional::nodiscard_tag,
                    invocable //
                };
            } // clang-format off
        ) // clang-format on
    );
}
