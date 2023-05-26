#pragma once

#include "../type_traits/member.h"
#include "../utility/cast_to.h"

namespace stdsharp::reflection
{
    template<typename Reflected>
    struct member
    {
    private:
        template<typename...>
        struct meta_set;

        template<ltr Name, typename Getter>
        struct meta_base : Getter
        {
            using reflected = Reflected;
            static constexpr auto name = Name;
        };

        template<ltr Name, typename T>
        using func_meta = meta_base<Name, T>;

        template<auto Ptr>
        struct data_meta_getter : member_pointer_traits<Ptr>
        {
            template<typename T>
                requires std::invocable<cast_fwd_fn<Reflected>, T>
            constexpr decltype(auto) operator()(T&& t) const
                noexcept(nothrow_invocable<cast_fwd_fn<Reflected>, T>)
            {
                return cast_fwd<Reflected>(cpp_forward(t)).*Ptr;
            }
        };

        template<ltr Name, auto Ptr>
        using data_meta = meta_base<Name, data_meta_getter<Ptr>>;

        template<ltr... Name, typename... Getter>
        struct meta_set<meta_base<Name, Getter>...> : indexed_types<meta_base<Name, Getter>...>
        {
            template<decltype(auto) N>
                requires(std::ranges::equal(N, Name) || ...)
            using member_of_t = std::invoke_result_t< //
                invocables<meta_base<Name, Getter> (*)(constant<std::ranges::equal(N, Name)>)...>,
                constant<true> // clang-format off
            >; // clang-format on

            template<decltype(auto) N>
            static consteval member_of_t<N> member_of()
            {
                return {};
            }
        };

    public:
        template<ltr... Name, typename... T>
            requires(cpp_is_constexpr(T{}) && ...)
        static consteval meta_set<func_meta<Name, T>...> func_reflect(const T...)
        {
            return {};
        }

        template<ltr... Name, member_of<Reflected> auto... Ptr>
        static consteval meta_set<data_meta<Name, Ptr>...>
            data_reflect(const regular_value_sequence<Ptr...>)
        {
            return {};
        }
    };

    template<typename Reflected>
    inline constexpr std::conditional_t<dependent_false<Reflected>(), int, void> function;

    template<typename Reflected>
    using function_t = decltype(function<Reflected>);

    template<typename Reflected>
    inline constexpr std::conditional_t<dependent_false<Reflected>(), int, void> data;

    template<typename Reflected>
    using data_t = decltype(data<Reflected>);

    template<typename T, typename U>
    inline constexpr auto data<std::pair<T, U>> =
        member<std::pair<T, U>>::template data_reflect<"first"_ltr, "second"_ltr>(
            regular_value_sequence<&std::pair<T, U>::first, &std::pair<T, U>::second>{}
        );
}