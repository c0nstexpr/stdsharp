//
// Created by BlurringShadow on 2021-10-20.
//

#include "functional/cpo.h"

namespace stdsharp::reflection
{
    using namespace ::std::literals;

    namespace details
    {
        template<auto Literal>
        struct member_t : functional::nodiscard_tag_t
        {
            static constexpr ::std::string_view name = //
                {::std::ranges::begin(Literal), ::std::ranges::end(Literal)};
        };

        template<typename>
        struct is_std_pair;

        template<typename T, typename U>
        struct is_std_pair<::std::pair<T, U>>
        {
        };

        template<auto Literal>
        struct data_member_t : details::member_t<Literal>
        {
            template<typename T>
                requires requires
                {
                    requires ::std::ranges::equal(Literal, "first");
                    is_std_pair<::std::remove_cvref_t<T>>{};
                }
            constexpr auto& operator()(T&& p) const noexcept { return ::std::forward<T>(p).first; }

            template<typename T>
                requires requires
                {
                    requires ::std::ranges::equal(Literal, "second");
                    is_std_pair<::std::remove_cvref_t<T>>{};
                }
            constexpr auto& operator()(T&& p) const noexcept { return ::std::forward<T>(p).second; }
        };

        template<auto Literal>
        struct member_function_t : details::member_t<Literal>
        {
        };

        template<typename LiteralT>
        concept comparable_literal = requires(LiteralT literal)
        {
            ::std::ranges::equal(literal, "");
            ::std::ranges::equal("", literal);
            ::std::ranges::equal(literal, literal);
        };
    }

    template<details::comparable_literal auto Literal>
    using member_t = details::member_t<Literal>;

    template<details::comparable_literal auto Literal>
    inline constexpr auto member = functional::tagged_cpo<member_t<Literal>>;

    template<details::comparable_literal auto Literal>
    using data_member_t = details::data_member_t<Literal>;

    template<details::comparable_literal auto Literal>
    inline constexpr auto data_member = functional::tagged_cpo<data_member_t<Literal>>;

    template<details::comparable_literal auto Literal>
    using member_function_t = details::member_function_t<Literal>;

    template<details::comparable_literal auto Literal>
    inline constexpr auto member_function = functional::tagged_cpo<member_function_t<Literal>>;

    template<typename>
    struct data_members_t;

    template<typename T>
        requires requires { details::is_std_pair<::std::remove_cvref_t<T>>{}; }
    struct reflection::data_members_t<T>
    {
        static constexpr auto value = {"first"sv, "second"sv};
    };
}
