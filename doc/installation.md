# Installation

This reference page gives installation instructions for all methods in all supported environments.

[&larr; back to Reference](reference.md)


## Contents

* [Supported platforms](#supported-platforms)
* [Source code compilation](#source-code-compilation)
  * [Advanced build option](#advanced-build-option)
* [Standalone binary](#standalone-binary)
* [Packages](#packages)


## Supported platforms

|                                                                      | Linux                   | macOS (Intel, Apple Silicon)                                                            | Windows                                  |
|----------------------------------------------------------------------|-------------------------|-----------------------------------------------------------------------------------------|------------------------------------------|
| [Source code compilation](#source-code-compilation)                  | - GCC 7+<br/>- Clang 7+ | Apple:<br/>- Clang 7+ (**no multithreading**)<br/>Homebrew:<br/>- GCC 7+<br/>- Clang 7+ | Cygwin/WSL:<br/>- GCC 7+<br/>- Clang 7+  |
| [Standalone binary](#standalone-binary)<br />(**no tab completion**) | &#9989;                 | &#10060;                                                                                | &#10060;                                 |
| [Packages](#packages)                                                | DEB/RMP                 | &#10060;                                                                                | &#10060;                                 |


## Source code compilation

Source code compilation is a recommended method of RAMPACK installation. It enables you to utilize native CPU
optimizations, which usually increase the speed by around 10%. To build the project, you need the following
prerequisites:

- git
- cmake 3.10+
- gcc 7+ / clang 7+

All other dependencies are contained within the git repository as submodules. The compilation was tested on:

- Linux
- macOS (Intel, Apple Silicon)
- Windows (Cygwin, WSL)


The project should be cloned with
`--recurse-submodules` option

```shell
git clone https://github.com/PKua007/rampack.git --recurse-submodules
```

or, if the project was already cloned without it, one should execute

```shell
git submodule update --init
```

Then issuing the following commands with install the program with tab completion scripts for `bash` and `zsh` shells

```shell
mkdir build
cd build
cmake ../
cmake --build .
sudo cmake --build . --target install
```

`cmake ../` prepares the build. It accepts [additional options](#advanced-build-option) to customize the build.
Autocompletion scripts will be installed only if the corresponding shell is detected. You can verify the installation by
executing

```shell
rampack --version
```


### Advanced build option
There are also project-specific CMake options to customize the build:

* ***-DCMAKE_C_COMPILER=...*** (*= as detected by CMake*)<br />
  ***-DCMAKE_CXX_COMPILER=...*** (*= as detected by CMake*)

  Provides manual paths for C and C++ compilers. Useful when many compilers are available in the system.

* ***-DCMAKE_INSTALL_PREFIX=...*** (*= /usr/local*)

  Changes the default installation prefix. If it is set to `/prefix/path`:
  * `rampack` binary will be installed to `/prefix/path/bin`
  * `bash` completion will be installed to `/prefix/path/share/bash-completion/completions`
  * `zsh` completion will be installed to `/prefix/path/share/zsh/site-functions`

* ***-DRAMPACK_STATIC_LINKING=ON/OFF*** (*= OFF*)

  Turns on/off static linking. Static-linked binaries have minimal or no dependencies and should be able to be run on
  all versions of the same platform (unavailable on macOS).

* ***-DRAMPACK_BUILD_TESTS*** (*= OFF*)

  Turns on/off building unit and validation tests. The tests can be executed using `ctest` command in the build folder.

* ***-DRAMPACK_NATIVE_ARCH*** (*= ON*)

  Turns on/off native CPU optimizations `-march=native` (unavailable in Apple Clang on Apple Silicon).


## Standalone binary

Standalone binary is available for all Linux platforms. It is static-linked, which means that it has no dependencies and
should run on all reasonably modern versions of various Linux distributions. `zsh` and `bash` tab completion is not
available.


## Packages

The project is also distributed as DEB (Debian/Ubuntu/...) and RPM (Red Hat/Fedora/SUSE/...) packages. The packages
contain static-linked binary as well as tab completion scripts. Installation:

```shell
dpkg -i rampack-[version]-Linux.deb
```

```shell
rpm -i rampack-[version]-Linux.rpm
```


[&uarr; back to the top](#installation)