# stdsharp

[![](https://github.com/BlurringShadow/stdsharp/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/BlurringShadow/stdsharp/actions/workflows/build-and-test.yml)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/f08b08ddd5e146c69b39ac5001f06c6a)](https://www.codacy.com/gh/BlurringShadow/stdsharp/dashboard?utm_source=github.com&utm_medium=referral&utm_content=BlurringShadow/stdsharp&utm_campaign=Badge_Grade)
[![Codacy Badge](https://app.codacy.com/project/badge/Coverage/f08b08ddd5e146c69b39ac5001f06c6a)](https://www.codacy.com/gh/BlurringShadow/stdsharp/dashboard?utm_source=github.com&utm_medium=referral&utm_content=BlurringShadow/stdsharp&utm_campaign=Badge_Coverage)
[![wakatime](https://wakatime.com/badge/github/BlurringShadow/stdsharp.svg)](https://wakatime.com/badge/github/BlurringShadow/stdsharp)

An optimized and supplement library of fundamental features for standard c++. Headers are arranged in accordance with the standard library structure.

<br/>

## Getting Started

### Prerequisites

- **[CMake v3.22+](https://github.com/BlurringShadow/stdsharp/blob/main/CMakeLists.txt#L1)** - required for building

- **C++ Compiler** - needs to support at least the **C++20** and **partial C++23** standard, i.e. _MSVC_, _GCC_, _Clang_. You could checkout [github workflow file](.github/workflows/build.yml) for suitable compilers.

- **Vcpkg or Other Suitable Dependencies Manager** - this project uses vcpkg manifest to maintain dependencies. Checkout the
  [vcpkg.json](vcpkg.json) for required dependencies.

<br/>

### Installing

- Clone the project and use cmake to install, or
- Add [my vcpkg registry](https://github.com/BlurringShadow/vcpkg-registry) to your vcpkg-configuration.json.

<br/>

### Building the project

Use cmake to build the project, checkout [github workflow file](.github/workflows/build.yml) for details.

<br/>

## License

This project is licensed under the [Unlicense](https://unlicense.org/) license - see the
[LICENSE](LICENSE) file for details
