libflush documentation
======================

libflush is a library to launch cache attacks on x86 as well as ARMv7 and ARMv8 architecture. It allows to easily implement attacks based on `Prime+Probe <https://eprint.iacr.org/2005/271.pdf/>`_, `Flush+Reload <https://eprint.iacr.org/2013/448.pdf/>`_, `Evict+Reload <https://www.usenix.org/system/files/conference/usenixsecurity15/sec15-paper-gruss.pdf/>`_, `Flush+Flush <http://arxiv.org/abs/1511.04594/>`_ and `Prefetch </>`_ attack techniques and thus offers a great playground for research and education purposes.

We have developed this library in the `ARMageddon: Cache Attacks on Mobile Devices <https://www.usenix.org/conference/usenixsecurity16/technical-sessions/presentation/lipp/>`_ paper and used it to build covert-channels that outperform state-of-the-art covert channels on Android by several orders of magnitude. We utilized it to implement `cache template attacks <https://www.usenix.org/system/files/conference/usenixsecurity15/sec15-paper-gruss.pdf/>`_ that monitor tap and swipe events as well as keystrokes, and even derive the words entered on the touchscreen. Moreover, we used it to attack cryptographic primitives in Java and to monitor cache activity in the ARM TrustZone from the normal world. With the `Prefetch Side-Channel Attacks: Bypassing SMAP and Kernel ASLR </>`_ paper we have extended the library to support prefetch attack techniques to obtain address information that can be used to defeat SMAP, SMEP and kernel ASLR.

The `ARMageddon: Cache Attacks on Mobile Devices <https://www.usenix.org/conference/usenixsecurity16/technical-sessions/presentation/lipp/>`_ paper by Lipp, Gruss, Spreitzer, Maurice and Mangard will be published at the Usenix Security Symposium 2016. The
`Prefetch Side-Channel Attacks: Bypassing SMAP and Kernel ASLR </>`_ paper by Gruss, Fogh, Maurice, Lipp and Mangard will be published at ACM Conference on Computer and Communications Security 2016.

.. toctree::
  :maxdepth: 2
  :numbered:

  installation
  usage
  example
  api
