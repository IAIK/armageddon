import logging
import subprocess
import io

logger = logging.getLogger('default')


def execute_command(command):
    proc = subprocess.Popen(command, shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    debug = True
    if debug is True:
        for l in io.TextIOWrapper(proc.stdout, encoding='utf-8'):
            print(l, end="")
        for l in io.TextIOWrapper(proc.stderr, encoding='utf-8'):
            print(l, end="")
    else:
        proc.communicate()
