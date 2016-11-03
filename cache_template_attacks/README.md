# Cache Template Attacks

Cache Template Attacks is a platform-independent tool that utilizes [libflush](../libflush) to launch [cache template attacks](https://www.usenix.org/system/files/conference/usenixsecurity15/sec15-paper-gruss.pdf) on x86 as well as ARMv7 and ARMv8 architecture. We used it to monitor tap and swipe events as well as keystrokes, and even derive the words entered on the touchscreen on mobile devices.

The [ARMageddon: Cache Attacks on Mobile Devices](https://www.usenix.org/conference/usenixsecurity16/technical-sessions/presentation/lipp) paper by Lipp, Gruss, Spreitzer, Maurice and Mangard has been published at the Usenix Security Symposium 2016 and presented at [Black Hat Europe 2016](https://www.blackhat.com/eu-16/briefings/schedule/index.html#armageddon-how-your-smartphone-cpu-breaks-software-level-security-and-privacy-4887).

## Table of content

- [Installation](#installation)
    - [Dependencies](#dependencies)
- [Build Configuration](#build-configuration)
- [Usage](#usage)
- [Example](#example)
- [License](#license)
- [References](#references)

## Installation

The tool is shipped with a Makefile and can be compiled by running:
```bash
make
```

The tool can be installed on the host system:
```bash
make install
```

In addition we provide a debug build that can be initiated by calling `make debug`.

### Dependencies
Cache Template Attacks utilizes libflush and, thus, this is the only dependency.

* [libflush](../libflush) (required)

However, by default it uses the toolchains provided by the Android NDK if built
for _armv7_ or _armv8_.

* [Android NDK](https://developer.android.com/ndk/index.html) - Android Native
    Development Kit (optional, for ARM builds)

## Build Configuration

The build system makes use of several configuration files. The parameters can be adjusted by modifying the files accordingly or by passing them to make (`make ARCH=x86`). The most important properties are the following:

* `ARCH`: Defines the target architecture.
    * _x86_ (default) - Support for _i386_ and _x86_64_
    * _armv7_ - Support for ARMv7
    * _armv8_ - Support for ARMv8
* `WITH_THREADS`: If the thread-based implementation should be used.
* `WITH_ANDROID`: If it is build for an Android device and ashmem has to be used.

If the library is build for the ARMv7 or the ARMv8 architecture the build system uses the [config-arm.mk](config-arm.mk) or [config-arm64.mk](config-arm64.mk) configuration file. By default the build system makes use of the toolchains provided by the [Android NDK](https://developer.android.com/ndk/index.html), thus its possible that the installation path of the NDK needs to be modified:

* `ANDROID_NDK_PATH`: Path to the installation of the Android NDK.
    * _/opt/android-ndk_ (default)
* `ANDROID_PLATFORM`: Defines the used Android platform that is used.
    * _android-21_ (default)

If you prefer to use a different toolchain/compiler, feel free to change `CC` and other properties accordingly.

## Usage

```bash
cache_template_attack [OPTIONS] <library>
```

The following options are available:

* **-r, -range**

    The range of addresses that should be scanned.
    (Example: *7f9783a000-7f9804d000*)

* **-o, -offset**

    The offset where the scan process should start.
    (Example: *0x920000*)

* **-f, -fork**

    The number of spy processes that should be created.
    (Default: *1*)

* **-t, -threshold**

    The threshold that is used to distinguish between a cache hit and a cache
    miss. If no value has been passed, a calibration process will figure out a
    threshold.

* **-n, -number-of-tests**

    The number of tests that are executed for each address.
    Default: *1000*

* **-u, -offset-update-time**

    The time in seconds before the master thread will update the offset that is
    used to spy on.
    Default: *1*

* **-c, -cpu**

    Bind to CPU.
    Default: *0*

* **-s, -spy**

    If the tool should only spy on a single address that is defined by the
    offset parameter.

* **-z, -show-timing**

    If the tool should print a timing information instead of the number of cache hits
    that it has detected. This is a convenient option to create traces of the
    usage over time.

* **-l, -logfile**

    If the tool should log the results in form of a CSV file.

* **-h, -help**

    Show the help information.

## Example

As an example you can use this tool to find addresses of a keyboard that are
triggered by certain events. For instance, you want to spy on the Samsung
Keyboard that is the default keyboard on the Samsung Galaxy S6 and want to find
addresses that are activated if the user presses any letter on the alphabet.

By inspecting the ``/proc/<pid>/maps`` file, one can get knowledge about which
files are mapped by the victim process. In the above example, we find the
following entries that describe a region of contiguous virtual memory mapped to
the ``SamsungIMEv2.odex`` file:

    7f971c6000-7f9783a000 r--p 00000000 08:0f 49066 /system/app/SamsungIMEv2/arm64/SamsungIMEv2.odex
    7f9783a000-7f9804d000 r-xp 00674000 08:0f 49066 /system/app/SamsungIMEv2/arm64/SamsungIMEv2.odex
    7f9804d000-7f9804e000 rw-p 00e87000 08:0f 49066 /system/app/SamsungIMEv2/arm64/SamsungIMEv2.odex
    7faaee1000-7faaee2000 r--p 00000000 08:0f 49066 /system/app/SamsungIMEv2/arm64/SamsungIMEv2.odex

We can use this information and pass it to our ``cache_template_attack`` tool:

    cache_template_attack -c 0 -r 7f9783a000-7f9804d000 -o 00674000 -t 230 -f 1 /system/app/SamsungIMEv2/arm64/SamsungIMEv2.odex

In parallel we need to simulate the event by either manually pressing on the
keyboard on the touchscreen or automating this process.

If the spy process detects cache hits on the address it currently spies on, it
will print it. In addition, you could save the file to a logfile which helps
evaluating your results.

     0x3d300 - 5
     0x3d300 - 11
     0x3d300 - 14
     0x3d300 - 23
     0x3d340 - 1
     0x3d340 - 2
     0x3d340 - 2
     0x3d340 - 2

If you have identified addresses that are triggered by certain events, you can
use the tool to spy just on them. In our example. we identified an address
that is triggered if the user pressed a letter on the screen and one that helps
us to decide if he pressed the space bar. Using only those two addresses, we are
now capable of deriving the length of the words the user typed into the phone:

![Spy on the Keyboard](./images/keyboard_sentence.png)

## License

[Licensed](LICENSE) under the zlib license.

## References

* [1] [ARMageddon: Cache Attacks on Mobile Devices (USENIX Security 2016) - Lipp, Gruss, Spreitzer, Maurice, Mangard](https://www.usenix.org/conference/usenixsecurity16/technical-sessions/presentation/lipp)
* [2] [ARMageddon: How Your Smartphone CPU Breaks Software-Level Security and Privacy (Black Hat Europe 2016) - Lipp, Maurice](https://www.blackhat.com/eu-16/briefings/schedule/index.html#armageddon-how-your-smartphone-cpu-breaks-software-level-security-and-privacy-4887)
* [3] [Cache Template Attacks: Automating Attacks on Inclusive Last-Level Caches (USENIX Security 2015) - Gruss, Spreitzer, Mangard](https://www.usenix.org/node/191011)
