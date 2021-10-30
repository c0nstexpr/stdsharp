//
// Created by BlurringShadow on 2021-10-20.
//

#include "functional/cpo.h"
#include "type_traits/type_traits.h"

namespace stdsharp::reflection
{
    namespace details
    {
        template<auto Literal>
        struct member_t : functional::nodiscard_tag_t
        {
            static constexpr ::std::string_view name = //
                {::std::ranges::begin(Literal), ::std::ranges::end(Literal)};
        };

        template<auto Literal>
        struct data_member_t : details::member_t<Literal>
        {
        };

        template<auto Literal>
        struct member_function_t : details::member_t<Literal>
        {
        };
    }

    template<::std::ranges::input_range auto Literal>
    using member_t = details::member_t<Literal>;

    template<::std::ranges::input_range auto Literal>
    inline constexpr auto member = functional::tagged_cpo<member_t<Literal>>;

    template<::std::ranges::input_range auto Literal>
    using data_member_t = details::data_member_t<Literal>;

    template<::std::ranges::input_range auto Literal>
    inline constexpr auto data_member = functional::tagged_cpo<data_member_t<Literal>>;

    template<::std::ranges::input_range auto Literal>
    using member_function_t = details::member_function_t<Literal>;

    template<::std::ranges::input_range auto Literal>
    inline constexpr auto member_function = functional::tagged_cpo<member_function_t<Literal>>;

    struct data_members_t : functional::nodiscard_tag_t
    {
    };

    inline constexpr auto data_members = functional::tagged_cpo<data_members_t>;
}
