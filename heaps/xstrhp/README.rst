===============
STRUCTURE HEAPS
===============

1. Introduction
===============

The implementation is generic to allow instantiating several structure heap for different type of structure.
The interface proposed is similar to system malloc-like in order to be redirected easily to it
if the memory constraint initially raised disappears.

Refer to the provided examples located in *./examples/src-c/* showing how to handle this using compilation swicthes.

2. Thread-safety
================

Thread-safety is achieved by providing a pointer to a valid **xtc_protect_t** structure as the **protect** parameter 
of the heap structure initialisation with **xsh_init()**.
If the **protect** parameter is provided with a **NULL** value, the resulting heap is not thread-safe.
It allows it to be used when concurrency protection is not required
or if concurrency protection is managed externally to the implementation.

The **protect** structure parameter must provide a valid **lock()** and **unlock()** functions that are called
by the implementation to control the structure heap accesses from thread concurrency.
The provided locking functions should be unique par heap unless several heaps share a common feature to protect.

Refer to the provided examples located in *./examples/src-c/* showing how to achieve it with C11 threading feature.
It can be easily adapted to any threading system.

3. Principle
============

The generic implementation is based on the following requirements:

- It requires to be initialized with the size of the structure to manage.
- It requires additional space for each memory block to manage some state information.

3.1. The memory pool
--------------------

The memory pool for managing dedicated structure has to be provided so that it can be
statically or dynamically allocated by the caller.
Refer to the provided examples located in *./examples/src-c/*
showing how to achieve static or dynamic preallocation.

The size of the memory pool must be able to hold the desired number of structure as well as additional state.
Helper macro-instructions **XSH_HEAP_LENGTH()** computes the right memory pool size
from the structure **name** - doing a **sizeof()** on the name - and the required **count** of structure.
Providing the name ensures the computing can be processed at compilation time.

The size that is actually computed is aligned on integer boundary
in order to hold properly a user structure content.
C language requires structures to be aligned on integer boundary.

3.2. The node structure
-----------------------

The node structure - **xsh_node_t** - contains basically two fields:
- the **allocated** flag indicates if the node is busy
- the **next** field allows to chain free memory blocks

Chaining free memory blocks eases the allocation management:

- allocation operation is just popping a free memory block
  instead of looping all memory blocks to find a free one e.g. not **allocated**.

- free operation is pushing the memory block in the list
  which additional cost overhead is very light compared to just unsetting **allocated** flag.

Other additional fields allow to track memory leak when using the debug version of the libray.
Check the **xsh_dump()** function and other information function.

The user payload start after the information fields and can hold the provide structure size.

3.3. The heap structure
-----------------------

The heap structure holds all necessary information to manage the memory pool.

- Some value like **node_offset**, **allocated_count** and **free_count** are precomputed or maintained
  during allocation invocations to speed up the latter.

- The **lfree** field is the head of free memory blocks.

- The **interface** field is a C generic structure of function pointers
  that allows to handle the same way different type of memory heaps
  - structure heaps, local heaps or even system heap.

3.4. Robustness
---------------

A memory block allocated with a specific structure heap cannot be freed
with another type of heap including system heap or another structure heap.

4. Usage
========

- The memory pool must be allocated sttaically or dynamically with the required size.
- A heap structure can be declared statically of dynamically.
- The heap allocated structure must be initialized with the allocated memory pool
  and optionally protection functions with the **xsh_init()** function.
  When no more needed, the heap must be cleaned up with **xsh_end()**.
- Allocations are performed and managed with **xsh_alloc()** and **xsh_free()**.

Check the doxygen documentation for a complete description of all interface functions.

Check the provided examples located in *./examples/src-c/*.

5. Limitations
==============

This implementation is not able to allocate arrays of structures.

It is more complex to achieve due to the node management.
Achieving should requires to split all user payload from all node additional states
and trying to keep relations and consistency accross all operations.

Use local heap for achieving allocation of arrays with a heap ceiled memory occupation.

6. Additional information
=========================

6.1. C Standard
---------------

This implementation is C 2011 compliant.
It may be compliant with earlier C standard but this has not been checked.

6.2. License
------------

This implementation is provided under the MIT Llicense, check the LICENSE file for more information.

Visit theksoft_ for more source material.

.. _theksoft: https://github.com/theksoft/xtc
