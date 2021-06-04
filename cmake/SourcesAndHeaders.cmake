set(${PROJECT_NAME}_BUILD_HEADERS_ONLY ON)

set(sources)

set(exe_sources)

set(utility_header_dir "include/utility")

set(
    headers
    "${utility_header_dir}/property/member_setter.h"
    "${utility_header_dir}/property/member_getter.h"

    "${utility_header_dir}/traits/function.h"
    "${utility_header_dir}/traits/integer_sequence.h"
    "${utility_header_dir}/traits/member.h"
    "${utility_header_dir}/traits/type_container.h"

    "${utility_header_dir}/concurrent_object.h"
    "${utility_header_dir}/string_conversion.h"
    "${utility_header_dir}/type_traits.h"
    "${utility_header_dir}/utility_core.h"
)
