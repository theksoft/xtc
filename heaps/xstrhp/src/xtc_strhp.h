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
 * The @c next field materializes the allocated state of the node.
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
 * @param heap Allocated structure heap object.
 * @param mem Base address of the memory pool to use for structure allocation.
 * @param length Length of the memory pool to use for structure allocation.
 * @param size Size of the related structure to be allocated with this heap.
 * @param protect Protection functions for thread safety.
 * @return
 * This function returns a generic XTC heap interface address @c xtc_heap_t*
 * related to the initialized structure heap.
 * Allocation and freeing can be performed using directly @c xsh_alloc() and @c xsh_free()
 * or by calling @c alloc() and @c free() fields of this returned interface.
 */

xtc_heap_t* xsh_init(xsh_heap_t *heap, void *mem, size_t length, size_t size, xtc_protect_t *protect);

/**
 * @brief End a structure heap object.
 * 
 * The function checks the structure heap content, reset the structure heap content.
 * 
 * @param heap Structure heap
 * @param count Filled with the number of remaining allocated blocks. Can be @c NULL.
 * @return 
 * This function returns the memory pool originally used to initialize the provided
 * structure heap with xsh_init(). It returns @c NULL on error.
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
 * @param heap Structure heap
 * @param size Size to allocate
 * @param fn [__DEBUG] File name where the call to xsh_alloc() takes place
 * @param line [__DEBUG] File line where the call to wsh_alloc() takes place.
 * @return
 * The function returns the memory address related to a free structure heap node
 * as a @c void* or @c NULL if there is no more free space in the structure heap.
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
 * @param heap Structure heap.
 * @param ptr Pointer allocated on the structure heap.
 */

void xsh_free(xsh_heap_t *heap, void *ptr);

/**
 * @brief Get the count of allocated blocks.
 * 
 * @param heap Structure heap
 * @return
 * The number of allocated blocks is returned.
 */

size_t xsh_count(xsh_heap_t *heap);

/**
 * @brief Get the count of free blocks.
 * 
 * @param heap Structure heap
 * @return
 * The number of free blocks is returned.
 */

size_t xsh_free_count(xsh_heap_t *heap);

/**
 * @brief Dump the allocated blocks of a structure heap.
 * 
 * This function dumps all allocated blocks
 * with the information where allocation takes place.
 * 
 * @param heap Structure heap
 */

#ifdef __DEBUG
void xsh_dump(xsh_heap_t *heap);
#endif  // __DEBUG

#ifdef __cplusplus
}
#endif

#endif // _XTC_STRHP_H_
