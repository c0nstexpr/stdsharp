#
# Project details
#
project(${CMAKE_PROJECT_NAME}Tests LANGUAGES CXX)
init_proj()

verbose_message("Adding tests under ${CMAKE_PROJECT_NAME}...")

#
# Set the sources for the unit tests and add the executable(s)
#
set(src_dir "src")
set(utility_src_dir "${src_dir}/utility")
set(containers_src_dir "${src_dir}/containers")
set(traits_src_dir "${src_dir}/type_traits")
set(functional_src_dir "${src_dir}/functional")
set(filesystem_src_dir "${src_dir}/filesystem")

set(
    src
    ${src_dir}/tests.cpp
    ${src_dir}/algorithm_test.cpp
    ${src_dir}/pattern_match_test.cpp
    ${src_dir}/concurrent_object_test.cpp
    ${utility_src_dir}/utility_test.cpp
    ${containers_src_dir}/containers_test.cpp
    ${containers_src_dir}/actions_test.cpp
    ${traits_src_dir}/value_sequence_test.cpp
    ${traits_src_dir}/type_sequence_test.cpp
    ${traits_src_dir}/member_test.cpp
    ${functional_src_dir}/decompose_test.cpp
    ${functional_src_dir}/symmetric_operations_test.cpp
    ${filesystem_src_dir}/filesystem_test.cpp
)

config_exe(${PROJECT_NAME} "${src}")

target_compile_options(
    ${PROJECT_NAME}
    PUBLIC $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:-Werror>
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
)

find_package(Catch2 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE stdsharp Catch2::Catch2WithMain)

#
# Configure clang-tidy
#
option(ENABLE_CLANG_TIDY "Enable clang-tidy" ON)

if(${ENABLE_CLANG_TIDY})
    message(STATUS "clang-tidy is enabled")
    target_enable_clang_tidy(${PROJECT_NAME})
endif()

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

verbose_message("Finished adding unit tests for ${CMAKE_PROJECT_NAME}.")