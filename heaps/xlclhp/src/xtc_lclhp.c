#include "xtc_lclhp.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

//==================================================================================================
// Local inline functions
//==================================================================================================

/**
 * @brief Get the local heap object identifier.
 * 
 * @return
 * This function returns the local heap identifier as @c xtc_heaps_id_t.
 */

static xtc_heaps_id_t get_id() {
  static unsigned int ref = 0x0FEEDBABE;
  return (xtc_heaps_id_t)(uintptr_t)&ref;
}

/**
 * @brief Check that local heap node size is compatible with integer alignment.
 * 
 * C langage ref.: Structure must be aligned on integer boundary.
 * 
 * This function is used only during heap local initialization.
 * It returns nothing but assert() if @c xlh_node_t type size is not compatible.
 */

static inline void check_node_size() {
#ifdef __DEBUG
  struct {
    xlh_node_t n1;
    struct {
      int dummy;
    } n2;
  } x;
  // Check possible integer size issue
  assert(sizeof(xlh_node_t) == (uintptr_t)&x.n2 - (uintptr_t)&x.n1);
#endif
}

/**
 * @brief Check that the provided heap is the expected type.
 *
 * As the heap structure has been cast in more generic type @c xtc_heap_t,
 * this function purpose ensure the right type of heap has been provided
 * e.g. is a @c xlh_heap_t type.
 *
 * @param this Provided heap structure.
 * @return
 * This function returns back the provided heap structure if the latter is correct
 * or @c NULL if it is not the expected structure.
 */

static inline xlh_heap_t* check(xlh_heap_t *this) {
  return this && this->interface.id == get_id() ? this : NULL;
}

/**
 * @brief Chekc if the provided node is free
 * 
 * The node is free if it is has a previous node in the list of free nodes, or,
 * if it is the head of this list.
 * 
 * @param heap Provided heap structure
 * @param node Heap memory block node
 * @return true if the node points at a free memory block
 * @return false otherwise
 */

static inline bool is_free(xlh_heap_t *heap, xlh_node_t *node) {
  return (node && (node == heap->head_free || NULL != node->free.previous));
}

//==================================================================================================
// Local functions declaration
//==================================================================================================

/**
 * @brief Node indicators for managing memory free operation.
 * 
 * It aims at speeding up the free operation.
 */

typedef struct {
  xlh_node_t *final;
  /**< Final node address after all necessary merges. */
  xlh_node_t *start;
  /**< Node adress from which to start the insertion of final node. */
  bool is_next_free;
  /**< Indicates if the next neighbor node is free if it exists. */
  bool is_previous_free;
  /**< Indicates if the previous neighbor node is free if it exists. */
  bool rm_next;
  /**< Indicates if the next neighbor node must be removed from the list of free nodes. */
  bool rm_previous;
  /**< Indicates if the previous neighbor node must be removed from the list of free nodes. */
  bool insert_final;
  /**< Indicates if the final node must be inserted back in the list of free nodes. */
} xlh_free_config_t;

static void* end(xlh_heap_t *this, size_t *count);
static xlh_node_t* create_node(xlh_heap_t *heap, void* start, size_t length);
static size_t destroy_node(xlh_heap_t *heap, xlh_node_t *node);
static xlh_node_t* find_fitting_node(xlh_heap_t *heap, size_t size);
static xlh_node_t* split_node(xlh_heap_t *heap, xlh_node_t *node, size_t size);
static void merge_node_with_next(xlh_heap_t *heap, xlh_node_t *node);
static void remove_free_node(xlh_heap_t *heap, xlh_node_t *node);
static void insert_free_node_fwd(xlh_heap_t *heap, xlh_node_t *node, xlh_node_t *start);
static void insert_free_node_rev(xlh_heap_t *heap, xlh_node_t *node, xlh_node_t *start);
static void prepare_free_node(xlh_heap_t *heap, xlh_node_t *node, xlh_free_config_t *cfg);

static xlh_node_t* get_node(xlh_heap_t *this, void *ptr);

#ifdef __DEBUG
static void dump(xlh_heap_t *heap);
#endif

static void dummy() {}

//==================================================================================================
// Interface functions
//==================================================================================================

static void* end(xlh_heap_t *this, size_t *count) {
  if (count) {
    *count = xlh_count(this);
  }
  return xlh_end(this, NULL);
}

