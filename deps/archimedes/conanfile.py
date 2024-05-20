from conan import ConanFile
from conan.tools.microsoft.visual import is_msvc


class Archimedes(ConanFile):
    name = "Archimedes"
    settings = "build_type"

    def requirements(self):
        self.requires("spdlog/1.12.0")
        self.requires("gtest/1.13.0")

    def configure(self):
        if is_msvc(self, True):
            self.options["spdlog/1.12.0"].use_std_fmt = True

    generators = "CMakeDeps", "CMakeToolchain"
