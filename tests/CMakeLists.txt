#
# Project details
#
project(${CMAKE_PROJECT_NAME}Tests LANGUAGES CXX)
init_proj()

verbose_message("Adding tests under ${PROJECT_NAME}...")

#
# Set the sources for the unit tests and add the executable(s)
#
set(src src/tests.cpp)
config_exe(${PROJECT_NAME} ${src} STD 20)

#
# Setup code coverage if enabled
#
if (${CMAKE_PROJECT_NAME}_ENABLE_CODE_COVERAGE)
    target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC -O0 -g -fprofile-arcs -ftest-coverage)
    target_link_options(${CMAKE_PROJECT_NAME} PUBLIC -fprofile-arcs -ftest-coverage)
    verbose_message("Code coverage is enabled and provided with GCC.")
endif()

#
# Load the desired unit testing framework
#
CPMAddPackage("gh:boost-ext/ut@1.1.8")
target_link_libraries(${PROJECT_NAME} boost::ut blurringshadow::utility)

#
# Add the unit tests
#
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

verbose_message("Finished adding unit tests for ${CMAKE_PROJECT_NAME}.")
