project(${CMAKE_PROJECT_NAME}Tests LANGUAGES CXX)

include(CTest)
find_package(Catch2 CONFIG REQUIRED)
include(Catch)

add_exe_target(
    ${PROJECT_NAME}
    SRC
    src/algorithm/algorithm.cpp
    src/bitset/bitset.cpp
    src/containers/actions.cpp
    src/containers/concepts.cpp
    src/filesystem/space_size.cpp
    src/functional/bind.cpp
    src/functional/invocables.cpp
    src/functional/pipeable.cpp
    src/functional/sequenced_invocables.cpp
    src/memory/allocation_adaptor.cpp
    src/memory/allocation_traits.cpp
    src/memory/box.cpp
    src/memory/composed_allocator.cpp
    src/memory/fixed_single_allocator.cpp
    src/memory/launder_iterator.cpp
    src/memory/pointer_traits.cpp
    src/memory/soo.cpp
    src/random/random.cpp
    src/type_traits/indexed_traits.cpp
    src/type_traits/int_type_1.cpp
    src/type_traits/int_type_2.cpp
    src/type_traits/object.cpp
    src/type_traits/type_sequence.cpp
    src/type_traits/type.cpp
    src/type_traits/value_sequence.cpp
    src/utility/adl_proof.cpp
    src/utility/dispatcher.cpp
    src/utility/fwd_cast.cpp
    src/utility/value_wrapper.cpp
    src/default_operator.cpp
    src/lazy.cpp
    src/pattern_match.cpp
    src/synchronizer.cpp
    LINKS stdsharp Catch2::Catch2WithMain
    INCLUDED_AS_SYSTEM FALSE
)
catch_discover_tests(${PROJECT_NAME})

target_diagnostics(${PROJECT_NAME})
target_compile_options(${PROJECT_NAME} PRIVATE $<$<CXX_COMPILER_ID:Clang>: -ftemplate-backtrace-limit=0>)
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_WARNING_AS_ERROR ON)

target_clang_sanitizer(${PROJECT_NAME} PRIVATE SANITIZER address undefined leak thread memory type)
target_msvc_sanitizer(${PROJECT_NAME} PRIVATE SANITIZER address kernel-address)

target_clang_tidy(${PROJECT_NAME})
target_llvm_coverage(${PROJECT_NAME})
