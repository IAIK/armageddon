Example
=======

A more sophisticated example using libflush can be found in the
`example <example>`__ directory. It implements a calibration tool for
the Flush+Reload, Prime+Probe, Evict+Reload, Flush+Flush and Prefetch
attack. The example can be compiled by running ``make example`` and
executed by running ``./example/build/<arch>/release/bin/example``.
In addition the example can also be build with the ``ndk-build`` tool.

.. literalinclude:: ../example/main.c
  :language: c
  :linenos:
