from conan import ConanFile


class net_chat(ConanFile):
    name = "net_chat"
    settings = "build_type"

    def requirements(self):
        self.requires("ncurses/6.5")

    generators = "CMakeDeps", "CMakeToolchain"
