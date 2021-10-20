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
        struct member_t : ::stdsharp::functional::nodiscard_tag_t
        {
            static constexpr ::std::string_view name = //
                {::std::ranges::begin(Literal), ::std::ranges::end(Literal)};
        };

        template<auto Literal>
        struct data_member_t : ::stdsharp::reflection::details::member_t<Literal>
        {
        };

        template<auto Literal>
        struct member_function_t : ::stdsharp::reflection::details::member_t<Literal>
        {
        };
    }

    template<::std::ranges::input_range auto Literal>
    using member_t = ::stdsharp::reflection::details::member_t<Literal>;

    template<::std::ranges::input_range auto Literal>
    inline constexpr auto member =
        ::stdsharp::functional::tagged_cpo<::stdsharp::reflection::member_t<Literal>>;

    template<::std::ranges::input_range auto Literal>
    using data_member_t = ::stdsharp::reflection::details::data_member_t<Literal>;

    template<::std::ranges::input_range auto Literal>
    inline constexpr auto data_member =
        ::stdsharp::functional::tagged_cpo<::stdsharp::reflection::data_member_t<Literal>>;

    template<::std::ranges::input_range auto Literal>
    using member_function_t = ::stdsharp::reflection::details::member_function_t<Literal>;

    template<::std::ranges::input_range auto Literal>
    inline constexpr auto member_function =
        ::stdsharp::functional::tagged_cpo<::stdsharp::reflection::member_function_t<Literal>>;

    struct data_members_t : ::stdsharp::functional::nodiscard_tag_t
    {
    };

    inline constexpr auto data_members =
        ::stdsharp::functional::tagged_cpo<::stdsharp::reflection::data_members_t>;
}
