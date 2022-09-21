include(${CMAKE_CURRENT_LIST_DIR}/../x64-linux.cmake)

set(VCPKG_CXX_FLAGS "${VCPKG_CXX_FLAGS} -stdlib=libc++")
set(VCPKG_C_FLAGS "${VCPKG_C_FLAGS}")
