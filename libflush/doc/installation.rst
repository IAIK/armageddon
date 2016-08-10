Installation
============

The library is shipped with a Makefile and can be compiled by running:

.. code-block:: sh

  make

The necessary header files and the shared and static build library can
be installed on the host system:

.. code-block:: sh

  make install

In addition we provide a debug build that can be initiated by calling
``make debug`` as well as a simple test suite that can be run by calling
``make test``. Code coverage of the test suite can be determined by
running ``make gcov``.

Additionally we provide an ``Android.mk`` and an ``Application.mk`` file
that can be used to build the library with the `Android
NDK <https://developer.android.com/ndk/index.html>`__ toolset:

.. code-block:: sh

  ndk-build NDK_APPLICATION_MK=`pwd`/Application.mk NDK_PROJECT_PATH=`pwd`

Dependencies
~~~~~~~~~~~~

libflush does not require any dependencies except for running the test suite or
building the documentation. However, by default it uses the toolchains provided
by the Android NDK if built for *armv7* or *armv8*.

-  `Android NDK <https://developer.android.com/ndk/index.html>`__ -
   Android Native Development Kit (optional for ARM builds)
-  `check <https://libcheck.github.io/check/>`__ - Unit Testing
   Framework for C (optional for test cases)
-  `libfiu <https://blitiri.com.ar/p/libfiu/>`__ - Fault injection in
   userspace (optional for test cases)
-  `doxygen <www.doxygen.org>`__ (optional, for HTML documentation)
-  `Sphinx <http://www.sphinx-doc.org>`__ (optional, for HTML
   documentation)
-  `sphinx\_rtd\_theme <https://github.com/snide/sphinx_rtd_theme>`__
   (optional, for HTML documentation)
-  `breathe <https://github.com/michaeljones/breathe>`__ (optional, for
   HTML documentation)

Build Configuration
-------------------

The build system makes use of several configuration files. The
parameters can be adjusted by modifying the files accordingly or by
passing them to make (``make ARCH=x86``). The most important properties
are the following:

-  ``ARCH``: Defines the target architecture.

   -  *x86* (default) - Support for *i386* and *x86\_64*
   -  *armv7* - Support for ARMv7
   -  *armv8* - Support for ARMv8

-  ``USE_EVICTION``: Use eviction instead of flush instruction in flush
   based functions. Required for devices that do not expose a flush
   instruction. (default: 0, enabled by default for _armv7_ architecture)
-  ``DEVICE_CONFIGURATION``: Defines cache and eviction based properties
   for the target device if eviction is used. See
   *libflush/eviction/strategies* for
   example device configurations.

   -  *default* (default) - Default device configuration.
   -  *alto45* - Alcatel OneTouch POP 2
   -  *bacon* - OnePlus One
   -  *mako* - Nexus 4
   -  *hammerhead* - Nexus 5
   -  *tilapia* - Nexus 7
   -  *manta* - Nexus 10
   -  *zeroflte* - Samsung Galaxy S6

-  ``TIME_SOURCE``: Gives the possibility to use different timing
   sources to measure the execution time. Depending on the available
   privileges, one might want to change the timing source.

   -  *register* - Performance register / Time-stamp counter (default)
   -  *perf* - Perf interface
   -  *monotonic\_clock* - Monotonic clock
   -  *thread\_counter* - Dedicated thread counter

-  ``WITH_PTHREAD``: Build with pthread support.
-  ``HAVE_PAGEMAP_ACCESS``: Defines if access to */proc/self/pagemap* is
   granted.

If the library is build for the ARMv7 or the ARMv8 architecture the
build system uses the ``config-arm.mk`` or
``config-arm64.mk`` configuration file. By default the
build system makes use of the toolchains provided by the `Android
NDK <https://developer.android.com/ndk/index.html>`__, thus its possible
that the installation path of the NDK needs to be modified:

-  ``ANDROID_NDK_PATH``: Path to the installation of the Android NDK.

   -  */opt/android-ndk* (default)

-  ``ANDROID_PLATFORM``: Defines the used Android platform that is used.

   -  *android-21* (default)

If you prefer to use a different toolchain/compiler, feel free to change
``CC`` and other properties accordingly.

Advanced Configuration
----------------------

Eviction strategy
^^^^^^^^^^^^^^^^^

If eviction is used, libflush uses the parameters defined by the
``DEVICE_CONFIGURATION``. The device configuration is represented by a header
file in *libflush/eviction/strategies* and is structured as the following:

**Cache specific configuration**

- ``NUMBER_OF_SETS``: The number of sets in the cache
- ``LINE_LENGTH``: The line length
- ``LINE_LENGTH_LOG2``: The log base 2 of the line length

**Eviction strategy configuration**

- ``ES_EVICTION_COUNTER``: Length of the loop
- ``ES_NUMBER_OF_ACCESSES_IN_LOOP``: Number of accesses of an address in one loop round
- ``ES_DIFFERENT_ADDRESSES_IN_LOOP``: Number of different addresses in one loop round
