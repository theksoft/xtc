===========
Local Heaps
===========

1. Introduction
===============

The implementation is generic to allow instantiating several local heap for different isolation purpose.
The interface proposed is similar to system malloc-like in order to be redirected easily to it
if the memory constraint initially raised disappears.

Refer to the provided examples located in *./examples/src-c/* showing how to handle this using compilation swicthes.

2. Thread-safety
================

Thread-safety is achieved by providing a pointer to a valid **xtc_protect_t** structure as the **protect** parameter 
of the heap structure initialisation with **xlh_init()**.
If the **protect** parameter is provided with a **NULL** value, the resulting heap is not thread-safe.
It allows it to be used when concurrency protection is not required
or if concurrency protection is managed externally to the implementation.

The **protect** structure parameter must provide a valid **lock()** and **unlock()** functions that are called
by the implementation to control the local heap accesses from thread concurrency.
The provided locking functions should be unique par heap unless several heaps share a common feature to protect.

Refer to the provided examples located in *./examples/src-c/* showing how to achieve it with C11 threading feature.
It can be easily adapted to any threading system.

3. Principle
============

The generic implementation is based on the following requirements:

- The memory pool will hold any kind of requested size.
- It requires additional space for each requested memory block to manage some state information.

This heap implementation manages a list of free memory blocks and a list of contiguous memory blocks.
Memory nodes are allocated with the user payload using additional space as stated above.
The more allocation it is requested, the more memory block it has and the more memory overhead it results.

**Memory block lists**

The list a free memory blocks is an ordered list of memory blocks that are available to memory allocation.
Free blocks are ordered by size from the biggest one to the smallest one.
Selecting a free block for allocation means finding the smallest free block that can fit the requested size.

The list of contiguous memory block refers all the blocks, free and allocated, ordered by increasing base addresses.
It means that each time a memory block is created whether free or allocated, it is inserted in this list
so that the previous one represents the previous contiguous memory area in the memory pool
and the next one represents the next contiguous memory area.
Two contiguous blocks are either both allocated or one is allocated while the other is free.
There shall never be two contiguous blocks that are free: they must have been merged.

After heap initialization, there is only one free memory block holding almost the total space of the memory pool
- less the size of node management structure.

Note that this kind of heap supports array allocation.

**Memory operations**

When a memory block is requested, a free memory block fitting the requested size is searched for.
When found, it is eventually split at the integer-aligned size boundary
if the remaining of the memory block can hold a new memory node.

When it is freed, it is basically inserted in the list of free memory blocks ordered by decreasing size.
It has possibly been previously meged with the previous and/or next contiguous blocks when free.

3.1. The memory pool
--------------------

The memory pool for any kind of data size has to be provided so that it can be
statically or dynamically allocated by the caller.
Refer to the provided examples located in *./examples/src-c/*
showing how to achieve static or dynamic preallocation.

The size of the memory pool is not the size that will be usable
as each allocation wil consume an additional memory overhead for managing the block.

The size that is actually allocated is aligned on integer boundary
in order to hold properly a user structure content if the purpose is a structure allocation.
C language requires structures to be aligned on integer boundary.

3.2. The node structure
-----------------------

The node structure - **xlh_node_t** - contains basically the following fields:

- the identifier of the related **heap** it belongs, basically build on the memory address of the heap.

- the allocated **size** is the requested size aligned on integer boundary.

- **blk** and **free** allows building the list of contiguous block and the list of free blocks.
  They are double linked lists to allow direct and reverse traversal.

Other additional fields allow to track memory leak when using the debug version of the libray.
Check the **xlh_dump()** function and other information function.
Note that memory leaks can be tracked also by aliasing heap allocation operations on
system allocation operations and using a standard memory tracker like *valgrind*.

The user payload start after the information fields and can hold the requested size.

3.3. The heap structure
-----------------------

The heap structure holds all necessary information to manage the local memory pool.

- Some value like **node_offset** and **count** are precomputed or maintained
  during allocation invocations to speed up the latter and avoid costly list traversal.

- The **head_blks** field is the head of the list of contiguous memory blocks.
  This list can never be empty.

- The **head_free** and **tail_free** fields are the head and tail of the list of free memory blocks.
  These lists can be empty.

- The **interface** field is a C generic structure of function pointers
  that allows to handle the same way different type of memory heaps
  - structure heaps, local heaps or even system heap.

3.4. Robustness
---------------

A memory block allocated with a specific local heap cannot be freed
with another type of heap including system heap or another local heap.

4. Usage
========

- The memory pool must be allocated sttaically or dynamically.
- A heap structure can be declared statically of dynamically.
- The heap allocated structure must be initialized with the allocated memory pool
  and optionally protection functions with the **xlh_init()** function.
  When no more needed, the heap must be cleaned up with **xlh_end()**.
  Note that the memory pool is not automatically freed when allocated dynamically.
  It must be explicitely freed by the caller.
- Allocations are performed and managed with **xlh_alloc()** and **xlh_free()**.

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
