#pragma once

#include <ranges>

#include "../type_traits/core_traits.h"
#include "../concepts/concepts.h"

using namespace ::std::literals;

namespace stdsharp::reflection
{
    enum mem_type
    {
        function,
        data
    };

    template<typename Reflected, ltr, mem_type>
    constexpr ::std::conditional_t<dependent_false<Reflected>(), int, void> member;

    struct member_info

    {
        ::std::string_view name{};

        enum
        {
            function,
            data
        } category{};

        friend bool operator==(member_info, member_info) = default;
    };

    template<typename T>
    constexpr ::std::conditional_t<dependent_false<T>(), int, void> get_members;

    template<typename T, typename U>
    constexpr ::std::array get_members<::std::pair<T, U>>{
        member_info{"first", member_info::data},
        member_info{"second", member_info::data} //
    };

    template<ltr, typename T>
    constexpr ::std::conditional_t<dependent_false<T>(), int, void> get_member;

    template<typename T, typename U>
    stdsharp_data_member_reflect(first, ::std::pair<T, U>);

    template<typename T, typename U>
    stdsharp_data_member_reflect(second, ::std::pair<T, U>);

    template<typename T>
    inline constexpr auto get_data_members = ::std::views::filter(
        get_members<T>,
        [](const member_info info) { return info.category == member_info::data; }
    );
}