#pragma once

#include "../type_traits/member.h"
#include "../utility/cast_to.h"

namespace stdsharp::reflection
{
    template<typename Reflected, literals::ltr Name, typename Getter>
    struct member : Getter
    {
        using reflected = Reflected;
        static constexpr auto name = Name;

        template<typename... Args>
            requires std::constructible_from<Getter, Args...>
        explicit(sizeof...(Args) == 1) constexpr member(Args&&... args) //
            noexcept(nothrow_constructible_from<Getter, Args...>):
            Getter(cpp_forward(args)...)
        {
        }

        member() = default;
    };

    template<typename Reflected, literals::ltr Name>
    struct member_reflect_fn
    {
        template<typename T>
            requires std::constructible_from<std::decay_t<T>, T>
        consteval auto operator()(T&& t) const
        {
            return member<Reflected, Name, std::decay_t<T>>{cpp_forward(t)};
        }
    };

    template<typename Reflected, literals::ltr Name>
    inline constexpr member_reflect_fn<Reflected, Name> member_reflect{};

    template<auto Ptr>
    using data_meta_getter = member_pointer_traits<Ptr>;

    template<literals::ltr Name, auto Ptr>
    using data_meta = member<member_t<Ptr>, Name, data_meta_getter<Ptr>>;

    template<literals::ltr Name, auto Ptr>
    struct data_member_reflect_fn
    {
        consteval data_meta<Name, Ptr> operator()() const { return {}; }
    };

    template<literals::ltr Name, auto Ptr>
    inline constexpr data_member_reflect_fn<Name, Ptr> data_member_reflect{};

    template<typename>
    inline constexpr indexed_values<> function{};

    template<typename>
    inline constexpr indexed_values<> data{};

    template<typename T, auto N>
    struct member_of_fn
    {
    private:
        template<
            literals::ltr... Name,
            typename... Getter,
            auto EqualRes = std::array{std::ranges::equal(N, Name)...},
            auto Idx = std::ranges::find(EqualRes, true) - EqualRes.cbegin()>
        static consteval decltype(auto) impl( //
            const indexed_values<member<T, Name, Getter>...>& members
        ) noexcept
            requires requires { cpo::get_element<Idx>(members); }
        {
            return cpo::get_element<Idx>(members);
        }

    public:
        consteval auto operator()() const noexcept
            requires requires { impl(function<T>); }
        {
            return impl(function<T>);
        }

        consteval auto operator()() const noexcept
            requires requires { impl(data<T>); }
        {
            return impl(data<T>);
        }
    };

    template<typename T, auto N>
    inline constexpr member_of_fn<T, N> member_of{};

    template<typename T, typename U>
    inline constexpr indexed_values data<std::pair<T, U>>{
        data_member_reflect<literals::ltr{"first"}, &std::pair<T, U>::first>(),
        data_member_reflect<literals::ltr{"second"}, &std::pair<T, U>::second>()
    };
}