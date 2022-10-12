set($ENV{CC} "clang")
set($ENV{CXX} "clang++")

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_C_COMPILER "clang")
set(VCPKG_CXX_FLAGS "${VCPKG_CXX_FLAGS} -stdlib=libc++")
set(VCPKG_C_FLAGS "${VCPKG_C_FLAGS}")

set(VCPKG_LINKER_FLAGS "${VCPKG_LINKER_FLAGS} --ld-path=ld.lld")

include(${CMAKE_CURRENT_LIST_DIR}/linux.cmake)
