cmake_minimum_required(VERSION 3.15)

include(cmake/Utils.cmake)

set(CMAKE_CXX_STANDARD 23)

#
# Project details
#
project("stdsharp" VERSION 0.1.0 LANGUAGES CXX)

init_proj()

config_interface_lib(std_sharp_utility)

#
# Provide alias to library
#
add_library(stdsharp::utility ALIAS std_sharp_utility)

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
target_install(std_sharp_utility VER ${PROJECT_VERSION} NAMESPACE "stdsharp::")

#
# Unit testing setup
#
enable_testing()
message(STATUS "Build unit tests for the project\n")
add_subdirectory(tests)
