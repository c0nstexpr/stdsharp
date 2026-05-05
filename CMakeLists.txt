cmake_minimum_required(VERSION 4.3.0)

include(cmake/Utils.cmake)
include(CMakeDependentOption)

project(stdsharp VERSION 0.9.2 LANGUAGES CXX)

find_package(ctre CONFIG REQUIRED)
find_package(Microsoft.GSL CONFIG REQUIRED)
add_interface_target(${PROJECT_NAME} STD 26 LINKS Microsoft.GSL::GSL ctre::ctre)

install(
    DIRECTORY cmake
    DESTINATION ${PROJECT_INSTALL_CMAKEDIR}/stdsharp
)

cmake_dependent_option(
    ${PROJECT_NAME}_BUILD_TEST
    "Whether to build test"
    ON
    PROJECT_IS_TOP_LEVEL
    OFF
)

if(${PROJECT_NAME}_BUILD_TEST)
    add_subdirectory(tests)
endif()

project_install(
    ${PROJECT_NAME}
    COMPATIBILITY SameMajorVersion
    DEPENDENCIES ctre;range-v3;Microsoft.GSL
)
