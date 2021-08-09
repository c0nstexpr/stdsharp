// Created by BlurringShadow at 2021-03-11-下午 4:30

#pragma once

#include "utility/traits/object.h"
#include "setter.h"
#include "getter.h"

namespace blurringshadow::utility::property
{
    template<typename GetterFn, typename SetterFn>
    class property_member : ::blurringshadow::utility::traits::fixed_object
    {
    public:
        using getter_t = getter<GetterFn>;
        using setter_t = setter<SetterFn>;

    private:
        getter_t& getter_;
        setter_t& setter_;

        class set_fn :
            ::blurringshadow::utility::traits::fixed_object,
            public ::blurringshadow::utility::nodiscard_invocable_obj<setter_t&>
        {
            property_member& p;

        public:
            constexpr set_fn(
                property_member& p,
                const ::blurringshadow::utility::traits:: // clang-format off
                    private_object<property_member>
            ) : p(p),
                ::blurringshadow::utility::nodiscard_invocable_obj<setter_t&>(p.setter_) // clang-format on
            {
            }

            template<typename T>
            constexpr auto& operator=(T&& t) //
                noexcept(::blurringshadow::utility::nothrow_assignable_from<setter_t, T>)
            {
                p.setter_ = std::forward<T>(t);
                return *this;
            }
        };

        class get_fn :
            ::blurringshadow::utility::traits::fixed_object,
            public ::blurringshadow::utility::nodiscard_invocable_obj<getter_t&>
        {
        public:
            constexpr get_fn(
                property_member& p,
                const ::blurringshadow::utility::traits:: // clang-format off
                    private_object<property_member>
            ) : ::blurringshadow::utility::nodiscard_invocable_obj<getter_t&>(p.getter_) // clang-format on
            {
            }
        };

    public:
        set_fn set{*this, ::blurringshadow::utility::traits::private_object<property_member>{}};
        get_fn get{*this, ::blurringshadow::utility::traits::private_object<property_member>{}};

        template<typename T>
        constexpr auto& operator=(T&& t) noexcept(noexcept((set = std::forward<T>(t))))
        {
            set = std::forward<T>(t);
            return *this;
        }

        property_member(getter_t& g, setter_t& s) noexcept: getter_(g), setter_(s) {}
    };

    inline constexpr ::blurringshadow::utility::nodiscard_invocable_obj get_property{
        []<typename GetterFn, typename SetterFn>(
            getter<GetterFn>& getter_v,
            setter<SetterFn>& setter_v // clang-format off
        ) noexcept // clang-format on
        {
            return ::ranges::overloaded<getter<GetterFn>&, setter<SetterFn>&>{
                getter_v, setter_v //
            };
        } //
    };
}
