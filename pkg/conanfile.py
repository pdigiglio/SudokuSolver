from conans import ConanFile, CMake, tools


class SudokuSolverConan(ConanFile):
    name = "sudoku_solver"
    version = "0.0.0"
    license = ""
    author = "Paolo Di Giglio <p.digiglio91@gmail.com>"
    url = ""
    description = "A library to solve 9x9 sudoku games"
    topics = ("")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    generators = "cmake"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def source(self):
        git = tools.Git(folder="SudokuSolver")
        git.clone("https://github.com/pdigiglio/SudokuSolver.git", "master")

#        # This small hack might be useful to guarantee proper /MT /MD linkage
#        # in MSVC if the packaged project doesn't have variables to set it
#        # properly
#        tools.replace_in_file("hello/CMakeLists.txt", "PROJECT(HelloWorld)",
#                              '''PROJECT(HelloWorld)
#include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
#conan_basic_setup()''')

    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder="SudokuSolver")
        cmake.build(target="SudokuSolverLib")

        # Explicit way:
        # self.run('cmake %s/hello %s'
        #          % (self.source_folder, cmake.command_line))
        # self.run("cmake --build . %s" % cmake.build_config)

    def package(self):
        self.copy("*.h", dst="include", src="SudokuSolver")
        self.copy("*SudokuSolver.lib", dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.dylib", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["SudokuSolverLib"]