xtc_heap_t* xlh_init(xlh_heap_t *this, void *mem, size_t length, xtc_protect_t *protect) {
  check_node_size();
  xtc_heap_t *rtn = NULL;
  if (this && mem && sizeof(xlh_node_t) < length) {
    if (!protect || (protect->lock && protect->unlock)) {
      memset(mem, 0, length);
      this->interface.id = get_id();
#ifndef __DEBUG
      this->interface.alloc = (xtc_alloc_t)xlh_alloc;
#else
      this->interface.dump = (xtc_dump_t)xlh_dump;
      this->interface.alloc_dbg = (xtc_alloc_t)xlh_alloc_dbg;
#endif
      this->interface.free = (xtc_free_t)xlh_free;
      this->interface.count = (xtc_count_t)xlh_count;
      this->interface.end = (xtc_end_t)end;
      this->mem_pool = mem;
      this->mem_length = length;
      this->node_offset = sizeof(xlh_node_t);
      this->head_blks = create_node(this, mem, length);
      this->head_free = this->head_blks;
      this->tail_free = this->head_blks;
      this->count = 0;
      this->protect.lock = (protect) ? protect->lock : dummy;
      this->protect.unlock = (protect) ? protect->unlock : dummy;
      rtn = &this->interface;
    }
  }
  return rtn;
}

void* xlh_end(xlh_heap_t *this, xlh_stats_t *stats) {
  void *rtn = NULL;
  xlh_stats_t tmp, *s = (stats) ? stats : &tmp;  
  xlh_allocated_stats(this, s);
  xlh_heap_t *heap = check(this);
  if (heap) {
    xtc_protect_t protect = heap->protect;
    protect.lock();
#ifdef __DEBUG
    if (s->count) {
      dump(heap);
    }
#endif
    rtn = heap->mem_pool;
    memset(heap, 0, sizeof(xlh_heap_t));
    protect.unlock();
    if (s->count) {
      fprintf(stderr, "WARNING: %ld memory leak(s) detected when ending heap!\n", s->count);
    }
  }
  return rtn;
}

#ifndef __DEBUG
void* xlh_alloc(xlh_heap_t *this, size_t size)
#else
void* xlh_alloc_dbg(xlh_heap_t *this, size_t size, const char *fn, int line)
#endif
{
  void *rtn = NULL;
  xlh_heap_t *heap = check(this);
  if (heap && size) {
    size_t size_to_alloc = XTC_ALIGNED_SIZE(size);
    heap->protect.lock();
    xlh_node_t *node = find_fitting_node(heap, size_to_alloc);
    if (node) {
      xlh_node_t *free_start = node->free.next;
      remove_free_node(heap, node);
      xlh_node_t *remain = split_node(heap, node, size_to_alloc);
      if (remain) {
        insert_free_node_fwd(heap, remain, free_start);
      }
#ifdef __DEBUG
      node->requested = size;
      node->fn = fn;
      node->line = line;
#endif
      rtn = (void*)(node + 1);
      heap->count++;
    }
    heap->protect.unlock();
  }
  return rtn;
}

void xlh_free(xlh_heap_t *this, void *ptr) {
  xlh_heap_t *heap = check(this);
  if (heap && ptr) {
    heap->protect.lock();
    xlh_node_t *node = get_node(heap, ptr);
    if (node && !is_free(heap, node)) {
      assert(heap->count > 0);
#ifdef __DEBUG
      node->requested = 0;
      node->fn = NULL;
      node->line = 0;
#endif
      xlh_free_config_t cfg;
      prepare_free_node(heap, node, &cfg);
      // Remove from free list
      if (cfg.rm_next) {
        remove_free_node(heap, node->blk.next);
      }
      if (cfg.rm_previous) {
        remove_free_node(heap, node->blk.previous);
      }
      // Merge nodes
      if (cfg.is_next_free) {
        merge_node_with_next(heap, node);
      }
      if (cfg.is_previous_free) {
        merge_node_with_next(heap, node->blk.previous);
      }
      // Insert result in free list
      if (cfg.insert_final) {
        insert_free_node_rev(heap, cfg.final, cfg.start);
      }
      heap->count--;
    }
    heap->protect.unlock();
  }
}

size_t xlh_count(xlh_heap_t *this) {
  size_t rtn = 0;
  xlh_heap_t *heap = check(this);
  if (heap) {
    heap->protect.lock();
    rtn = heap->count;
    heap->protect.unlock();
  }
  return rtn;
}

