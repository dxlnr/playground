import os
import platform
import sys
import pathlib

from setuptools import setup, Extension, find_packages
import setuptools.command.build_ext

cwd = os.path.dirname(os.path.abspath(__file__))

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
        print(build_temp)
        # extdir = pathlib.Path(self.get_ext_fullpath(ext.name))
        # extdir.mkdir(parents=True, exist_ok=True)
        #
        # # example of cmake args
        # config = 'Debug' if self.debug else 'Release'
        # cmake_args = [
        #     '-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + str(extdir.parent.absolute()),
        #     '-DCMAKE_BUILD_TYPE=' + config
        # ]
        #
        # # example of build args
        # build_args = [
        #     '--config', config,
        #     '--', '-j4'
        # ]
        #
        # os.chdir(str(build_temp))
        # self.spawn(['cmake', str(cwd)] + cmake_args)
        # if not self.dry_run:
        #     self.spawn(['cmake', '--build', '.'] + build_args)
        # # Troubleshooting: if fail on line above then delete all possible
        # # temporary CMake files including "CMakeCache.txt" in top level dir.
        # os.chdir(str(cwd))


if __name__ == '__main__':
    setup(
        name='py',
        version='0.1',
        packages=['py'],
        ext_modules=[CMakeExtension('src/foo')],
        cmdclass={
            'build_ext': build_ext,
        }
    )
