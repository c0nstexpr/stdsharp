FROM maranov/clang-git
RUN apk update && apk upgrade && apk --no-cache add build-base cmake ninja zip unzip curl

ENV repo_dir=/home/stdsharp
COPY ./* ${repo_dir}
WORKDIR ${repo_dir}

RUN git clone https://github.com/microsoft/vcpkg.git vcpkg/
RUN vcpkg/bootstrap-vcpkg.sh
RUN vcpkg/vcpkg integrate install

RUN cmake -S ./ -DCMAKE_MAKE_PROGRAM=ninja -GNinja -DCMAKE_TOOLCHAIN_FILE="vcpkg/scripts/buildsystems/vcpkg.cmake" -DCMAKE_CXX_COMPILER=clang++-12

RUN cmake --build

LABEL Name=stdsharp Tag=latest Version=0.0.1