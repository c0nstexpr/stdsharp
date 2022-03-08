FROM debian:unstable-slim
RUN apt-get update &&\
    apt-get -y upgrade &&\
    apt-get -y install git cmake ninja-build zip unzip curl clang-13 pkg-config

ENV repo_dir=stdsharp/ CXX=clang++-13 C=clang-13
COPY ./* ${repo_dir}
WORKDIR ${repo_dir}
RUN git clone https://github.com/microsoft/vcpkg.git vcpkg/ &&\
    cd vcpkg/ &&\
    ./bootstrap-vcpkg.sh && ./vcpkg integrate install

RUN cmake -S ./ -DCMAKE_MAKE_PROGRAM=ninja -GNinja -DCMAKE_TOOLCHAIN_FILE="vcpkg/scripts/buildsystems/vcpkg.cmake"

RUN cmake --build

RUN cd build/ &&\
    set CTEST_OUTPUT_ON_FAILURE=1 &&\
    ctest -C Debug

LABEL Name=stdsharp Tag=latest Version=0.0.1