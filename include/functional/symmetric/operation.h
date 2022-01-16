#include "containers/actions.h"
#include "pattern_match.h"

namespace stdsharp::functional::symmetric
{
    struct operation_t
    {
        template<typename T>
        constexpr auto operator()(const T&) const noexcept
        {
            return constexpr_pattern_match::from_type<T>( //
                [](const ::std::type_identity<::std::plus<>>) noexcept
                {
                    return functional::minus_v; //
                },
                [](const ::std::type_identity<::std::minus<>>) noexcept
                {
                    return functional::plus_v; //
                },
                [](const ::std::type_identity<::std::multiplies<>>) noexcept
                {
                    return functional::divides_v; //
                },
                [](const ::std::type_identity<::std::divides<>>) noexcept
                {
                    return functional::multiplies_v; //
                },
                [](const ::std::type_identity<::std::negate<>>) noexcept
                {
                    return functional::negate_v; //
                } //
            );
        }
    };

    inline constexpr auto operation = functional::tagged_cpo<operation_t>;
}