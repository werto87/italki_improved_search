from conan import ConanFile


class CompressorRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def configure(self):
        # We can control the options of our dependencies based on current options
        self.options["catch2"].with_main = True
        self.options["catch2"].with_benchmark = True

    def requirements(self):
        self.requires("catch2/2.13.7")
        self.requires("boost/1.83.0", force=True)
        self.requires("confu_json/1.0.0")
        self.requires("certify/cci.20201114")
        self.requires("corrade/2020.06")
