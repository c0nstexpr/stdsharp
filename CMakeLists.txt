cmake_minimum_required(VERSION 3.24)

include(cmake/Utils.cmake)

#
# Project details
#
project(
  stdsharp
  VERSION 0.6.2
  LANGUAGES CXX)

config_lib(${PROJECT_NAME} INTERFACE STD 23)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

#
# Model project dependencies
#
find_package(fmt CONFIG REQUIRED)
find_package(range-v3 CONFIG REQUIRED)
find_package(ctre CONFIG REQUIRED)
find_package(nameof CONFIG REQUIRED)

target_compile_options(
  ${PROJECT_NAME} INTERFACE $<$<CXX_COMPILER_ID:MSVC>: /external:anglebrackets>
                            $<$<CXX_COMPILER_ID:Clang>: -fexperimental-library>)

target_link_options(${PROJECT_NAME} INTERFACE $<$<CXX_COMPILER_ID:Clang>:
                    -lc++experimental>)

target_link_libraries(${PROJECT_NAME} INTERFACE fmt::fmt range-v3::range-v3
                                                ctre::ctre nameof::nameof)

verbose_message("Successfully added all dependencies and linked against them.")

target_install(${PROJECT_NAME} COMPATIBILITY SameMajorVersion DEPENDENCIES
               fmt;ctre;range-v3;nameof)

install(FILES cmake/Utils.cmake
        DESTINATION ${${PROJECT_NAME}_INSTALL_CMAKEDIR}/stdsharp)

get_directory_property(is_subproject PARENT_DIRECTORY)

if(NOT is_subproject)
  set(is_standalone YES)
else()
  set(is_standalone NO)
endif()

include(CMakeDependentOption)

cmake_dependent_option(${PROJECT_NAME}_BUILD_TEST "Whether to build test" ON
                       "${is_standalone}" OFF)
cmake_dependent_option(
  ${PROJECT_NAME}_INCLUDE_AS_SYSTEM
  "Whether to include stdsharp headers as system headers" OFF
  "${is_standalone}" ON)

if(${PROJECT_NAME}_INCLUDE_AS_SYSTEM)
  target_include_as_system(${PROJECT_NAME} INTERFACE)
endif()

if(${PROJECT_NAME}_BUILD_TEST)
  include(CTest)
  add_subdirectory(tests)
endif()
