# stdsharp

![C++](https://img.shields.io/badge/C%2B%2B-26-blue)
[![Build Status](https://github.com/c0nstexpr/stdsharp/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/c0nstexpr/stdsharp/actions/workflows/build-and-test.yml)
[![Codacy Code Grade](https://app.codacy.com/project/badge/Grade/f08b08ddd5e146c69b39ac5001f06c6a)](https://app.codacy.com/gh/c0nstexpr/stdsharp/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)
[![Codacy Code Coverage](https://app.codacy.com/project/badge/Coverage/f08b08ddd5e146c69b39ac5001f06c6a)](https://app.codacy.com/gh/c0nstexpr/stdsharp/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_coverage)
[![wakatime](https://wakatime.com/badge/github/c0nstexpr/stdsharp.svg)](https://wakatime.com/badge/github/c0nstexpr/stdsharp)

An optimized and supplement library of fundamental features for standard c++. Headers are arranged in accordance with the standard library structure.

<br/>

## Getting Started

### Prerequisites

- **[CMake v3.22+](https://github.com/c0nstexpr/stdsharp/blob/main/CMakeLists.txt#L1)** - required for building

- **C++ Compiler** - needs to support at least the **C++20** and **partial C++23/26** features, i.e. _MSVC_, _GCC_, _Clang_. You could checkout [github workflow file](.github/workflows/build.yml) for suitable compilers.

  - Note that if you're using clang as compiler, [ld.lld](https://lld.llvm.org/) is required. Using [libc++](https://libcxx.llvm.org/) as compiler STL is recommended to ensure compatibility.

- **Vcpkg or Other Suitable Dependencies Manager** - this project uses vcpkg manifest to maintain dependencies. Checkout the
  [vcpkg.json](vcpkg.json) for required dependencies.

<br/>

### Installing

- Clone the project and use cmake to install, or
- Add [my vcpkg registry](https://github.com/c0nstexpr/vcpkg-registry) to your vcpkg-configuration.json.

<br/>

### Building the project

Use cmake to build the project, checkout [github workflow file](.github/workflows/build.yml) for details.

<br/>

## License

This project is licensed under the [Unlicense](https://unlicense.org/) license - see the
[LICENSE](LICENSE) file for details
