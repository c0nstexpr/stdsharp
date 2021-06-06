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
# Load the desired unit testing framework
#
CPMAddPackage("gh:boost-ext/ut@1.1.8")
target_link_libraries(${PROJECT_NAME} boost::ut blurringshadow::utility)

#
# Configure code coverage
#
option(ENABLE_COVERAGE "Enable test coverage" OFF)

if(ENABLE_COVERAGE)
    set(url https://raw.githubusercontent.com/bilke/cmake-modules/master/CodeCoverage.cmake)
    set(cmake_path ${CMAKE_BINARY_DIR}/CodeCoverage.cmake)
    if(NOT EXISTS "${cmake_path}")
        message(STATUS "Downloading code coverage util `CodeCoverage.cmake` from ${url}")
        file(DOWNLOAD "${url}" "${cmake_path}")
    endif()
    include(${cmake_path})
    message(STATUS "Downloaded succesfully.")

    setup_target_for_coverage_gcovr_xml(
        NAME ${PROJECT_NAME}Coverage
        EXECUTABLE ctest -j ${PROCESSOR_COUNT}
        DEPENDENCIES ${PROJECT_NAME}
        BASE_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

#
# Add the unit tests
#
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

verbose_message("Finished adding unit tests for ${CMAKE_PROJECT_NAME}.")

