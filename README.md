# cpp_host

This repository contains the `generic_host` C++ library and example projects.

The code in this project is available under the terms of the [MIT License](LICENSE).

## Bootstrapping

Run `./bootstrap.sh` (or `bootstrap.ps1` on Windows) to install required tools and configure the build. The scripts ensure Python and Conan are available, bootstrap vcpkg, and then compile the project.

## Running the tests

The CMake build does not enable tests by default. To build and run them, configure the project with `GH_BUILD_TESTS` set to `ON`:

```bash
cmake -S . -B build -DGH_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

The `catch_discover_tests` helper in `CMakeLists.txt` registers each Catch2 test case individually so `ctest` will list them all.


## Conan package

A `conanfile.py` is provided to build and package the library. To create a package run:

```bash
conan create . --output-folder=conan_build
```

This will build the library and export a Conan package that can be uploaded to your preferred remote.

## Releases

Versions are generated automatically using [GitVersion](https://gitversion.net/) and
release notes are created from Conventional Commits. A GitHub Actions workflow
builds the project, packages the artifacts with CPack and publishes a release
whenever a tag matching `v*.*.*` is pushed.

See [CHANGELOG.md](CHANGELOG.md) for a history of changes.
