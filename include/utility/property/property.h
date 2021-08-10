// Created by BlurringShadow at 2021-03-11-下午 4:30

#pragma once

#include "utility/traits/object.h"
#include "setter.h"
#include "getter.h"

namespace blurringshadow::utility::property
{
    template<typename GetterFn, typename SetterFn, typename GetterT, typename SetterT>
    class property_member : ::blurringshadow::utility::traits::fixed_object
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
            static constexpr auto set = []<typename... Args>(property_member & p, Args&&... args)
                noexcept(::blurringshadow::utility::nothrow_invocable<setter_t, Args...>) // clang-format on
                -> decltype(auto)
            {
                return p.setter_(::std::forward<Args>(args)...);
            };

        private:
            struct get_fn
            {
                [[nodiscard]] constexpr decltype(auto) operator()(const property_member& p) const
                    noexcept(::blurringshadow::utility::nothrow_invocable<getter_t>)
                {
                    return p.getter_();
                }
            };

        public:
            static constexpr get_fn get{};
        };

        template<typename... Args>
        constexpr decltype(auto) set(Args&&... args) //
            noexcept(::blurringshadow::utility::nothrow_invocable<setter_t, Args...>)
        {
            return func_obj::set(*this, ::std::forward<Args>(args)...);
        };

        constexpr decltype(auto) get() const
            noexcept(::blurringshadow::utility::nothrow_invocable<getter_t>)
        {
            return func_obj::get(*this);
        };

        template<typename T>
        constexpr auto& operator=(T&& t) //
            noexcept(::blurringshadow::utility::nothrow_assignable_from<setter_t, T>)
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
