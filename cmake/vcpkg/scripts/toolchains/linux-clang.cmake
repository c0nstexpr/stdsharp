include_guard(GLOBAL)

set(CMAKE_CXX_COMPILER "clang++;-stdlib=libc++")
set(CMAKE_C_COMPILER "clang;-stdlib=libc++")

set(VCPKG_LINKER_FLAGS "${VCPKG_LINKER_FLAGS} --ld-path=ld.lld")

include(${CMAKE_CURRENT_LIST_DIR}/linux.cmake)
