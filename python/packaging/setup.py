import os
import platform
import sys
import pathlib

from setuptools import setup, Extension, find_packages
from collections import defaultdict
import setuptools.command.build_ext
import setuptools.command.install
import setuptools.command.sdist

from tools.cmake import CMake
from tools.prebuild import build_poco

try:
    from wheel.bdist_wheel import bdist_wheel
except ImportError:
    # This is useful when wheel is not installed and bdist_wheel is not
    # specified on the command line. If it _is_ specified, parsing the command
    # line will fail before wheel_concatenate is needed
    wheel_concatenate = None


class sdist(setuptools.command.sdist.sdist):
    def run(self):
        with concat_license_files():
            super().run()


class install(setuptools.command.install.install):
    def run(self):
        super().run()


cwd = os.path.dirname(os.path.abspath(__file__))

# the list of runtime dependencies required by this built package
install_requires = [
    "requests",
]

cmake = CMake()

def build_deps():
    """Pre-build dependencies before packaging everything up."""

    build_poco()


def get_third_parties():
    """Collect all the third party modules."""
    third_party_path = os.path.join(cwd, "ThirdParty")

    default_modules_path = [os.path.join(third_party_path, name) for name in ["poco"]]


class CMakeExtension(Extension):
    def __init__(self, name):
        # don't invoke the original build_ext for this special extension
        super().__init__(name, sources=[])


class build_ext(setuptools.command.build_ext.build_ext):
    def run(self):
        for ext in self.extensions:
            self.build_cmake(ext)
        super().run()

    def build_cmake(self, ext):

        # these dirs will be created in build_py, so if you don't have
        # any python sources to bundle, the dirs will be missing
        build_temp = pathlib.Path(self.build_temp)
        build_temp.mkdir(parents=True, exist_ok=True)

        extdir = pathlib.Path(self.get_ext_fullpath(ext.name))
        extdir.mkdir(parents=True, exist_ok=True)

        print("EXXXT-DIR: ", extdir)

        # example of cmake args
        config = "Debug" if self.debug else "Release"
        cmake_args = [
            "-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=" + str(extdir.parent.absolute()),
            "-DCMAKE_BUILD_TYPE=" + config,
        ]

        # example of build args
        build_args = ["--config", config, "--", "-j4"]

        os.chdir(str(build_temp))
        self.spawn(["cmake", str(cwd)] + cmake_args)
        if not self.dry_run:
            self.spawn(["cmake", "--build", "."] + build_args)
        # Troubleshooting: if fail on line above then delete all possible
        # temporary CMake files including "CMakeCache.txt" in top level dir.
        os.chdir(str(cwd))


def configure_extension_build():
    r"""Configures extension build options according to system environment and user input.

    Returns:
      The input to parameters ext_modules, cmdclass, packages, and entry_points as required in setuptools.setup.
    """
    # try:
    #     cmake_cache_vars = defaultdict(lambda: False, cmake.get_cmake_cache_variables())
    # except FileNotFoundError:
    #     cmake_cache_vars = defaultdict(lambda: False)

    library_dirs = []
    extra_install_requires = []

    # Set extensions and package.
    extensions = []
    extensions.append(CMakeExtension("src/foo"))

    packages = find_packages(exclude=("tools", "tools.*"))

    cmdclass = {
        "bdist_wheel": bdist_wheel,
        "build_ext": build_ext,
        "install": install,
        # "sdist": sdist,
    }

    return extensions, cmdclass, packages, extra_install_requires


if __name__ == "__main__":
    # Precompile dependencies.
    build_deps()

    extensions, cmdclass, packages, extra_install_requires = configure_extension_build()

    install_requires += extra_install_requires

    setup(
        name="py",
        version="0.1",
        ext_modules=extensions,
        cmdclass=cmdclass,
        packages=packages,
        install_requires=install_requires,
    )
