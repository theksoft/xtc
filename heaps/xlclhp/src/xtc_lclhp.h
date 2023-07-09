#ifndef _XTC_LCLHP_H_
#define _XTC_LCLHP_H_

#include "xtc_gnrhp.h"
#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Forward declaration of a pointer on XLH node.
 * @see xlh_node_t
 */
typedef struct xlh_node_s *xlh_node_ptr_t;

/**
 * @brief Double-link chained list declaration for all block and free block lists.
 * @see xlh_node_t
 */
typedef struct {
  xlh_node_ptr_t  next, previous; /**< Next and previous nodes */
} xlh_node_ptrs_t;

/**
 * @brief Local heap (XLH) node used for managing local heap memory blocks.
 * 
 * The list of free memory blocks @c free defines the allocation possibility.
 * If these pointers are not NULL or if it is the head of the list, the block is allocated.
 * 
 * The list of blocks @c blk allows managing local heap fragmentation and defragmentation.
 */
typedef struct xlh_node_s {
  uintptr_t heap;                 /**< Address of the associated heap. */
  size_t size;                    /**< Node available/allocated (integer-aligned) size. */
  xlh_node_ptrs_t blk;            /**< Next and previous node in the list of memory blocks (ordered by increasing address). */
  xlh_node_ptrs_t free;           /**< Next and previous node in the list of free blocks (ordered by decreasing size). */
#ifdef __DEBUG
  size_t requested;
  const char *fn;
  int line;
#endif
} xlh_node_t;

/**
 * @brief Local heap (XLH)
 * 
 * It is exported in order to allow the allocation of the heap management structure on the stack.
 * 
 * The @c interface allows a generic management of the heap whathever its effective type is.
 * 
 * The @c protect interface must be set in case the local heap is accessed in concurrent accesses.
 * 
 * The fields @c mem_pool and @c mem_length defines the memory area in which local allocation will be managed.
 * This memory area can be statically declared or dynamically allocated during an initial allocation phase for example.
 * 
 * The @c head_blks pointer refers the head of the list of fragmented memory blocks managed within the local heap.
 * This list increases when blocks are splitted during allocation phase and decreases when a freed block is merged
 * with its free neighboring blocks providing there are also free.
 * 
 * The @c head_free and @c tail_free refers the head and tail of list of available memory blocks for allocation.
 * The blocks are ordered by decreasing size.
 * 
 * During allocation, the allocation policy is to find the smallest free block that can fit the requested size.
 * Note the the actual allocated size is integer-aligned to ensure the allocated memory can hold a structure
 * which must be aligned on integer boundary as defined by C language.
 * 
 * The @c count of memory is computed within each allocation and free calls
 * in order to avoid an performance-expensive loop or the memory block lists to compute it.
 * 
 * @see xtc_heap_t
 */
typedef struct {
  xtc_heap_t interface;           /**< Generic heap interface. */
  xtc_protect_t protect;          /**< Protection interface. */
  void *mem_pool;                 /**< Initially allocated pool. */
  size_t mem_length;              /**< Length of initially allocated pool. */
  size_t node_offset;             /**< Node offset. */
  xlh_node_t *head_blks;          /**< Head of the list of memory blocks. */
  xlh_node_t *head_free;          /**< Head of the list of free nodes. */
  xlh_node_t *tail_free;          /**< Tail of the list of free nodes. */
  size_t count;                   /**< Count of allocated blocks. */
} xlh_heap_t;

/**
 * @brief Structure definition holding either allocated memory block stats or free memory block stats.
 * 
 * @see xlh_allocated_stats(), xlh_free_stats()
 */
typedef struct {
  size_t  count;                  /**< Count of memory blocks. */
  size_t  total_size;             /**< Total actual size. */
  size_t  max_block_size;         /**< Actual size of the largest block. */
} xlh_stats_t;

/**
 * @brief Initializes a local heap object.
 * 
 * @param heap Allocated but uninitialized local heap object
 * @param mem Base address of the memory pool to use for local allocation.
 * @param length Length of the memory pool to use for local allocation.
 * @param protect Protection functions for thread safety.
 * @return
 * This function returns a generic XTC heap interface address @c xtc_heap_t*
 * related to the initialized local heap.
 * Allocation and freeing can be performed using directly @c xsh_alloc() and @c xsh_free()
 * or by calling @c alloc() and @c free() fields of this returned interface.
 * 
 * @see xlh_end()
 */

