# Input Simulator

Input Simulator is a tool that can be used to simulate events like taps on the
touch screen.

It has been used for our [ARMageddon: Cache Attacks on Mobile Devices](https://www.usenix.org/conference/usenixsecurity16/technical-sessions/presentation/lipp) paper by Lipp, Gruss, Spreitzer, Maurice and Mangard that has been published at the Usenix Security Symposium 2016 and presented at [Black Hat Europe 2016](https://www.blackhat.com/eu-16/briefings/schedule/index.html#armageddon-how-your-smartphone-cpu-breaks-software-level-security-and-privacy-4887).

## Table of content

- [Installation](#installation)
    - [Dependencies](#dependencies)
- [Usage](#usage)
- [Customization](#example)
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
### Dependencies

By default it uses the toolchains provided by the Android NDK if built
for _armv7_ or _armv8_.

* [Android NDK](https://developer.android.com/ndk/index.html) - Android Native
    Development Kit (optional, for ARM builds)

## Build Configuration

The build system makes use of several configuration files. The parameters can be adjusted by modifying the files accordingly or by passing them to make (`make ARCH=x86`). The most important properties are the following:

* `ARCH`: Defines the target architecture.
    * _x86_ (default) - Support for _i386_ and _x86_64_
    * _armv7_ - Support for ARMv7
    * _armv8_ - Support for ARMv8
* `DEVICE_CONFIGURATION`: Define which device you are using to enable 
    device specific code.

If the library is build for the ARMv7 or the ARMv8 architecture the build system uses the [config-arm.mk](config-arm.mk) or [config-arm64.mk](config-arm64.mk) configuration file. By default the build system makes use of the toolchains provided by the [Android NDK](https://developer.android.com/ndk/index.html), thus its possible that the installation path of the NDK needs to be modified:

* `ANDROID_NDK_PATH`: Path to the installation of the Android NDK.
    * _/opt/android-ndk_ (default)
* `ANDROID_PLATFORM`: Defines the used Android platform that is used.
    * _android-21_ (default)

If you prefer to use a different toolchain/compiler, feel free to change `CC` and other properties accordingly.

## Usage

```
input-simulator [OPTIONS] <letter>
```

The following options are available:

* **-r, -repititions**

    How often the event should be triggered. -1 means infinite repititions.

* **-d, -delay**

    The delay between each triggered event.

* **-h, -help**

    Show the help information.

## Customization

**This code needs to be customized to suite your needs.** While we provide the mapping
for the keyboards of three devices, they might do not match the keyboard and the
device you use. Thus, you need to modify or extend the source code in
`key-mapping.c` and `simulator.c`.

## License

[Licensed](LICENSE) under the zlib license.

## References

* [1] [ARMageddon: Cache Attacks on Mobile Devices (USENIX Security 2016) - Lipp, Gruss, Spreitzer, Maurice, Mangard](https://www.usenix.org/conference/usenixsecurity16/technical-sessions/presentation/lipp)
* [2] [ARMageddon: How Your Smartphone CPU Breaks Software-Level Security and Privacy (Black Hat Europe 2016) - Lipp, Maurice](https://www.blackhat.com/eu-16/briefings/schedule/index.html#armageddon-how-your-smartphone-cpu-breaks-software-level-security-and-privacy-4887)
* [3] [Cache Template Attacks: Automating Attacks on Inclusive Last-Level Caches (USENIX Security 2015) - Gruss, Spreitzer, Mangard](https://www.usenix.org/node/191011)
