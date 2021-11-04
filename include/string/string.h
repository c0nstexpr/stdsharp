#pragma once

#include <string>
#include <string_view>

#include <unicode/unistr.h>

#include "functional/invocable_obj.h"

namespace stdsharp::string
{
    using namespace ::std::literals;

    inline constexpr functional::invocable_obj to_utf8(
        functional::nodiscard_tag,
        [](const ::std::string_view view)
        {
            ::std::string str;
            str.reserve(view.size());
            ::icu::UnicodeString{view.data()}.toUTF8String(str);

            return ::std::move(str);
        } //
    );
}