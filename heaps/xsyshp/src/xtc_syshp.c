/*
  System heap in the XTC form.
  For tracking memory issue, prefer the use of a standard memory tracker tool.
*/

#include "xtc_syshp.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

//==================================================================================================
// Local inline functions
//==================================================================================================

/**
 * @brief Get the system heap object identifier.
 * 
 * @return
 * This function returns the system heap identifier as @c xtc_heaps_id_t.
 */

static xtc_heaps_id_t get_id() {
  static unsigned int ref = 0x0FEEDBABE;
  return (xtc_heaps_id_t)(uintptr_t)&ref;
}

/**
 * @brief Check that the provided heap is the expected type.
 *
 * As the heap structure has been cast in more generic type @c xtc_heap_t,
 * this function purpose ensures the right type of heap has been provided
 * e.g. is a @c xss_heap_t type.
 *
 * @param this Provided heap structure.
 * @return
 * This function returns back the provided heap structure if the latter is correct
 * or @c NULL if it is not the expected structure.
 */

static inline xss_heap_t* check(xss_heap_t *this) {
  return this && this->interface.id == get_id() ? this : NULL;
}

//==================================================================================================
// Local functions declaration
//==================================================================================================

static xss_node_t* get_node(xss_heap_t *this, void *ptr);

#ifdef __DEBUG
static void dump(xss_heap_t *heap);
#endif

static void dummy() {}

//==================================================================================================
// Interface functions
//==================================================================================================

xtc_heap_t* xss_init(xss_heap_t *this, xtc_protect_t *protect) {
  xtc_heap_t *rtn = NULL;
  if (this) {
    if (!protect || (protect->lock && protect->unlock)) {
      this->interface.id = get_id();
#ifndef __DEBUG
      this->interface.alloc = (xtc_alloc_t)xss_alloc;
#else
      this->head = NULL;
      this->interface.dump = (xtc_dump_t)xss_dump;
      this->interface.alloc_dbg = (xtc_alloc_t)xss_alloc_dbg;
#endif
      this->interface.free = (xtc_free_t)xss_free;
      this->interface.count = (xtc_count_t)xss_count;
      this->interface.end = (xtc_end_t)xss_end;
      this->count = 0;
      this->total_size = 0;
      this->protect.lock = (protect) ? protect->lock : dummy;
      this->protect.unlock = (protect) ? protect->unlock : dummy;
      rtn = &this->interface;
    }
  }
  return rtn;
}

void* xss_end(xss_heap_t *this, size_t *count) {
  xss_heap_t *heap = check(this);
  if (heap) {
    xtc_protect_t protect = heap->protect;
    protect.lock();
    size_t cnt = heap->count;
    if (count) {
      *count = cnt;
    }
#ifdef __DEBUG
    if (cnt) {
      dump(heap);
    }
#endif
    memset(heap, 0, sizeof(xss_heap_t));
    protect.unlock();
    if (cnt) {
      fprintf(stderr, "WARNING: %ld memory leak(s) detected when ending heap!\n", cnt);
    }
  }
  return NULL;
}

#ifndef __DEBUG
void* xss_alloc(xss_heap_t *this, size_t size)
#else
void* xss_alloc_dbg(xss_heap_t *this, size_t size, const char *fn, int line)
#endif
{
  void *rtn = NULL;
  xss_heap_t *heap = check(this);
  if (heap && size) {
    heap->protect.lock();
    xss_node_t *node = malloc(size + sizeof(xss_node_t));
    if (node) {
      memset(node, 0, sizeof(xss_node_t));
      node->heap = (uintptr_t)heap;
      node->size = size;
      heap->total_size += size;
      heap->count++;
#ifdef __DEBUG
      node->fn = fn;
      node->line = line;
      node->previous = NULL;
      node->next = heap->head;
      if (heap->head) {
        heap->head->previous = node;
      }
      heap->head = node;
#endif
      rtn = (void*)(node + 1);
    }
    heap->protect.unlock();
  }
  return rtn;
}

void xss_free(xss_heap_t *this, void *ptr) {
  xss_heap_t *heap = check(this);
  if (heap && ptr) {
    heap->protect.lock();
    xss_node_t *node = get_node(heap, ptr);
    if (node) {
      heap->count--;
      heap->total_size -= node->size;
#ifdef __DEBUG
      if (heap->head == node) {
        heap->head = node->next;
      }
      if (node->previous) {
        node->previous->next = node->next;
      }
      if (node->next) {
        node->next->previous = node->previous;
      }
#endif
      memset(node, 0, sizeof(xss_node_t));
      free(node);
    }
    heap->protect.unlock();
  }
}

size_t xss_count(xss_heap_t *this) {
  size_t rtn = 0;
  xss_heap_t *heap = check(this);
  if (heap) {
    heap->protect.lock();
    rtn = heap->count;
    heap->protect.unlock();
  }
  return rtn;
}

size_t xss_total_size(xss_heap_t *this) {
  size_t rtn = 0;
  xss_heap_t *heap = check(this);
  if (heap) {
    heap->protect.lock();
    rtn = heap->total_size;
    heap->protect.unlock();
  }
  return rtn;
}

#ifdef __DEBUG

void xss_dump(xss_heap_t *this) {
  xss_heap_t *heap = check(this);
  if (heap) {
    heap->protect.lock();
    dump(heap);
    heap->protect.unlock();
  }
}

#endif

//==================================================================================================
// Local functions implementation
//==================================================================================================

/**
 * @brief Get the system heap node object related to the provided pointer.
 * 
 * System heap validity must have been checked by caller.
 * The provided pointer @c ptr is valid if the identifier is the same as the heap.
 * 
 * @param heap The heap which the provided pointer ptr belongs to.
 * @param ptr The allocated system heap node.
 * @return
 * The function returns a valid @c xss_node_t* related to the provided pointer @c ptr
 * or @c NULL if the provided pointer does not belong to the provided heap.
 */

static xss_node_t* get_node(xss_heap_t *heap, void *ptr) {
  xss_node_t *rtn = NULL;
  xss_node_t *tmp = (xss_node_t*)ptr - 1;
  if (tmp->heap == (uintptr_t)heap) {
    rtn = tmp;
  }
  return rtn;
}

#ifdef __DEBUG

/**
 * @brief Dump the heap allocated block information.
 * 
 * @param heap System heap
 */

static void dump(xss_heap_t *heap) {
  xss_node_t *node;
  printf("HEAP SYS, Blks. %p[%ld] - %ld bytes\n",
    (void*)heap->head, heap->count, heap->total_size);
  for (node = heap->head; NULL != node; node = node->next) {
    printf("\t%p[%ld] @ %s (%d)\n",
      (void*)(node + 1), node->size, node->fn, node->line);
  }
}

#endif
