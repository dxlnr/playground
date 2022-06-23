import multiprocessing
import os
import sys
import pathlib
from subprocess import check_call, CalledProcessError
from typing import Any, Dict, List, Optional, Union

# from .env import build_type

CMakeValue = Optional[Union[bool, str]]

class CMake:
    """Manages cmake."""

    def __init__(self, build_dir: str = "build") -> None:
        self._cmake_command = CMake._get_cmake_command()
        self.build_dir = build_dir

    @staticmethod
    def _get_cmake_command() -> str:
        """Returns cmake command."""

        cmake_command = "cmake"
        return cmake_command

    @staticmethod
    def _get_version(cmd: Optional[str]) -> Any:
        """Returns cmake version."""
        pass

    @property
    def _cmake_cache_file(self) -> str:
        r"""Returns the path to CMakeCache.txt.
        Returns:
          string: The path to CMakeCache.txt.
        """
        return os.path.join(self.build_dir, "CMakeCache.txt")

    # def get_cmake_cache_variables(self) -> Dict[str, CMakeValue]:
    #     r"""Gets values in CMakeCache.txt into a dictionary.
    #     Returns:
    #       dict: A ``dict`` containing the value of cached CMake variables.
    #     """
    #     with open(self._cmake_cache_file) as f:
    #         return get_cmake_cache_variables_from_file(f)

    def _run(self, args: List[str]) -> None:
        """Executes cmake with arguments."""

        print("build dir: ", self.build_dir)

        command = [self._cmake_command] + args
        print(" ".join(command))
        try:
            check_call(command, cwd=self.build_dir)
        except (CalledProcessError, KeyboardInterrupt) as e:
            sys.exit(1)

    def generate(self) -> None:
        """Runs cmake to generate native build files."""

        build_temp = pathlib.Path(self.build_dir)
        build_temp.mkdir(parents=True, exist_ok=True)
        self.build_dir = build_temp.absolute()

        try:
            os.chdir(str(build_temp))
        except:
            print(sys.exc_info())

        args = []
        args.append("..")

        self._run(args)


    def build(self, my_env: Dict[str, str]) -> None:
        """Runs cmake to build binaries."""

        build_args = [
            "--build",
            ".",
            # "--target",
            # "install",
            "--config",
            "Release"
            # build_type.build_type_string,
        ]

        # max_jobs = os.getenv("MAX_JOBS")
        #
        # if max_jobs is not None:
        #     max_jobs = max_jobs or str(multiprocessing.cpu_count())
        #
        #     build_args += ["--"]
        #     build_args += ["-j", max_jobs]

        self._run(build_args)
