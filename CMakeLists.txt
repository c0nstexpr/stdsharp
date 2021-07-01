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
set(utility_tratis_header_dir "${utility_header_dir}/traits")
set(utility_property_header_dir "${utility_header_dir}/property")

set(
    headers
    ${utility_property_header_dir}/member_setter.h
    ${utility_property_header_dir}/member_getter.h

    ${utility_tratis_header_dir}/function.h
    ${utility_tratis_header_dir}/value_sequence.h
    ${utility_tratis_header_dir}/sequence_set.h
    ${utility_tratis_header_dir}/member.h
    ${utility_tratis_header_dir}/type_container.h

    ${utility_header_dir}/concurrent_object.h
    ${utility_header_dir}/string_conversion.h
    ${utility_header_dir}/type_traits.h
    ${utility_header_dir}/algorithm.h
    ${utility_header_dir}/functional.h
    ${utility_header_dir}/random.h
    ${utility_header_dir}/utility.h
)

config_interface_lib(blurringshadow_utility "${headers}" STD 20)

#
# Provide alias to library
#
add_library(blurringshadow::utility ALIAS blurringshadow_utility)

#
# Format the lib
#
clang_format(blurringshadow_utility)

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
# Install library for easy downstream inclusion
#
target_install(blurringshadow_utility VER ${PROJECT_VERSION} NAMESPACE "blurringshadow::")

#
# Unit testing setup
#
enable_testing()
message(STATUS "Build unit tests for the project\n")
add_subdirectory(tests)
