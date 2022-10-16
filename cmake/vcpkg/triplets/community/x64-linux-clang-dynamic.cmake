set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE
    "${CMAKE_CURRENT_LIST_DIR}/../../scripts/toolchains/linux-clang.cmake")
include(${CMAKE_CURRENT_LIST_DIR}/x64-linux-dynamic.cmake)
