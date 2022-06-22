import multiprocessing
import os
import sys

# from .env import build_type


class CMake:
    """Manages cmake."""

    def __init__(self, build_dir: str = "") -> None:
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

    def _run(self, args: List[str]) -> None:
        """Executes cmake with arguments."""

        command = [self._cmake_command] + args
        print(" ".join(command))
        try:
            check_call(command, cwd=self.build_dir)
        except (CalledProcessError, KeyboardInterrupt) as e:
            sys.exit(1)

    def build(self, my_env: Dict[str, str]) -> None:
        """Runs cmake to build binaries."""

        build_args = [
            "--build",
            ".",
            "--target",
            "install",
            "--config",
            # build_type.build_type_string,
        ]

        max_jobs = os.getenv("MAX_JOBS")

        if max_jobs is not None:
            max_jobs = max_jobs or str(multiprocessing.cpu_count())

            build_args += ["--"]
            build_args += ["-j", max_jobs]

        self._run(build_args, my_env)
