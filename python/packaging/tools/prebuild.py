import os

from .cmake import CMake


def prebuild() -> None:
    """."""
    pre_build_list = [
        "ThirdParty/poco/cmake-build",
        "ThirdParty/pybind11/build",
        "ThirdParty/IoU/release",
    ]

    my_env = os.environ.copy()

    for pb in pre_build_list:
        if cmake is None:
            cmake = CMake(pb)
        cmake.generate()
        cmake.build(my_env)
