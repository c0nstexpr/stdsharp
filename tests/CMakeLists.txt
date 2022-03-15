#
# Project details
#
project(${CMAKE_PROJECT_NAME}Tests LANGUAGES CXX)
init_proj()

verbose_message("Adding tests under ${CMAKE_PROJECT_NAME}...")

#
# Set the sources for the unit tests and add the executable(s)
#
set(header_dir "include")
set(utility_header_dir "${header_dir}/utility")
set(containers_header_dir "${header_dir}/containers")
set(traits_header_dir "${header_dir}/type_traits")
set(functional_header_dir "${header_dir}/functional")

set(src_dir "src")
set(utility_src_dir "${src_dir}/utility")
set(containers_src_dir "${src_dir}/containers")
set(traits_src_dir "${src_dir}/type_traits")
set(functional_src_dir "${src_dir}/functional")

set(exe_src ${src_dir}/tests.cpp)
set(
    headers
    ${header_dir}/test_utils.h
    ${header_dir}/test.h
    ${header_dir}/algorithm_test.h
    ${header_dir}/pattern_match_test.h
    ${header_dir}/concurrent_object_test.h
    ${utility_header_dir}/utility_test.h
    ${containers_header_dir}/containers_test.h
    ${containers_header_dir}/actions_test.h
    ${traits_header_dir}/value_sequence_test.h
    ${traits_header_dir}/type_sequence_test.h
    ${traits_header_dir}/member_test.h
    ${functional_header_dir}/decompose_test.h
    ${functional_header_dir}/symmetric_operations_test.h
)
set(
    src
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
)
config_exe(${PROJECT_NAME} ${exe_src} INCLUDES "${headers}" SOURCES "${src}")

if (MSVC) # visual studio doesn't support clang-tidy directly by cmake
    set_target_properties(
        ${PROJECT_NAME}
        PROPERTIES
        VS_GLOBAL_EnableClangTidyCodeAnalysis true
    )
    set(additional_option /W0)
    set(BOOST_UT_DISABLE_MODULE true)
else ()
    if (DEFINED ENV{CLANG_TIDY_WHERE})
        set_target_properties(${PROJECT_NAME} PROPERTIES CXX_CLANG_TIDY "$ENV{CLANG_TIDY_WHERE}")
    endif ()
    if (CMAKE_CXX_COMPILER_ID MATCHES "(.*Clang)|GNU")
        set(additional_option -w)
    endif ()
endif ()

target_compile_options(${PROJECT_NAME}_LIB PUBLIC ${additional_option})

find_package(ut)
target_link_libraries(${PROJECT_NAME}_LIB PUBLIC ut stdsharp)

#
# Add the unit tests
#
enable_testing()
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

verbose_message("Finished adding unit tests for ${CMAKE_PROJECT_NAME}.")