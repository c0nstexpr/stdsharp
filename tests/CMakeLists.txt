#
# Project details
#
project(${CMAKE_PROJECT_NAME}Tests LANGUAGES CXX)

verbose_message("Adding tests under ${CMAKE_PROJECT_NAME}...")

find_package(Catch2 CONFIG REQUIRED)
include(Catch)

#
# Set the sources for the unit tests and add the executable(s)
#
set(src
    src/algorithm/algorithm.cpp
    src/containers/actions.cpp
    src/containers/concepts.cpp
    src/filesystem/filesystem.cpp
    src/functional/forward_bind.cpp
    src/functional/invocables.cpp
    src/functional/pipeable.cpp
    src/functional/sequenced_invocables.cpp
    src/functional/symmetric_operations.cpp
    src/memory/box.cpp
    src/memory/composed_allocator.cpp
    src/memory/pointer_traits.cpp
    src/memory/single_stack_allocator.cpp
    src/memory/soo.cpp
    src/type_traits/indexed_traits.cpp
    src/type_traits/member.cpp
    src/type_traits/type_sequence.cpp
    src/type_traits/value_sequence.cpp
    src/utility/forward_cast.cpp
    src/utility/dispatcher.cpp
    src/utility/utility.cpp
    src/utility/value_wrapper.cpp
    src/lazy.cpp
    src/pattern_match.cpp
    src/synchronizer.cpp)

config_lib(${PROJECT_NAME}Lib INTERFACE)

target_link_libraries(${PROJECT_NAME}Lib INTERFACE stdsharp::stdsharp
                                                   Catch2::Catch2WithMain)

config_exe(${PROJECT_NAME} EXE_SRC ${src})

target_compile_options(
  ${PROJECT_NAME}
  PUBLIC $<$<CXX_COMPILER_ID:MSVC>:/W4 /wd4459>
         $<$<CXX_COMPILER_ID:Clang>:-Qunused-arguments -ftime-trace
         -fdiagnostics-show-template-tree -ftemplate-backtrace-limit=0>)
target_enable_clang_sanitizer(${PROJECT_NAME} PUBLIC SANITIZER address
                              undefined)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}Lib)

set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_WARNING_AS_ERROR ON)

#
# Configure clang-tidy
#
option(ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)

if(${ENABLE_CLANG_TIDY})
  message(STATUS "clang-tidy is enabled")

  target_enable_clang_tidy(${PROJECT_NAME})
endif()

catch_discover_tests(${PROJECT_NAME})

#
# Configure code coverage
#
set(CODE_COVERAGE_EXT CACHE STRING "Enable code coverage")

if(CODE_COVERAGE_EXT)
  set(PROFILE_FILE "${PROJECT_NAME}.profraw")

  add_custom_command(
    OUTPUT ${PROFILE_FILE}
    DEPENDS ${PROJECT_NAME}
    COMMAND ${CMAKE_COMMAND} -E rm -f ${PROFILE_FILE}
    COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=${PROFILE_FILE}
            $<TARGET_FILE:${PROJECT_NAME}> || exit 0 # Run first to generate
                                                     # profraw file
  )

  target_coverage(
    ${PROJECT_NAME}
    FORMAT
    ${CODE_COVERAGE_EXT}
    DEPENDS
    ${PROFILE_FILE}
    PROFILE_FILE
    ${PROFILE_FILE})
endif()
