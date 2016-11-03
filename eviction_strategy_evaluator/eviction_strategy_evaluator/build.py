import logging
import os
from .utils import execute_command

logger = logging.getLogger('default')


class Builder(object):
    def __init__(self, configuration, strategy):
        self.configuration = configuration
        self.strategy = strategy

        # create build directory
        device_codename = self.strategy.device_configuration['device']['codename']
        self.build_dir = os.path.join(configuration['build']['directory'], device_codename, self.strategy.get_name())

        if not os.path.exists(self.build_dir):
            os.makedirs(self.build_dir)

        # create strategy file
        self.strategy_file = os.path.join(self.build_dir, "strategy.h")
        self.strategy.save_strategy_to_file(self.strategy_file)

    def build(self, force):
        if os.path.exists(self.get_executable_path()) and not force:
            logger.info('Executable has already been build.')
            return False

        self.build_libflush()
        self.build_executable()

        return True

    def build_libflush(self):
        logger.info("Building libflush...")

        # compile libflush
        libflush_srcdir = self.configuration['libflush']['source-directory']
        architecture = self.strategy.device_configuration['device']['arch']

        libflush_build_dir = os.path.join(self.build_dir, "libflush")
        if not os.path.exists(libflush_build_dir):
            os.makedirs(libflush_build_dir)

        libflush_depend_dir = os.path.join(self.build_dir, ".depend")
        if not os.path.exists(libflush_depend_dir):
            os.makedirs(libflush_depend_dir)

        execute_command([
            "make",
            "-C",
            libflush_srcdir,
            "ARCH=" + architecture,
            "BUILDDIR=" + libflush_build_dir,
            "DEPENDDIR=" + libflush_depend_dir,
            "DEVICE_CONFIGURATION=" + self.strategy_file,
            "USE_EVICTION=1",
            "clean"
        ])

        execute_command([
            "make",
            "-C",
            libflush_srcdir,
            "ARCH=" + architecture,
            "BUILDDIR=" + libflush_build_dir,
            "DEPENDDIR=" + libflush_depend_dir,
            "DEVICE_CONFIGURATION=" + self.strategy_file,
            "USE_EVICTION=1"
        ])

    def build_executable(self):
        logger.info("Building executable...")

        current_path = os.path.dirname(os.path.abspath(__file__))
        executable_srcdir = os.path.join(current_path, "source")
        libflush_srcdir = self.configuration['libflush']['source-directory']
        architecture = self.strategy.device_configuration['device']['arch']
        libflush_build_dir = os.path.join(self.build_dir, "libflush")

        executable_build_dir = os.path.join(self.build_dir, "executable")
        if not os.path.exists(executable_build_dir):
            os.makedirs(executable_build_dir)

        executable_depend_dir = os.path.join(self.build_dir, ".depend")
        if not os.path.exists(executable_depend_dir):
            os.makedirs(executable_depend_dir)

        execute_command([
            "make",
            "-C",
            executable_srcdir,
            "LIBFLUSH_SOURCE=" + libflush_srcdir,
            "LIBFLUSH_INC=" + libflush_srcdir,
            "LIBFLUSH_BUILDDIR=" + libflush_build_dir,
            "DEPENDDIR=" + executable_depend_dir,
            "ARCH=" + architecture,
            "BUILDDIR=" + executable_build_dir,
            "clean"
        ])

        execute_command([
            "make",
            "-C",
            executable_srcdir,
            "LIBFLUSH_SOURCE=" + libflush_srcdir,
            "LIBFLUSH_INC=" + libflush_srcdir,
            "LIBFLUSH_BUILDDIR=" + libflush_build_dir,
            "DEPENDDIR=" + executable_depend_dir,
            "ARCH=" + architecture,
            "BUILDDIR=" + executable_build_dir
        ])

    def get_executable_path(self):
        executable_build_dir = os.path.join(self.build_dir, "executable")
        executable_path = os.path.join(executable_build_dir, "release/bin/executable")

        return executable_path
