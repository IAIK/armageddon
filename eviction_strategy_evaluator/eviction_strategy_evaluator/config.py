import yaml
import logging
from collections import Iterable

logger = logging.getLogger('default')


def check_subconfiguration(config, identifier, config_variables):
    invalid_configuration = False

    if identifier not in config:
        invalid_configuration = True
        logger.debug('No "cache" section in device configuration')
    elif not isinstance(config[identifier], Iterable):
        logger.debug('"%s" is no valid section in the device configuration', identifier)
        invalid_configuration = True
    else:
        for config_name, config_type, config_required in config_variables:
            if config_name not in config[identifier]:
                if config_required is True:
                    logger.debug('No "%s" defined in "%s" section of the device configuration', config_name, identifier)
                    invalid_configuration = True
                continue

            if type(config[identifier][config_name]) is not config_type:
                logger.debug('Incorrect type for "%s" defined in the "%s" section of the device configuration', config_name, identifier)
                invalid_configuration = True
                continue

    return invalid_configuration

def parse_configuration(filename):
    try:
        config = yaml.safe_load(open(filename))
    except:
        return None

    invalid_configuration = False

    libflush_config_names = [
        ('source-directory', str, True)
    ]

    invalid_configuration = check_subconfiguration(config, 'libflush', libflush_config_names)

    build_config_names = [
        ('directory', str, True)
    ]

    invalid_configuration = check_subconfiguration(config, 'build', build_config_names)

    logs_config_names = [
        ('directory', str, True)
    ]

    invalid_configuration = check_subconfiguration(config, 'logs', logs_config_names)

    if invalid_configuration is True:
        return None

    return config


def parse_device_configuration(filename):
    try:
        config = yaml.safe_load(open(filename))
    except:
        return None

    invalid_configuration = False

    device_config_names = [
        ('name',                 str, False),
        ('codename',             str, True),
        ('arch',                 str, True),
        ('threshold',            int, False),
        ('adb-id',               str, False),
        ('executable-directory', str, False),
        ('log-directory',        str, False)
    ]

    invalid_configuration = check_subconfiguration(config, 'device', device_config_names)

    cache_config_names = [
        ('number-of-sets', int, True),
        ('line-length',    int, True)
    ]

    invalid_configuration = check_subconfiguration(config, 'cache', cache_config_names)

    if invalid_configuration is True:
        return None

    return config
