// Created by BlurringShadow at 2021-03-11-下午 4:30

#pragma once

#include "type_traits/object.h"
#include "setter.h"
#include "getter.h"

namespace stdsharp::utility::property
{
    template<::std::invocable GetterFn, typename SetterFn, typename GetterT, typename SetterT>
    class property_member
    {
    public:
        using getter_t = GetterT;
        using setter_t = SetterT;

    private:
        getter_t& getter_;
        setter_t& setter_;

    public:
        static constexpr ::std::type_identity<setter_t> set_tag{};
        static constexpr ::std::type_identity<getter_t> get_tag{};

        constexpr auto operator()(const decltype(set_tag)) noexcept
        {
            return ::stdsharp::functional::make_invocable_ref(setter_);
        }

        constexpr auto operator()(const decltype(get_tag)) const noexcept
        {
            return ::stdsharp::functional::make_invocable_ref(
                ::stdsharp::functional::nodiscard_tag,
                getter_ //
            );
        }

        template<typename... Args>
            requires ::std::invocable<setter_t, Args...>
        constexpr decltype(auto) set(Args&&... args) //
            noexcept(::stdsharp::concepts::nothrow_invocable<setter_t, Args...>)
        {
            return setter_(::std::forward<Args>(args)...);
        };

        constexpr decltype(auto) get() const //
            noexcept(::stdsharp::concepts::nothrow_invocable<getter_t>)
        {
            return getter_(); //
        };

        template<typename T>
        constexpr auto& operator=(T&& t) //
            noexcept(::stdsharp::concepts::nothrow_assignable_from<setter_t, T>)
        {
            setter_ = std::forward<T>(t);
            return *this;
        }

        property_member(getter_t& g, setter_t& s) noexcept: getter_(g), setter_(s) {}
    };

    template<typename GetterT, typename SetterT> // clang-format off
    property_member(GetterT&, SetterT&) ->
        property_member<
            typename ::std::remove_cvref_t<GetterT>::invocable_t,
            typename ::std::remove_cvref_t<SetterT>::invocable_t,
            GetterT,
            SetterT
        >; // clang-format on
}
