# Eviction Strategy Evaluator

Eviction Strategy Evaluator is a tool that utilizes [libflush](../libflush) to
find an effective and fast eviction strategy for any device.

It has been used for our [ARMageddon: Cache Attacks on Mobile Devices](https://www.usenix.org/conference/usenixsecurity16/technical-sessions/presentation/lipp) paper by Lipp, Gruss, Spreitzer, Maurice and Mangard that has been published at the Usenix Security Symposium 2016 and presented at [Black Hat Europe 2016](https://www.blackhat.com/eu-16/briefings/schedule/index.html#armageddon-how-your-smartphone-cpu-breaks-software-level-security-and-privacy-4887).

## Table of content

- [Installation](#installation)
    - [Dependencies](#dependencies)
- [Usage](#usage)
- [Configuration](#configuration)
    - [Global configuration](#global-configuration)
    - [Device configuration](#device-configuration)
- [Example](#example)
- [License](#license)
- [References](#references)

## Installation

The tool is shipped with a setup.py and can be compiled by running:
```bash
python setup.py install
```

### Dependencies
Eviction Strategy Evaluator builds the source code of [libflush](../libflush)
and, thus, the dependencies of [libflush](../libflush) need to be installed on
the host system. In addition, it requires the following dependencies:

* [click](http://click.pocoo.org) (required)
* [PyYAML](http://pyyaml.org) (required)
* [pandas](http://pandas.pydata.org) (required)
* [adb](https://developer.android.com/studio/command-line/adb.html) (to
    communicate with Android devices)

## Usage

```bash
eviction_strategy_evaluator [OPTIONS] <command> [COMMAND_OPTIONS]
```

The following options are available:

* **-c, --configuration-file** (required)

    Path to the used global configuration file.

* **-x, --device-configuration-file** (required)

    Path to the used device configuration file.

* **-v, --verbose**

    Verbose mode

* **-f, --force**

    Force mode to override files and re-compile existing binaries.

* **--help**

    Show the help information.

The following commands are available:

* **evaluate_strategy**

    Evaluates a single log file LOGFILE using the given
    threshold.

    The following options are available:

    * **-t, --threshold** (required)

      The threshold that is used to distinguish between a cache hit and a cache
      miss.

* **evaluate_strategies** [OPTIONS] LOGFILE_DIRECTORY

    Evaluates all log files in the given LOGFILE_DIRECTORY using the given
    threshold.

    The following options are available:

    * **-t, --threshold** (required)

      The threshold that is used to distinguish between a cache hit and a cache
      miss.

* **run_strategy**

    Compiles an eviction strategy for the target devices and executes it. Then
    it will pull the log file.

    The following options are available:

    * **-n, --number-of-measurements** 

      The number of measurements that should be taken.

    * **-e, --eviction-counter**  (required)

      The number of loop executions.

    * **-a, --number-of-accesses-in-loop**  (required)

      The number of accesses in a loop.

    * **-d, --different-addresses-in-loop**  (required)

      The number of accesses to different addresses in a loop.

    * **-s, --step-size** 

      The loop increment.

    * **-m, --mirroring** 

      If the eviction strategy should be mirrored.

* **run_strategies**

    Compile and run multiple eviction strategies for the target devices and executes them. Then
    it will pull the log files.

    The following options are available:

    * **-n, --number-of-measurements** 

      The number of measurements that should be taken.

    * **-e, --max-eviction-counter**  (required)

      The maximum number of loop executions.

    * **-a, --max-number-of-accesses-in-loop**  (required)

      The maximum number of accesses in a loop.

    * **-d, --max-different-addresses-in-loop**  (required)

      The maximum number of accesses to different addresses in a loop.

    * **-s, --max-step-size** 

      The maximum loop increment.

    * **-m, --with-mirroring** 

      If the eviction strategies should be also tested with mirroring.

## Configuration

In order to use the eviction strategy evaluator you need two different
configuration files. One global configuration file specifying the location of
the source of [libflush](../libflush) as well as the directories used for
storing the executables and logfiles. In addition, you need a device
configuration file that specifies several properties of the target device.

### Global configuration

The global configuration file must provide the following entries:

```yml
libflush:
  source-directory: /path/to/libflush
build:
  directory: /path/to/builds
logs:
  directory: /path/to/logs
```

### Device configuration

The device configuration file must provide the following entries:

```yml
device:
  name: Alcatel POP2 Touch
  codename: alto45
  arch: armv7
  adb-id: e7e3ca47
  executable-directory: /data/local/tmp
  log-directory: /data/local/tmp
  threshold: 95
cache:
  number-of-sets: 512
  line-length: 64
```

## Example

After you have created your configuration files you can evaluate different
strategies on it:

    eviction_strategy_evaluator -c config.yml -x alto45.yml run_strategies -e 25 -a 10 -d 10

Then you can evaluate them:

    eviction_strategy_evaluator -c config.yml -x alto45.yml evaluate_strategies /tmp/logs/alto45 -t 95

You will end up with a ``strategies.db`` as well as a ``strategies.csv`` file.

## License

[Licensed](LICENSE) under the zlib license.

## References

* [1] [ARMageddon: Cache Attacks on Mobile Devices (USENIX Security 2016) - Lipp, Gruss, Spreitzer, Maurice, Mangard](https://www.usenix.org/conference/usenixsecurity16/technical-sessions/presentation/lipp)
* [2] [ARMageddon: How Your Smartphone CPU Breaks Software-Level Security and Privacy (Black Hat Europe 2016) - Lipp, Maurice](https://www.blackhat.com/eu-16/briefings/schedule/index.html#armageddon-how-your-smartphone-cpu-breaks-software-level-security-and-privacy-4887)
* [3] [Cache Template Attacks: Automating Attacks on Inclusive Last-Level Caches (USENIX Security 2015) - Gruss, Spreitzer, Mangard](https://www.usenix.org/node/191011)