size_t xlh_max_free_blk(xlh_heap_t *this) {
  size_t rtn = 0;
  xlh_heap_t *heap = check(this);
  if (heap) {
    heap->protect.lock();
    if (heap->head_free) {
      rtn = heap->head_free->size;
    }
    heap->protect.unlock();
  }
  return rtn;
}

void xlh_free_stats(xlh_heap_t *this, xlh_stats_t *stats) {
  xlh_heap_t *heap = check(this);
  if (stats && heap) {
    memset(stats, 0, sizeof(xlh_stats_t));
    heap->protect.lock();
    for (xlh_node_t *node = heap->head_free; node != NULL; node = node->free.next) {
      stats->count++;
      stats->total_size += node->size;
    }
    if (heap->head_free) {
      stats->max_block_size = heap->head_free->size;
    }
    heap->protect.unlock();
  }
}

void xlh_allocated_stats(xlh_heap_t *this, xlh_stats_t *stats) {
  xlh_heap_t *heap = check(this);
  if (stats && heap) {
    memset(stats, 0, sizeof(xlh_stats_t));
    heap->protect.lock();
    for (xlh_node_t *node = heap->head_blks; node != NULL; node = node->blk.next) {
      if (is_free(heap, node)) continue;
      stats->count++;
      stats->total_size += node->size;
      if (stats->max_block_size < node->size) {
        stats->max_block_size = node->size;
      }
    }
    assert(stats->count == heap->count);
    heap->protect.unlock();
  }
}

#ifdef __DEBUG

