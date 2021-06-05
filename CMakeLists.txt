cmake_minimum_required(VERSION 3.15)

include(cmake/Utils.cmake)
#
# Project details
#
project("blurringshadow" VERSION 0.1.0 LANGUAGES CXX)

init_proj()

#
# Create library, setup header and source files
#
set(utility_header_dir "include/utility")

set(
    headers
    ${utility_header_dir}/property/member_setter.h
    ${utility_header_dir}/property/member_getter.h

    ${utility_header_dir}/traits/function.h
    ${utility_header_dir}/traits/integer_sequence.h
    ${utility_header_dir}/traits/member.h
    ${utility_header_dir}/traits/type_container.h

    ${utility_header_dir}/concurrent_object.h
    ${utility_header_dir}/string_conversion.h
    ${utility_header_dir}/type_traits.h
    ${utility_header_dir}/utility_core.h
)

config_interface_lib(blurringshadow_utility "${headers}" STD 20)

#
# Provide alias to library
#
add_library(blurringshadow::utility ALIAS blurringshadow_utility)

#
# Model project dependencies 
#
find_package(fmt CONFIG REQUIRED)
find_package(Microsoft.GSL CONFIG REQUIRED)

target_link_libraries(
    blurringshadow_utility 
    INTERFACE 
        fmt::fmt
        Microsoft.GSL::GSL
)

verbose_message("Successfully added all dependencies and linked against them.")

#
# Format the project using the `clang-format` target (i.e: cmake --build build --target clang-format)
#
target_add_clang_format("${headers}")

#
# Install library for easy downstream inclusion
#
target_install(blurringshadow_utility VER ${PROJECT_VERSION} NAMESPACE "blurringshadow::")

#
# Unit testing setup
#

if(${PROJECT_NAME}_ENABLE_UNIT_TESTING)
    enable_testing()
	message(STATUS "Build unit tests for the project\n")
    add_subdirectory(tests)
endif()
