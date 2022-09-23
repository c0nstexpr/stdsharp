set($ENV{CC} "clang")
set($ENV{CXX} "clang++")
set(VCPKG_LINKER_FLAGS "${VCPKG_LINKER_FLAGS} --ld-path=ld.lld")
