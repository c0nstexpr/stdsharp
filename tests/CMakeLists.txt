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
set(utility_tratis_header_dir "${utility_header_dir}/traits")
set(utility_property_header_dir "${utility_header_dir}/property")

set(utility_src_dir "src")
set(utility_tratis_src_dir "${utility_src_dir}/traits")
set(utility_property_src_dir "${utility_src_dir}/property")

set(exe_src ${utility_src_dir}/tests.cpp)
set(
    headers 
    ${utility_header_dir}/test_utils.h
    ${utility_header_dir}/algorithm_test.h
    ${utility_tratis_header_dir}/sequence_test.h
)
set(
    src
    ${utility_src_dir}/algorithm_test.cpp
    ${utility_tratis_src_dir}/sequence_test.cpp
)
config_exe(${PROJECT_NAME} ${exe_src} INCLUDES "${headers}" SOURCES "${src}")

#
# Load the desired unit testing framework
#
CPMAddPackage(
  NAME ut
  GITHUB_REPOSITORY boost-ext/ut
  GIT_TAG 248ee8a37cc1d6e4178f3220bff5f82d380f0f05
  OPTIONS "BOOST_UT_DISABLE_MODULE ON"
)
target_link_libraries(${PROJECT_NAME}_LIB PUBLIC ut blurringshadow::utility)

#
# Add the unit tests
#
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

verbose_message("Finished adding unit tests for ${CMAKE_PROJECT_NAME}.")
