import os

from .cmake import CMake

def build_poco(cmake: CMake = None) -> None:
    """."""
    my_env = os.environ.copy()

    if cmake is None:
        cmake = CMake("ThirdParty/poco/cmake-build")

    cmake.generate()
    cmake.build(my_env)


def build_pybind(cmake: CMake = None) -> None:
    """."""
    my_env = os.environ.copy()

    if cmake is None:
        cmake = CMake("ThirdParty/pybind11/build")

    cmake.generate()
    cmake.build(my_env)


def build_iou(cmake: CMake = None) -> None:
    """."""
    my_env = os.environ.copy()

    if cmake is None:
        cmake = CMake("ThirdParty/IoU/release")

    cmake.generate()
    cmake.build(my_env)
