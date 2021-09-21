// Created by BlurringShadow at 2021-03-11-下午 4:30

#pragma once

#include "type_traits/object.h"
#include "setter.h"
#include "getter.h"

namespace stdsharp::utility::property
{
    template<typename GetterFn, typename SetterFn, typename GetterT, typename SetterT>
    class property_member : ::stdsharp::type_traits::unique_object
    {
    public:
        using getter_t = GetterT;
        using setter_t = SetterT;

    private:
        getter_t& getter_;
        setter_t& setter_;

    public:
        struct func_obj
        { // clang-format off
            static constexpr auto set = []<typename... Args>(property_member& p, Args&&... args)
                noexcept(::stdsharp::concepts::nothrow_invocable<setter_t, Args...>) // clang-format on
                -> decltype(auto)
            {
                return p.setter_(::std::forward<Args>(args)...);
            };

            static constexpr ::stdsharp::functional::invocable_obj get{
                ::ranges::overload(
                    [](const property_member& p) // clang-format off
                        noexcept(::stdsharp::concepts::nothrow_invocable<getter_t>)
                        -> decltype(auto) // clang-format on
                    {
                        return p.getter_(); //
                    },
                    [](property_member&& p) // clang-format off
                        noexcept(::stdsharp::concepts::nothrow_invocable<getter_t>)
                        -> decltype(auto) // clang-format on
                    {
                        return ::std::move(p.getter_()); //
                    } // clang-format off
                ) // clang-format on
            };
        };

        template<typename... Args>
        constexpr decltype(auto) set(Args&&... args) //
            noexcept(::stdsharp::concepts::nothrow_invocable<setter_t, Args...>)
        {
            return func_obj::set(*this, ::std::forward<Args>(args)...);
        };

        constexpr decltype(auto) get() const& //
            noexcept(::stdsharp::concepts::nothrow_invocable<getter_t>)
        {
            return func_obj::get(*this);
        };

        constexpr decltype(auto) get() && //
            noexcept(::stdsharp::concepts::nothrow_invocable<getter_t>)
        {
            return func_obj::get(::std::move(*this));
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
