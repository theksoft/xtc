#ifndef _XTC_LCLHP_H_
#define _XTC_LCLHP_H_

#include "xtc_gnrhp.h"
#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Forward declaration of a pointer on XLH node
 */
typedef struct xlh_node_s *xlh_node_ptr_t;

typedef struct {
  xlh_node_ptr_t  next, previous; /**< Next and previous nodes */
} xlh_node_ptrs_t;

typedef struct xlh_node_s {
  uintptr_t heap;                 /**< Address of the associated heap. */
  size_t size;                    /**< Node available size. */
  xlh_node_ptrs_t blk;            /**< Next and previous node in the list of memory blocks (ordered by increasing address). */
  xlh_node_ptrs_t free;           /**< Next and previous node in the list of free blocks (ordered by decreasing size). */
} xlh_node_t;

typedef struct {
  xtc_heap_t interface;           /**< Generic heap interface. */
  xtc_protect_t protect;          /**< Protection interface. */
  void *mem_pool;                 /**< Initially allocated pool. */
  size_t mem_length;              /**< Length of initially allocated pool. */
  size_t node_offset;             /**< Node offset. */
  xlh_node_t *head_blks;          /**< Head of the list of memory blocks. */
  xlh_node_t *head_free;          /**< Head of the list of free nodes. */
  xlh_node_t *tail_free;          /**< Tail of the list of free nodes. */
} xlh_heap_t;

typedef struct {
  size_t  count;                  /**< Count of memory blocks. */
  size_t  total_size;             /**< Total actual size. */
  size_t  max_block_size;         /**< Actual size of the largest block. */
} xlh_stats_t;

xtc_heap_t* xlh_init(xlh_heap_t *heap, void *mem, size_t length, xtc_protect_t *protect);

void* xlh_end(xlh_heap_t *heap, xlh_stats_t *stats);

void* xlh_alloc(xlh_heap_t *heap, size_t size);

void xlh_free(xlh_heap_t *heap, void *ptr);

size_t xlh_max_free_blk(xlh_heap_t *heap);

void xlh_free_stats(xlh_heap_t *heap, xlh_stats_t *stats);

void xlh_allocated_stats(xlh_heap_t *heap, xlh_stats_t *stats);

#ifdef __cplusplus
}
#endif

#endif
