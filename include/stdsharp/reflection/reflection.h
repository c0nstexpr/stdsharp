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

        template<literals::ltr Name, typename Getter>
        struct meta_base : Getter
        {
            using reflected = Reflected;
            static constexpr auto name = Name;
        };

        template<literals::ltr Name, typename T>
        using func_meta = meta_base<Name, T>;

        template<auto Ptr>
        struct data_meta_getter : member_pointer_traits<Ptr>
        {
            constexpr decltype(auto) operator()(decay_same_as<Reflected> auto&& t) const noexcept
            {
                return cpp_forward(t).*Ptr;
            }
        };

        template<literals::ltr Name, auto Ptr>
        using data_meta = meta_base<Name, data_meta_getter<Ptr>>;

        template<literals::ltr... Name, typename... Getter>
        struct meta_set<meta_base<Name, Getter>...>
        {
            template<auto N>
                requires(std::ranges::equal(N, Name) || ...)
            static consteval auto member_of()
            {
                constexpr std::array found_indices{std::ranges::equal(N, Name)...};
                constexpr auto i = std::ranges::find(found_indices, true) - found_indices.cbegin();
                return std::tuple_element_t<i, indexed_types<meta_base<Name, Getter>...>>{};
            }
        };

    public:
        template<literals::ltr... Name, typename... T>
            requires(cpp_is_constexpr(T{}) && ...)
        static consteval meta_set<func_meta<Name, T>...> func_reflect(const T... /*unused*/)
        {
            return {};
        }

        template<literals::ltr... Name, member_of<Reflected> auto... Ptr>
        static consteval meta_set<data_meta<Name, Ptr>...>
            data_reflect(const regular_value_sequence<Ptr...> /*unused*/)
        {
            return {};
        }
    };

    template<typename Reflected>
    inline constexpr auto function = empty;

    template<typename Reflected>
    inline constexpr auto data = empty;

    template<typename T, typename U>
    inline constexpr auto data<std::pair<T, U>> = member<std::pair<T, U>>::
        template data_reflect<literals::ltr{"first"}, literals::ltr{"second"}>(
            regular_value_sequence<&std::pair<T, U>::first, &std::pair<T, U>::second>{}
        );
}