xtc_heap_t* xlh_init(xlh_heap_t *heap, void *mem, size_t length, xtc_protect_t *protect);

/**
 * @brief Ends a local heap object.
 * 
 * The function checks the local heap content, reset the local heap content.
 * 
 * @param heap Initialized local heap
 * @param stats Filled with the allocated blocks stats before ending. Can be @c NULL.
 * @return void* 
 * This function returns the memory pool originally used to initialize the provided
 * local heap with xlh_init(). It returns @c NULL on error.
 * 
 * @see xlh_init()
 */

void* xlh_end(xlh_heap_t *heap, xlh_stats_t *stats);

/**
 * @brief Allocate memory on the provided local heap.
 * 
 * The function performs the following checks:
 * - The provided heap @c this must be a valid local heap previously initialized with xlh_init().
 * 
 * The function uses the smallest node of the list of free memory block node
 * that can fit the integer-aligned size requested.
 * 
 * @param heap Initialized local heap
 * @param size Size to allocate
 * @param fn [__DEBUG] File name where the call to xlh_alloc() takes place
 * @param line [__DEBUG] File line where the call to xlh_alloc() takes place.
 * @return
 * The function returns the memory address related to a free local heap node as a @c void* pointer
 * or a @c NULL pointer if there is no more free space in the local heap.
 * The returned memory block pointer must be freed with xlh_free() when the memory area is no more used.
 * 
 * @see xlh_init(), xlh_free()
 */

#ifndef __DEBUG

void* xlh_alloc(xlh_heap_t *heap, size_t size);

#else // __DEBUG

void* xlh_alloc_dbg(xlh_heap_t *heap, size_t size, const char *fn, int line);
#define xlh_alloc(heap, size)     xlh_alloc_dbg(heap, size, __FILE__, __LINE__)

#endif  // __DEBUG

/**
 * @brief Free a previously allocated pointer related to the provided local heap.
 * 
 * The function performs the following checks:
 * - The provided heap @c this must be a local heap.
 * - The provided pointer @c ptr must be a valid local heap pointer allocated address.
 * 
 * The local heap node related to the allocated pointer is inserted back
 * in the list of free memory nodes and possibly merged with neighbors freed nodes if necessary.
 * 
 * @param heap Initialized local heap.
 * @param ptr Pointer allocated on the local heap.
 * 
 * @see xlh_init(), xlh_alloc()
 */

void xlh_free(xlh_heap_t *heap, void *ptr);

/**
 * @brief Get the count of allocated blocks.
 * 
 * @param heap Initialized local heap
 * @return
 * The number of allocated blocks is returned.
 * 
 * @see xlh_init(), xlh_allocated_stats()
 */

size_t xlh_count(xlh_heap_t *heap);

/**
 * @brief Get the largest free memory block available to allocation.
 * 
 * @param heap Initialized local heap
 * @return
 * The size of the largest free memory block that can be allocated is returned.
 * 
 * @see xlh_init(), xlh_free_stats()
 */

size_t xlh_max_free_blk(xlh_heap_t *heap);

/**
 * @brief Get the free memory block stats.
 * 
 * Refer to the xlh_stats_t structure definition for stats content description.
 * 
 * @param heap Initialized local heap
 * @param stats Structure receiving computed free stats
 * 
 * @see xlh_init(), xlh_max_free_blk()
 */

void xlh_free_stats(xlh_heap_t *heap, xlh_stats_t *stats);

/**
 * @brief Get the allocated memory block stats.
 * 
 * Refer to the xlh_stats_t structure definition for stats content description.
 * 
 * @param heap Initialized local heap
 * @param stats Structure receiving computed allocated stats
 * 
 * @see xlh_init(), xlh_count()
 */

void xlh_allocated_stats(xlh_heap_t *heap, xlh_stats_t *stats);

/**
 * @brief Dump the allocated blocks of a local heap.
 * 
 * This function dumps all allocated blocks of a valid local heap
 * with the information where allocation takes place.
 * 
 * @param heap Initialized local heap
 * 
 * @see xlh_init()
 */

#ifdef __DEBUG
void xlh_dump(xlh_heap_t *heap);
#endif  // __DEBUG

#ifdef __cplusplus
}
#endif

#endif
