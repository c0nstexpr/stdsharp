cmake_minimum_required(VERSION 3.15)

include(cmake/Utils.cmake)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

#
# Project details
#
project("stdsharp" VERSION 0.1.0 LANGUAGES CXX)

init_proj()

config_interface_lib(${PROJECT_NAME} SYSTEM_INCLUDED ON)

#
# Model project dependencies
#
find_package(fmt CONFIG REQUIRED)
find_package(range-v3 CONFIG REQUIRED)

message(STATUS "adding scope-lite")
include(FetchContent)
FetchContent_Declare(
  scope-lite
  GIT_REPOSITORY https://github.com/martinmoene/scope-lite.git
  GIT_TAG 1046bb766ebf65eef51afdd0237808f7468dacd4
)
FetchContent_MakeAvailable(scope-lite)


target_compile_options(
    ${PROJECT_NAME}
    INTERFACE $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:-Wno-logical-op-parentheses -Wno-dangling-else>
              $<$<CXX_COMPILER_ID:MSVC>:/wd4127>
)

target_link_libraries(
    ${PROJECT_NAME}
    INTERFACE fmt::fmt range-v3::range-v3 scope-lite
)

verbose_message("Successfully added all dependencies and linked against them.")

include(GNUInstallDirs)
target_install(
  NAME ${PROJECT_NAME}
  VERSION ${PROJECT_VERSION}
  NAMESPACE ${PROJECT_NAME}
  BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}
  INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include
  INCLUDE_DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  COMPATIBILITY SameMajorVersion
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(
    FILES ${CPM_DOWNLOAD_LOCATION}
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake
)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(STDSHARP_BUILD_TEST ON)
else()
    target_include_as_system(${PROJECT_NAME} INTERFACE)
endif()
option(STDSHARP_BUILD_TEST "Whether to build test")

if(STDSHARP_BUILD_TEST)
    add_subdirectory(tests)
endif()