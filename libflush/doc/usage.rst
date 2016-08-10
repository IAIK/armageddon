Usage
=====

The following sections illustrate the usage of libflush. For a complete overview
of the available functions please refer to the :doc:`api`.

Initialization and termination
------------------------------

In order to use libflush one needs to include the header file and call
the ``libflush_init`` to initialize the library. In the end
``libflush_terminate`` needs to be called to clean-up.

.. code-block:: c

    #include <libflush.h>

    int main(int argc, char* argv[]) {
        // Initialize libflush
        libflush_session_t* libflush_session;
        if (libflush_init(&libflush_session, NULL) == false) {
            return -1;
        }
        
        // Use libflush...
        
        // Terminate libflush
        if (libflush_terminate(libflush_session) == false) {
            return -1;
        }
        
        return 0;
    }

and link against the library:

::

    gcc example.c `pkg-config --libs libflush`

Flush or evict an address
-------------------------

To evict an address from the cache to the main memory, a dedicated flush
instruction (if available) or eviction can be used.

.. code-block:: c

    void* address = &foo;

    // Use the flush instruction (if possible)
    libflush_flush(libflush_session, address);

    // Use eviction
    libflush_evict(libflush_session, address);

Get timing information
----------------------

To retrieve a time stamp depending on the used time source can be
achieved with the following function.

.. code-block:: c

    uint64 time = libflush_get_timing(libflush_session);

In addition wrapper functions to measure the execution time of different
functions are given.

.. code-block:: c

    uint64 time = libflush_reload_address(libflush_session, address);
