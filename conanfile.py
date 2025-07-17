from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout

class GenericHostConan(ConanFile):
    name = "generic_host"
    version = "0.1.0"
    license = "MIT"
    url = "https://github.com/example/cpp_host"
    description = "Cross-platform .NET-style hosting abstraction for C++"
    topics = ("host", "cpp")

    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    exports_sources = "CMakeLists.txt", "src/*", "include/*"

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        self.requires("boost/1.85.0")
        self.requires("spdlog/1.15.3")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["generic_host"]


