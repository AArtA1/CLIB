# clib

This is a library that implements image and video algorithms which are used in computer vision tasks by using own floating point representations.

## Preset

### Before start 

Add the libraries by using the command line script below:

'''
sudo apt-get install cmake gcc libavformat-dev ffmpeg libx11-dev doxygen libboost-all-dev libpng-dev libjpeg-dev libx11-dev ffmpeg 
'''

Contents
--------

1.  [Build](#build)
    1.  [Generate a build system](#generate-a-build-system)
    2.  [Build a project](#build-a-project)
2.  [Options](#options)
    1.  [CLIB_COVERAGE](#CLIB_COVERAGE)
    2.  [CLIB_TESTING](#CLIB_TESTING)
    3.  [DOXYGEN_OUTPUT_LANGUAGE](#DOXYGEN_OUTPUT_LANGUAGE)
3.  [Targets](#targets)
    1.  [Default](#default)
    2.  [clib_library](#clib_library)
    3.  [clib-unit-tests](#clib-unit-tests)
    4.  [check](#check)
    5.  [coverage](#coverage)
    6.  [doc](#doc)
4.  [Examples](#examples)
5.  [Usage](#usage)
    1.  [Through the installation](#through-the-installation)
    2.  [As a submodule](#as-a-submodule)
6.  [Tools](#tools)
7.  [Bonus](#bonus)

Build
-----

Building this project, like any other CMake project, consists of two stages:

### Generate a build system

```shell
cmake -S path/to/sources -B path/to/build/directory [options ...]
```

[More about options](#options).

### Build a project

```shell
cmake --build path/to/build/directory [--target target]
```

[More about targets](#targets).

Options
-------

### CLIB_COVERAGE

```shell
cmake -S ... -B ... -DCLIB_COVERAGE=ON [other options ...]
```

Turns on the [`coverage`](#coverage) target which performs code coverage measurement.

### CLIB_TESTING

```shell
cmake -S ... -B ... -DCLIB_TESTING=OFF [other options ...]
```

Provides the ability to turn off unit testing and hence the [`check`](#check) target. As a result, the code coverage measurement is also turned off (see [Code coverage](#CLIB_COVERAGE)).

Also, testing is automatically disabled if the project is included to another project as a subproject using the [`add_subdirectory`](https://cmake.org/cmake/help/v3.14/command/add_subdirectory.html) command.

### DOXYGEN_OUTPUT_LANGUAGE

```shell
cmake -S ... -B ... -DDOXYGEN_OUTPUT_LANGUAGE=English [other options ...]
```

Switches the language of the documentation generated by the [`doc`](#doc) target. For a list of available languages, see [Doxygen site](http://www.doxygen.nl/manual/config.html#cfg_output_language).

Default language is Russian.

Targets
-------

### Default

```shell
cmake --build path/to/build/directory
cmake --build path/to/build/directory --target all
```

If a target is not specified (which is equivalent to the `all` target), it builds everything possible including unit tests and also calls the [`check`](#check) target.

### clib_library

```shell
cmake --build path/to/build/directory --target clib_library
```

Compiles the `clib_library` library. Enabled by default.

### clib-unit-tests

```shell
cmake --build path/to/build/directory --target clib-unit-tests
```

Builds unit tests. Enabled by default.

### check

```shell
cmake --build path/to/build/directory --target check
```

Launches built (and builds if not yet) unit tests. Enabled by default.

See also [`clib-unit-tests`](#clib-unit-tests).

### coverage

```shell
cmake --build path/to/build/directory --target coverage
```

Analyzes run unit tests (and runs is not yet) using [gcovr](https://gcovr.com).

Target is only available if [`CLIB_COVERAGE`](#CLIB_COVERAGE) option is on.

See also [`check`](#check).

### doc

!Doxygen must be installed 

```shell
cmake --build path/to/build/directory --target doc
```

Generates source code documentation using [Doxygen](http://doxygen.nl).


### Cppcheck

CMake has build-in support for [Cppcheck](http://cppcheck.sourceforge.net).

It is provided by the [`CMAKE_CXX_CPPCHECK`](https://cmake.org/cmake/help/v3.14/variable/CMAKE_LANG_CPPCHECK.html#variable:CMAKE_<LANG>_CPPCHECK) option:

```shell
cmake -S path/to/sources -B path/to/build/directory -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_CPPCHECK="cppcheck;--enable=all;-Ipath/to/sources/include"
```

After that, static analysis will be automatically run every time the source code is compiled and recompiled.


Examples
--------

#### Building the project in the debug mode and measure a coverage

```shell
cmake -S path/to/sources -B path/to/build/directory -DCMAKE_BUILD_TYPE=Debug -DCLIB_COVERAGE=ON
cmake --build path/to/build/directory --target coverage --parallel 16
```

#### Installing the project without building and testing it

```shell
cmake -S path/to/sources -B path/to/build/directory -DCLIB_TESTING=OFF -DCMAKE_INSTALL_PREFIX=path/to/install/directory
cmake --build path/to/build/directory --target install
```

#### Building the project in the release mode with a specified compiler

```shell
cmake -S path/to/sources -B path/to/build/directory -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++-8 -DCMAKE_PREFIX_PATH=path/to/installed/dependencies
cmake --build path/to/build/directory --parallel 4
```

#### Generating the documentation in English

```shell
cmake -S path/to/sources -B path/to/build/directory -DCMAKE_BUILD_TYPE=Release -DDOXYGEN_OUTPUT_LANGUAGE=English
cmake --build path/to/build/directory --target doc
```

Usage
-----

### Through the installation

One of the ways to use the module is to install it into the system.

```shell
cmake --build path/to/build/directory --target install
```

After that, all the libraries from the `clib::` namespace can be used from any other project using the [`find_package`](https://cmake.org/cmake/help/v3.14/command/find_package.html) command:

```cmake
find_package(clib 1.0 REQUIRED)

add_executable(some_executable some.cpp sources.cpp)
target_link_libraries(some_executable PRIVATE clib::library)
```

`clib::headers` library is used for the headers only, and `clib::library` library is used when it is also needed to link with the `libclib_library` library.

### As a submodule

The project can also be used by another project as a submodule using the [`add_subdirectory`](https://cmake.org/cmake/help/v3.14/command/add_subdirectory.html) command:

In this case, libraries `clib::library` and `clib::headers` will be available in the same manner.

Tools
-----

1.  [CMake](https://cmake.org) 3.14

    CMake 3.14 is required because of incorrect work of the command `install(TARGETS ... EXPORT ...)`: is does not set default install paths properly.

2.  [doctest](https://github.com/onqtam/doctest) testing framework

    Testing might be turned off (see [Testing](#CLIB_TESTING)).

3.  [Doxygen](http://doxygen.nl)

    Switching the language of the generated documentation is provided by the [`DOXYGEN_OUTPUT_LANGUAGE`](#DOXYGEN_OUTPUT_LANGUAGE) option.

