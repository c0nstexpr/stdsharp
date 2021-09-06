#
# Project details
#
project(${CMAKE_PROJECT_NAME}Tests LANGUAGES CXX)
init_proj()

verbose_message("Adding tests under ${PROJECT_NAME}...")

#
# Set the sources for the unit tests and add the executable(s)
#
set(utility_header_dir "include")
set(utility_traits_header_dir "${utility_header_dir}/traits")
set(utility_property_header_dir "${utility_header_dir}/property")

set(utility_src_dir "src")
set(utility_traits_src_dir "${utility_src_dir}/traits")
set(utility_property_src_dir "${utility_src_dir}/property")

set(exe_src ${utility_src_dir}/tests.cpp)
set(
    headers
    ${utility_header_dir}/test_utils.h
    ${utility_header_dir}/utility_test.h
    ${utility_header_dir}/algorithm_test.h
    ${utility_header_dir}/random_test.h
    ${utility_header_dir}/pattern_match_test.h
    ${utility_traits_header_dir}/value_sequence_test.h
    ${utility_traits_header_dir}/type_sequence_test.h
    ${utility_traits_header_dir}/member_test.h
    ${utility_property_header_dir}/property_test.h
)
set(
    src
    ${utility_src_dir}/utility_test.cpp
    ${utility_src_dir}/algorithm_test.cpp
    ${utility_src_dir}/random_test.cpp
    ${utility_src_dir}/pattern_match_test.cpp
    ${utility_traits_src_dir}/value_sequence_test.cpp
    ${utility_traits_src_dir}/type_sequence_test.cpp
    ${utility_traits_src_dir}/member_test.cpp
    ${utility_property_src_dir}/property_test.cpp
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

target_include_directories(
    ${PROJECT_NAME}_LIB
    SYSTEM PUBLIC $<TARGET_PROPERTY:ut,INTERFACE_INCLUDE_DIRECTORIES>
)

add_subdirectory(./ut)

target_link_libraries(${PROJECT_NAME}_LIB PUBLIC ut std_sharp::utility)


#
# Add the unit tests
#
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

verbose_message("Finished adding unit tests for ${CMAKE_PROJECT_NAME}.")
