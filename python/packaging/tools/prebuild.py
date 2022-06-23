import os

from .cmake import CMake

def build_poco(cmake: CMake = None) -> None:
    """."""
    my_env = os.environ.copy()

    if cmake is None:
        cmake = CMake("ThirdParty/poco/cmake-build")

    cmake.generate()
    cmake.build(my_env)
