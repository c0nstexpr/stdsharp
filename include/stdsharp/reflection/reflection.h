#pragma once

#include <ranges>
#include <stdexcept>

#include "../type_traits/member.h"

namespace stdsharp::reflection
{
    template<typename Reflected>
    struct member
    {
    private:
        template<ltr Name, typename T>
        struct invoker : T
        {
            using reflected = Reflected;
            static constexpr auto name = Name;

            consteval auto operator[](const ::std::string_view n)
            {
                if(n == name) return *this;
                throw ::std::invalid_argument{"Invalid member name"};
            }
        };

        template<ltr Name, auto Ptr>
        struct data_mem
        {
            using reflected = Reflected;
            static constexpr auto name = Name;
            using type = member_t<Ptr>;

            consteval auto operator[](const ::std::string_view n)
            {
                if(n == name) return *this;
                throw ::std::invalid_argument{"Invalid member name"};
            }
        };

    public:
        template<ltr Name, typename T>
            requires cpp_is_constexpr(T{})
        static consteval invoker<Name, T> func_reflect(const T = {}) noexcept
        {
            return {};
        }

        template<ltr Name, member_of<Reflected> auto Ptr>
        static consteval data_mem<Name, Ptr> data_reflect() noexcept
        {
            return {};
        }

        static consteval const Reflected& cast(const Reflected& t) noexcept { return t; }
    };

    template<typename Reflected>
    inline constexpr ::std::conditional_t<dependent_false<Reflected>(), int, void> function;

    template<typename Reflected>
    inline constexpr ::std::conditional_t<dependent_false<Reflected>(), int, void> data;

    template<typename T, typename U>
    inline constexpr auto data<::std::pair<T, U>> = make_indexed_values(
        member<::std::pair<T, U>>::template data_reflect<"first", &::std::pair<T, U>::first>(),
        member<::std::pair<T, U>>::template data_reflect<"second", &::std::pair<T, U>::second>()
    );
}