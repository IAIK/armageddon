import logging
import os

from .utils import execute_command

logger = logging.getLogger('default')


class Executor(object):
    def __init__(self, configuration, strategy, builder):
        self.configuration = configuration
        self.strategy = strategy
        self.builder = builder
        self.device_configuration = self.strategy.device_configuration

    def run(self, number_of_runs, force):
        local_executable = self.builder.get_executable_path()

        # create logdir
        device_codename = self.strategy.device_configuration['device']['codename']
        log_dir = os.path.join(self.configuration['logs']['directory'], device_codename)
        if not os.path.exists(log_dir):
            os.makedirs(log_dir)
        local_logfile = os.path.join(log_dir, self.strategy.get_name() + ".log")

        # check if log file exists
        if os.path.exists(local_logfile) and not force:
            logger.info('Strategy logfile already exists.')
            return False

        if 'adb-id' in self.device_configuration['device']:
            return self.__run_android(local_executable, local_logfile, number_of_runs)
        else:
            return self.__run_local(local_executable, local_logfile, number_of_runs)

    def __run_android(self, local_executable, local_logfile, number_of_runs):
        remote_executeable_dir = self.device_configuration['device']['executable-directory']
        remote_executable = os.path.join(remote_executeable_dir, self.strategy.get_name())
        remote_logfile = os.path.join(remote_executeable_dir, self.strategy.get_name() + ".log")
        adb_id = self.device_configuration['device']['adb-id']

        # Upload
        logger.info("Uploading executable")
        execute_command([
            "adb",
            "-s", adb_id,
            "push",
            local_executable,
            remote_executable
        ])

        # Setting chmod
        logger.info("Setting change mode")
        execute_command([
            "adb",
            "-s", adb_id,
            "shell",
            "su",
            "-c",
            "'",
            "chmod",
            "777",
            remote_executable,
            "'"
        ])

        # Running
        logger.info("Running measurements")
        execute_command([
            "adb",
            "-s", adb_id,
            "shell",
            "su",
            "-c",
            "'",
            remote_executable,
            "-n", str(number_of_runs),
            "-c", "0",
            remote_logfile,
            "'"
        ])

        execute_command([
            "adb",
            "-s", adb_id,
            "shell",
            "su",
            "-c",
            "'",
            "chmod",
            "777",
            remote_logfile,
            "'"
        ])

        # Running
        logger.info("Fetching results")
        execute_command([
            "adb",
            "-s", adb_id,
            "pull",
            remote_logfile,
            local_logfile
        ])

        # Clean-up
        logger.info("Cleaning up")
        execute_command([
            "adb",
            "-s", adb_id,
            "shell",
            "su",
            "-c",
            "'",
            "rm",
            remote_executable,
            "'"
        ])

        execute_command([
            "adb",
            "-s", adb_id,
            "shell",
            "su",
            "-c",
            "'",
            "rm",
            remote_logfile,
            "'"
        ])

        return True

    def __run_local(self, local_executable, local_logfile, number_of_runs):
        execute_command([
            local_executable,
            "-n", str(number_of_runs),
            local_logfile
        ])

        return True
