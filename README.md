# Generic Host for C++

This project provides a small library that mimics the .NET Generic Host pattern for C++. It allows applications to configure and start background services in a consistent manner across platforms.

## Features

- Host builder API similar to `.NET`'s hosting model
- Example application demonstrating usage
- Catch2 tests for core functionality
- Dependency management through [vcpkg](https://github.com/microsoft/vcpkg)

## Building

The easiest way to build the project is to use the provided `bootstrap` scripts which fetch `vcpkg` and configure CMake:

```bash
./bootstrap.sh
```

On Windows use the PowerShell variant:

```powershell
./bootstrap.ps1
```

This will configure and build the library and example into the `build/` directory.

## Running tests

After building, tests can be run with CTest:

```bash
cd build
ctest
```

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
