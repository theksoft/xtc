#ifndef _XTC_STRHP_H_
#define _XTC_STRHP_H_

#include "xtc_gnrhp.h"
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// Structure must be aligned on integer boundary
#define XSH_NODE_SIZE(name)           (XTC_ALIGNED_STRUCT(name) + sizeof(xsh_node_t))
#define XSH_HEAP_LENGTH(name, count)  ((count) * XSH_NODE_SIZE(name))

/**
 * @brief Structure heap (XSH) node used for managing list of free memory blocks.
 * 
 * The list of free memory blocks allows to improve allocation and free performance.
 */
typedef struct xsh_node_s {
  struct xsh_node_s *next;  /**< Next node in the list. */
  int allocated;            /**< Indicates the block is allocated. */
#ifdef __DEBUG
  const char *fn;
  int line;
#endif
} xsh_node_t;

/**
 * @brief Structure heap (XSH)
 * 
 * It is exported in order to allow the allocation of the heap management structure on the stack.
 * 
 * The @c interface allows a generic management of the heap whathever its effective type is.
 * 
 * The @c protect interface must be set in case the structure heap is accessed in concurrent accesses.
 * 
 * The fields @c mem_pool and @c mem_length defines the memory area in which structure allocation will be managed.
 * This memory area can be statically declared or dynamically allocated during an initial allocation phase for example.
 * 
 * The @c size field is the size of the structure to manage.
 * 
 * The @c node_size and @c node_offset are precomputed to fasten node to memory area back and forth translations.
 * 
 * The @c lfree refers the head of list of available memory blocks for allocation.
 * 
 * The @c count and @c free_count of memory is computed within each allocation and free calls
 * in order to avoid an performance-expensive loop of the memory block lists to compute it.
 * 
 * @see xtc_heap_t
 */
typedef struct {
  xtc_heap_t interface;           /**< Generic heap interface. */
  xtc_protect_t protect;          /**< Protection interface. */
  void *mem_pool;                 /**< Initially allocated pool. */
  size_t mem_length;              /**< Length of initially allocated pool. */
  size_t size;                    /**< Size of managed structure. */
  size_t node_size, node_offset;  /**< Node size and offset. */
  xsh_node_t *lfree;              /**< List of free nodes. */
  size_t count;                   /**< Count of allocated blocks. */
  size_t free_count;              /**< Count of free blocks. */
} xsh_heap_t;

/**
 * @brief Initialize a structure heap object.
 * 
 * @param heap Allocated but uninitialized structure heap object.
 * @param mem Base address of the memory pool to use for structure allocation.
 * @param length Length of the memory pool to use for structure allocation.
 * @param size Size of the related structure to be allocated with this heap.
 * @param protect Protection functions for thread safety.
 * @return
 * This function returns a generic XTC heap interface address @c xtc_heap_t*
 * related to the initialized structure heap.
 * Allocation and freeing can be performed using directly @c xsh_alloc() and @c xsh_free()
 * or by calling @c alloc() and @c free() fields of this returned interface.
 * 
 * @see xsh_end()
 */

xtc_heap_t* xsh_init(xsh_heap_t *heap, void *mem, size_t length, size_t size, xtc_protect_t *protect);

/**
 * @brief End a structure heap object.
 * 
 * The function checks the structure heap content, reset the structure heap content.
 * 
 * @param heap Initialized structure heap
 * @param count Filled with the number of remaining allocated blocks. Can be @c NULL.
 * @return 
 * This function returns the memory pool originally used to initialize the provided
 * structure heap with xsh_init(). It returns @c NULL on error.
 * 
 * @see xsh_init()
 */

void* xsh_end(xsh_heap_t *heap, size_t *count);

/**
 * @brief Allocate memory on the provided structure heap.
 * 
 * The function performs the following checks:
 * - The provided heap @c this must be a structure heap.
 * - The provided size to allocate @c size must be the configured structure size.
 * 
 * The function uses the front node of the list of free structure heap node
 * for allocation.
 * 
 * @param heap Initialized structure heap
 * @param size Size to allocate
 * @param fn [__DEBUG] File name where the call to xsh_alloc() takes place
 * @param line [__DEBUG] File line where the call to wsh_alloc() takes place.
 * @return
 * The function returns the memory address related to a free structure heap node
 * as a @c void* pointer or @c NULL if there is no more free space in the structure heap.
 * The returned memory block pointer must be freed with xsh_free() when the memory area is no more used.
 * 
 * @see xsh_init() and xsh_free()
 */

#ifndef __DEBUG

void* xsh_alloc(xsh_heap_t *heap, size_t size);

#else // __DEBUG

void* xsh_alloc_dbg(xsh_heap_t *heap, size_t size, const char *fn, int line);
#define xsh_alloc(heap, size)     xsh_alloc_dbg(heap, size, __FILE__, __LINE__)

#endif  // __DEBUG

/**
 * @brief Free a previously allocated pointer related to the provided local heap.
 * 
 * The function performs the following checks:
 * - The provided heap @c this must be a structure heap.
 * - The provided pointer @c ptr must be a valid structure heap pointer address.
 * 
 * The structure heap node related to the allocated pointer is pushed back
 * in front of the list of structure heap free nodes.
 * 
 * @param heap Initialized structure heap.
 * @param ptr Pointer allocated on the structure heap.
 * 
 * @see xsh_init(), xsh_alloc()
 */

void xsh_free(xsh_heap_t *heap, void *ptr);

/**
 * @brief Get the count of allocated blocks.
 * 
 * @param heap Initialized structure heap
 * @return
 * The number of allocated blocks is returned.
 * 
 * @see xsh_init(), xsh_free_count()
 */

size_t xsh_count(xsh_heap_t *heap);

/**
 * @brief Get the count of free blocks.
 * 
 * @param heap Initialized structure heap
 * @return
 * The number of free blocks is returned.
 * 
 * @see xsh_init(), xsh_count()
 */

size_t xsh_free_count(xsh_heap_t *heap);

/**
 * @brief Dump the allocated blocks of a structure heap.
 * 
 * This function dumps all allocated blocks of a valid structure heap
 * with the information where allocation takes place.
 * 
 * @param heap Initialize structure heap
 * 
 * @see xsh_init()
 */

#ifdef __DEBUG
void xsh_dump(xsh_heap_t *heap);
#endif  // __DEBUG

#ifdef __cplusplus
}
#endif

#endif // _XTC_STRHP_H_
