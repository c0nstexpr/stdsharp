#include "functional/functional.h"
#include "functional/operation.h"
#include "pattern_match.h"

namespace stdsharp
{
    inline constexpr struct symmetric_operation_t
    {
        template<typename T>
        constexpr auto operator()([[maybe_unused]] const T&) const
        {
            constexpr_pattern_match::from_type<T>( //
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
    } symmetric_operation{};
}