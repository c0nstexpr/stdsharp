cmake_minimum_required(VERSION 3.15)

include(cmake/Utils.cmake)

set(CMAKE_CXX_STANDARD 23)

#
# Project details
#
project("stdsharp" VERSION 0.1.0 LANGUAGES CXX)

init_proj()

config_interface_lib(stdsharp)

#
# Model project dependencies
#
find_package(fmt CONFIG REQUIRED)
find_package(range-v3 CONFIG REQUIRED)

target_link_libraries(
    stdsharp
    INTERFACE fmt::fmt range-v3::range-v3
)

verbose_message("Successfully added all dependencies and linked against them.")

#
# Install library for easy downstream inclusion
#
target_install(stdsharp VER ${PROJECT_VERSION} NAMESPACE "stdsharp::")

option(STDSHARP_BUILD_TEST CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
if(STDSHARP_BUILD_TEST)
    add_subdirectory(tests)
endif()