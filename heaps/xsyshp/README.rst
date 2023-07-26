============
System Heaps
============

1. Introduction
===============

The implementation is allows instantiating several system heap but all redirecting on same system allocation.
Its interest is limited but is provided to provide same interfaces as structure and local heap.

2. Thread-safety
================

Thread-safety is achieved by providing a pointer to a valid **xtc_protect_t** structure as the **protect** parameter 
of the heap structure initialisation with **xss_init()**.
If the **protect** parameter is provided with a **NULL** value, the resulting heap is not thread-safe.
It allows it to be used when concurrency protection is not required
or if concurrency protection is managed externally to the implementation.

The **protect** structure parameter must provide a valid **lock()** and **unlock()** functions that are called
by the implementation to control the system heap accesses from thread concurrency.
The provided locking functions should be unique per heap unless several heaps share a common feature to protect.
Note that although malloc()/free() are generally thread-safe, this wrapping implmentation manges additional
information that must be protected against concurrent accesses so that providing a **protect** still makes sense.

Refer to the provided examples located in *./examples/src-c/* showing how to achieve it with C11 threading feature.
It can be easily adapted to any threading system.

3. Principle
============

The generic implementation is based on the following requirements:

- It requires additional space for each requested memory block to manage some state information.
- It almost wraps directly on malloc()/free() but manages additional state information.

This heap implementation manages additional statistic information.

3.2. The node structure
-----------------------

The node structure - **xss_node_t** - contains basically the following fields:

- the identifier of the related **heap** it belongs, basically build on the memory address of the heap.

- the allocated **size** taht has been requested.

Other additional fields allow to track memory leak when using the debug version of the libray.
Check the **xss_dump()** function and other information function.
Note that memory leaks can be tracked using a standard memory tracker like *valgrind*.

The user payload start after the information fields and can hold the requested size.

3.3. The heap structure
-----------------------

The heap structure holds all necessary information to manage the local memory pool.

- Some value like **total_size** and **count** are precomputed or maintained
  during allocation invocations.

- The **interface** field is a C generic structure of function pointers
  that allows to handle the same way different type of memory heaps
  - structure heaps, local heaps or this system heap.

3.4. Robustness
---------------

A memory block allocated with a specific system heap cannot be freed
with another type of heap including another system heap and system free().

4. Usage
========

- A heap structure can be declared statically of dynamically.
- The heap allocated structure must be initialized with optional 
  protection functions with the **xss_init()** function.
  When no more needed, the heap must be cleaned up with **xss_end()**.
- Allocations are performed and managed with **xss_alloc()** and **xss_free()**.

Check the doxygen documentation for a complete description of all interface functions.

Check the provided examples located in *./examples/src-c/*.

5. Additional information
=========================

5.1. C Standard
---------------

This implementation is C 2011 compliant.
It may be compliant with earlier C standard but this has not been checked.

5.2. License
------------

This implementation is provided under the MIT Llicense, check the LICENSE file for more information.

Visit theksoft_ for more source material.

.. _theksoft: https://github.com/theksoft/xtc
