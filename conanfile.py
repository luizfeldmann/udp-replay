from conan import ConanFile
from conan.tools.files import load
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps

class Recipe(ConanFile):
    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"

    def requirements(self):
        self.requires("cxxopts/3.3.1")
        self.requires("pcapplusplus/25.05")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()

        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
