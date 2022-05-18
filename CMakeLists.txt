cmake_minimum_required(VERSION 3.15)

include(cmake/Utils.cmake)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED true)

#
# Project details
#
project("stdsharp" VERSION 0.2.2 LANGUAGES CXX)

init_proj()

config_interface_lib(${PROJECT_NAME})

#
# Model project dependencies
#
find_package(fmt CONFIG REQUIRED)
find_package(range-v3 CONFIG REQUIRED)
find_package(ctre CONFIG REQUIRED)

target_compile_options(
    ${PROJECT_NAME}
    INTERFACE $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:-Wno-logical-op-parentheses -Wno-dangling-else>
              $<$<CXX_COMPILER_ID:MSVC>:/wd4127 /external:anglebrackets>
)

target_link_libraries(
    ${PROJECT_NAME}
    INTERFACE fmt::fmt range-v3::range-v3 ctre::ctre
)

verbose_message("Successfully added all dependencies and linked against them.")

include(GNUInstallDirs)
target_install(
  NAME ${PROJECT_NAME}
  VERSION ${PROJECT_VERSION}
  NAMESPACE ${PROJECT_NAME}
  BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}
  INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include
  INCLUDE_DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
  DEPENDENCIES fmt;ctre;range-v3
)

get_directory_property(is_subproject PARENT_DIRECTORY)

if(NOT is_subproject)
    set(is_standalone YES)
else()
    set(is_standalone NO)
    target_include_as_system(${PROJECT_NAME} INTERFACE)
endif()

include(CMakeDependentOption)

CMAKE_DEPENDENT_OPTION(STDSHARP_BUILD_TEST "Whether to build test" ON "${is_standalone}" OFF)

if(STDSHARP_BUILD_TEST)
    #
    # Add the unit tests
    #
    include(CTest)
    add_subdirectory(tests)
endif()