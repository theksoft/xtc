#include "xtc_lclhp.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

//==================================================================================================
// Local inline functions
//==================================================================================================

static xtc_heaps_id_t get_id() {
  static unsigned int ref = 0x0FEEDBABE;
  return (xtc_heaps_id_t)(uintptr_t)&ref;
}

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

static inline xlh_heap_t* check(xlh_heap_t *this) {
  return this && this->interface.id == get_id() ? this : NULL;
}

static inline bool is_free(xlh_heap_t *heap, xlh_node_t *node) {
  return (node && (node == heap->head_free || NULL != node->free.previous));
}

//==================================================================================================
// Local functions declaration
//==================================================================================================

typedef struct {
  xlh_node_t *final, *start;
  bool is_next_free, is_previous_free;
  bool rm_next, rm_previous, insert_final;
} xlh_free_config_t;

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

static void dummy() {}

//==================================================================================================
// Interface functions
//==================================================================================================

xtc_heap_t* xlh_init(xlh_heap_t *this, void *mem, size_t length, xtc_protect_t *protect) {
  check_node_size();
  xtc_heap_t *rtn = NULL;
  if (this && mem && sizeof(xlh_node_t) < length) {
    if (!protect || (protect->lock && protect->unlock)) {
      memset(mem, 0, length);
      this->interface.id = get_id();
      this->interface.alloc = (xtc_alloc_t)xlh_alloc;
      this->interface.free = (xtc_free_t)xlh_free;
      this->mem_pool = mem;
      this->mem_length = length;
      this->node_offset = sizeof(xlh_node_t);
      this->head_blks = create_node(this, mem, length);
      this->head_free = this->head_blks;
      this->tail_free = this->head_blks;
      this->protect.lock = (protect) ? protect->lock : dummy;
      this->protect.unlock = (protect) ? protect->unlock : dummy;
      rtn = &this->interface;
    }
  }
  return rtn;
}

void* xlh_alloc(xlh_heap_t *this, size_t size) {
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
    }
    rtn = (void*)(node + 1);
    heap->protect.unlock();
  }
  return rtn;
}

void xlh_free(xlh_heap_t *this, void *ptr) {
  xlh_heap_t *heap = check(this);
  if (heap && ptr) {
    heap->protect.lock();
    xlh_node_t *node = get_node(heap, ptr);
    if (node) {
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
      // Insert in free list
      if (cfg.insert_final) {
        insert_free_node_rev(heap, cfg.final, cfg.start);
      }
    }
    heap->protect.unlock();
  }
}

size_t xlh_max_free_blk(xlh_heap_t *this) {
  size_t rtn = 0;
  xlh_heap_t *heap = check(this);
  if (heap && heap->head_free) {
    rtn = heap->head_free->size;
  }
  return rtn;
}

void xlh_free_stats(xlh_heap_t *this, xlh_stats *stats) {
  xlh_heap_t *heap = check(this);
  if (stats && heap) {
    memset(stats, 0, sizeof(xlh_stats));
    for (xlh_node_t *node = heap->head_free; node != NULL; node = node->free.next) {
      stats->count++;
      stats->total_size += node->size;
    }
    if (heap->head_free) {
      stats->max_block_size = heap->head_free->size;
    }
  }
}

void xlh_allocated_stats(xlh_heap_t *this, xlh_stats *stats) {
  xlh_heap_t *heap = check(this);
  if (stats && heap) {
    memset(stats, 0, sizeof(xlh_stats));
    for (xlh_node_t *node = heap->head_blks; node != NULL; node = node->blk.next) {
      stats->count++;
      stats->total_size += node->size;
      if (stats->max_block_size < node->size) {
        stats->max_block_size = node->size;
      }
    }
  }
}

//==================================================================================================
// Local functions implementation
//==================================================================================================

static xlh_node_t* create_node(xlh_heap_t *heap, void* start, size_t length) {
  memset(start, 0, sizeof(xlh_node_t));
  xlh_node_t *rtn = (xlh_node_t*)start;
  rtn->heap = (uintptr_t)heap;
  rtn->size = length - sizeof(xlh_node_t);
  return rtn;
}

static size_t destroy_node(xlh_heap_t *heap, xlh_node_t *node) {
  (void)heap;
  size_t length = node->size + sizeof(xlh_node_t);
  memset(node, 0, sizeof(xlh_node_t));
  return length;
}

static xlh_node_t* find_fitting_node(xlh_heap_t *heap, size_t size) {
  xlh_node_t *node = NULL;
  for (node = heap->tail_free; NULL != node; node = node->free.previous) {
    if (size <= node->size) break;
  }
  return node;
}

static xlh_node_t* split_node(xlh_heap_t *heap, xlh_node_t *node, size_t size) {
  assert(size == XTC_ALIGNED_SIZE(size));
  xlh_node_t *rtn = NULL;
  size_t remain = node->size - size;
  if (remain > sizeof(xlh_node_t)) {
    node->size = size;
    rtn = create_node(heap, (char*)(node + 1) + size, remain);
    rtn->blk.next = node->blk.next;
    rtn->blk.previous = node;
    node->blk.next->blk.previous = rtn;
    node->blk.next = rtn;
  }
  return rtn;
}

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

static void insert_free_node_fwd(xlh_heap_t *heap, xlh_node_t *node, xlh_node_t *start) {
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

static void insert_free_node_rev(xlh_heap_t *heap, xlh_node_t *node, xlh_node_t *start) {
  xlh_node_t *tmp;
  for (tmp = start; NULL != tmp; tmp = tmp->free.previous) {
    if (tmp->size >= node->size) break;
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
  (void)heap; (void)node; (void)start;
}

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
}

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