void xlh_dump(xlh_heap_t *this) {
  xlh_heap_t *heap = check(this);
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
 * @brief Create a node object.
 * 
 * @param heap Heap object to which the memory block belongs.
 * @param start Start address of the memory block.
 * @param length Content length of the node to create.
 * @return
 * This function returns the newly created and initialized node.
 */

static xlh_node_t* create_node(xlh_heap_t *heap, void* start, size_t length) {
  memset(start, 0, sizeof(xlh_node_t));
  xlh_node_t *rtn = (xlh_node_t*)start;
  rtn->heap = (uintptr_t)heap;
  rtn->size = length - sizeof(xlh_node_t);
  return rtn;
}

/**
 * @brief Destroy a node object.
 * 
 * @param heap Valid heap object to which the node belongs.
 * @param node Node address.
 * @return
 * This function returns the size of the freed space related to the provided node.
 */

static size_t destroy_node(xlh_heap_t *heap, xlh_node_t *node) {
  (void)heap;
  size_t length = node->size + sizeof(xlh_node_t);
  memset(node, 0, sizeof(xlh_node_t));
  return length;
}

/**
 * @brief Find a free node fitting the requested size.
 * 
 * @param heap Valid heap object.
 * @param size Requested size.
 * @return
 * This function returns the address of the smallest free node fitting the requested size
 * or @c NULL if none was found.
 */

static xlh_node_t* find_fitting_node(xlh_heap_t *heap, size_t size) {
  xlh_node_t *node = NULL;
  for (node = heap->tail_free; NULL != node; node = node->free.previous) {
    if (size <= node->size) break;
  }
  return node;
}

/**
 * @brief Split a free node if necessary with regards to a requested size
 * 
 * The node is split if the remaining size of the provided node is able to hold another node object.
 * On the opposite, no new node is created.
 * 
 * @param heap Valid heap object.
 * @param node Free node that may be split.
 * @param size Requested size.
 * @return 
 * This function returns the address of a new node related to the remaining size of the original node
 * or @c NULL if the remaining size cannot hold a new node.
 */

static xlh_node_t* split_node(xlh_heap_t *heap, xlh_node_t *node, size_t size) {
  assert(size == XTC_ALIGNED_SIZE(size));
  xlh_node_t *rtn = NULL;
  size_t remain = node->size - size;
  if (remain > sizeof(xlh_node_t)) {
    node->size = size;
    rtn = create_node(heap, (char*)(node + 1) + size, remain);
    rtn->blk.next = node->blk.next;
    rtn->blk.previous = node;
    if (node->blk.next) {
      node->blk.next->blk.previous = rtn;
    }
    node->blk.next = rtn;
  }
  return rtn;
}

/**
 * @brief Merge a free node with its following neighbor.
 * 
 * It is assumed the neighbor next is free as well.
 * After merge, a single node exists replacing the provided node and its following neighbor.
 * 
 * @param heap Valid heap object.
 * @param node Node address
 */

static void merge_node_with_next(xlh_heap_t *heap, xlh_node_t *node) {
  (void)heap; (void)node;
  xlh_node_t *next = node->blk.next;
  if (next) {
    node->blk.next = next->blk.next;
    if (node->blk.next) {
      node->blk.next->blk.previous = node;
    }
    node->size += destroy_node(heap, next);
  }
}

/**
 * @brief Remove a node from the list of free nodes.
 * 
 * @param heap Valid heap object.
 * @param node Node to remove
 */

static void remove_free_node(xlh_heap_t *heap, xlh_node_t *node) {
  if (heap->head_free == node) {
    heap->head_free = node->free.next;
  }
  if (heap->tail_free == node) {
    heap->tail_free = node->free.previous;
  }
  if (node->free.previous) {
    node->free.previous->free.next = node->free.next;
  }
  if (node->free.next) {
    node->free.next->free.previous = node->free.previous;
  }
  node->free.next = node->free.previous = NULL;
}

/**
 * @brief Insert (forward) a node in the list a free nodes.
 * 
 * The list is forward looked up starting from the start node for the right insertion point
 * so that the list remain reverse ordered by block size.
 * If the start node is @c NULL, the node is inserted at the tail of the list.
 * 
 * @param heap Valid heap object
 * @param node Node to insert
 * @param start Start node after which the node must be inserted.
 */

static void insert_free_node_fwd(xlh_heap_t *heap, xlh_node_t *node, xlh_node_t *start) {
  assert(!is_free(heap, node));
  xlh_node_t *tmp;
  for (tmp = start; NULL != tmp; tmp = tmp->free.next) {
    if (tmp->size <= node->size) break;
  }
  if (tmp) {
    // insert before tmp
    node->free.next = tmp;
    node->free.previous = tmp->free.previous;
    if (tmp->free.previous) {
      tmp->free.previous->free.next = node;
    }
    tmp->free.previous = node;
    if (heap->head_free == tmp) {
      heap->head_free = node;
    }
  } else {
    // insert at tail
    node->free.previous = heap->tail_free;
    node->free.next = NULL;
    if (heap->tail_free) {
      heap->tail_free->free.next = node;
      heap->tail_free = node;
    } else {
      assert(NULL == heap->head_free);
      heap->tail_free = heap->head_free = node;
    }
  }
}

/**
 * @brief Insert (reverse) a node in the list a free nodes.
 * 
 * The list is reverse looked up starting from the start node for the right insertion point
 * so that the list remain reverse ordered by block size.
 * If the start node is @c NULL, the node is inserted at the head of the list.
 * 
 * @param heap Valid heap object
 * @param node Node to insert
 * @param start Start node after which the node must be inserted.
 */

static void insert_free_node_rev(xlh_heap_t *heap, xlh_node_t *node, xlh_node_t *start) {
  xlh_node_t *tmp;
  for (tmp = start; NULL != tmp; tmp = tmp->free.previous) {
    if (tmp->size >= node->size) break;
  }
  if (is_free(heap, node)) {
    // If node is already free, it may already be placed correctly
    if (!tmp || tmp == node || tmp->free.next == node) {
      assert(tmp || node == heap->head_free);
      return;
    } else {
      // Must be removed before reinserted
      remove_free_node(heap, node);
    }
  }
  if (tmp) {
    // insert after tmp
    node->free.next = tmp->free.next;
    node->free.previous = tmp;
    if (tmp->free.next) {
      tmp->free.next->free.previous = node;
    }
    tmp->free.next = node;
    if (heap->tail_free == tmp) {
      heap->tail_free = node;
    }
  } else {
    // insert at head
    node->free.next = heap->head_free;
    node->free.previous = NULL;
    if (heap->head_free) {
      heap->head_free->free.previous = node;
      heap->head_free = node;
    } else {
      assert(NULL == heap->tail_free);
      heap->tail_free = heap->head_free = node;
    }
  }
}

/**
 * @brief Compute node free operation indicators.
 * 
 * @param heap Valid heap node
 * @param node Node to free
 * @param cfg Indicator structure
 */

static void prepare_free_node(xlh_heap_t *heap, xlh_node_t *node, xlh_free_config_t *cfg) {
  xlh_node_t *P = node->blk.previous, *N = node->blk.next;  
  bool is_next_free = is_free(heap, N),
        is_previous_free = is_free(heap, P);
  // Compute flags
  bool P_gt_N = (is_next_free && is_previous_free && (P->size > N->size));
  size_t size_final = node->size;
  size_final += (is_previous_free) ? P->size + sizeof(xlh_node_t) : 0;
  size_final += (is_next_free) ? N->size + sizeof(xlh_node_t) : 0;
  bool final_gt_Pp = (is_previous_free && (!P->free.previous || (size_final > P->free.previous->size)));

  // Assign config
  cfg->is_next_free = is_next_free;
  cfg->is_previous_free = is_previous_free;
  cfg->final = (is_previous_free) ? P : node;
  cfg->rm_next = is_next_free;
  cfg->rm_previous = final_gt_Pp || (is_next_free && is_previous_free && !P_gt_N);
  cfg->insert_final = (!is_previous_free || final_gt_Pp || !P_gt_N);
  cfg->start = NULL;
  if (!is_previous_free && !is_next_free) {
    cfg->start = heap->tail_free;
  } else if (is_next_free && !P_gt_N) {
    cfg->start = N->free.previous;
  } else {  // ((P_gt_N && final_gt_Pp) || (is_previous_free && !is_next_free))
    cfg->start = P->free.previous;
  }
  if (cfg->start == cfg->final) {
    cfg->start = cfg->final->free.previous;
  }
}

/**
 * @brief Get the local heap node object related to the provided pointer.
 * 
 * Local heap validity must have been checked by caller.
 * The provided pointer @c ptr is valid if the address content is located in the range of the
 * initial configured memory pool and if the identifier is the same as the heap.
 * 
 * @param heap The heap to which the provided pointer ptr belongs.
 * @param ptr The allocated local heap node.
 * @return
 * The function returns a valid @c xlh_node_t* related to the provided pointer @c ptr
 * or @c NULL if the provided pointer does not belong to the provided heap.
 */

static xlh_node_t* get_node(xlh_heap_t *heap, void *ptr) {
  xlh_node_t *rtn = NULL;
  intptr_t delta = (intptr_t)ptr - (intptr_t)heap->mem_pool;
  // Must be in the pool range
  if (delta > 0 && delta < (intptr_t)heap->mem_length) {
    xlh_node_t *tmp = (xlh_node_t*)ptr - 1;
    if (tmp->heap == (uintptr_t)heap) {
      rtn = tmp;
    }
  }
  return rtn;
}

#ifdef __DEBUG

/**
 * @brief Dump the heap allocated block information.
 * 
 * @param heap Local heap
 */

static void dump(xlh_heap_t *heap) {
  xlh_node_t *node;
  printf("HEAP %p [%ld], Blks %p[%ld], Frees > %p %p <\n",
    heap->mem_pool, heap->mem_length,
    (void*)heap->head_blks, heap->count,
    (void*)heap->head_free, (void*)heap->tail_free);
  for (node = heap->head_blks; NULL != node; node = node->blk.next) {
    if (!is_free(heap, node)) {
      printf("\t%p[%ld?>%ld] @ %s (%d)\n",
        (void*)(node + 1), node->requested, node->size, node->fn, node->line);
    }
  }
}

/*
  Not exported / undocumented.
  Dump all heap structure and lists.
*/
void xlh_dump_heap(xlh_heap_t *this) {
  xlh_heap_t *heap = check(this);
  size_t i; xlh_node_t *node;
  assert(heap);
  printf("############################################################\n");
  printf("\tHEAP : pool %p size %08lx (%ld)\n", heap->mem_pool, heap->mem_length, heap->mem_length);
  printf("\t  HEAP BLOCKS : blk  > %p (%ld)\n", (void*)heap->head_blks, heap->count);
  printf("\t  HEAP FREE   : free > %p %p <\n", (void*)heap->head_free, (void*)heap->tail_free);
  printf("------------------------------------------------------------\n");
  for (i = 0, node = heap->head_blks; NULL != node && i < 100; node = node->blk.next, i++) {
    printf("\t\tNODE[%ld] : @ %p # %08lx (%ld) @BP %p @BN %p @FP %p @FN %p\n",
      i, (void*)node, node->size, node->size,
      (void*)node->blk.previous, (void*)node->blk.next,
      (void*)node->free.previous, (void*)node->free.next);
  }
}

#endif
