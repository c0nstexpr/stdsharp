cmake_minimum_required(VERSION 3.15)

include(cmake/Utils.cmake)

set(CMAKE_CXX_STANDARD 20)

#
# Project details
#
project("std_sharp" VERSION 0.1.0 LANGUAGES CXX)

init_proj()

#
# Create library, setup header and source files
#
#set(utility_header_dir "include/utility")
#set(utility_traits_header_dir "${utility_header_dir}/traits")
#set(utility_property_header_dir "${utility_header_dir}/property")
#
#set(
#    headers
#    ${utility_property_header_dir}/setter.h
#    ${utility_property_header_dir}/getter.h
#    ${utility_property_header_dir}/property.h
#
#    ${utility_traits_header_dir}/value_sequence.h
#    ${utility_traits_header_dir}/type_sequence.h
#    ${utility_traits_header_dir}/function.h
#    ${utility_traits_header_dir}/member.h
#
#    ${utility_header_dir}/concurrent_object.h
#    ${utility_header_dir}/type_traits.h
#    ${utility_header_dir}/algorithm.h
#    ${utility_header_dir}/functional.h
#    ${utility_header_dir}/random.h
#    ${utility_header_dir}/utility.h
#    ${utility_header_dir}/assert.h
#    ${utility_header_dir}/pattern_match.h
#)

config_interface_lib(std_sharp_utility)

#
# Provide alias to library
#
add_library(std_sharp::utility ALIAS std_sharp_utility)

#
# Model project dependencies 
#
find_package(fmt CONFIG REQUIRED)
find_package(range-v3 CONFIG REQUIRED)

target_link_libraries(
    std_sharp_utility
    INTERFACE fmt::fmt range-v3::range-v3
)

verbose_message("Successfully added all dependencies and linked against them.")

#
# Install library for easy downstream inclusion
#
target_install(std_sharp_utility VER ${PROJECT_VERSION} NAMESPACE "std_sharp::")

#
# Unit testing setup
#
enable_testing()
message(STATUS "Build unit tests for the project\n")
add_subdirectory(tests)
