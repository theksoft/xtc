#ifndef _XTC_SYSHP_H_
#define _XTC_SYSHP_H_

#include "xtc_gnrhp.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief System heap (XSS) node used for managing system heap memory block information.
 * 
 * The list of allocated blocks @c next and @c previous allows managing system heap tracking.
 * This can be achieved with more information through standard memory tracking tool.
 */
typedef struct xss_node_s {
  uintptr_t heap;                     /**< Address of the associated heap. */
  size_t size;                        /**< Node requested size. */
#ifdef __DEBUG
  struct xss_node_s *next, *previous; /**< [__DEBUG] Next and previous node in the list of allocated memory blocks. */
  const char *fn;
  int line;
#endif
} xss_node_t;

/**
 * @brief System heap (XSS)
 * 
 * It is exported in order to allow the allocation of the heap management structure on the stack.
 * 
 * The @c interface allows a generic management of the heap whathever its effective type is.
 * 
 * The @c protect interface must be set in case the system heap is accessed in concurrent accesses.
 * 
 * The @c count and @c total_size of memory are computed within each allocation and free calls.
 * 
 * The @c head pointer refers the head of the list of allocated blocks managed within the system heap.
 * 
 * @see xtc_heap_t
 */
typedef struct {
  xtc_heap_t interface;           /**< Generic heap interface. */
  xtc_protect_t protect;          /**< Protection interface. */
  size_t count;                   /**< Count of allocated blocks. */
  size_t total_size;              /**< Current total allocated size. */
#ifdef __DEBUG
  xss_node_t *head;               /**< [__DEBUG] Head of the list of memory blocks. */
#endif
} xss_heap_t;

/**
 * @brief Initializes a system heap object.
 * 
 * @param heap Allocated but uninitialized system heap object
 * @param protect Protection functions for thread safety.
 * @return
 * This function returns a generic XTC heap interface address @c xtc_heap_t*
 * related to the initialized system heap.
 * Allocation and freeing can be performed using directly @c xss_alloc() and @c xss_free()
 * or by calling @c alloc() and @c free() fields of this returned interface.
 * 
 * @see xss_end()
 */

xtc_heap_t* xss_init(xss_heap_t *heap, xtc_protect_t *protect);

/**
 * @brief Ends a system heap object.
 * 
 * The function checks the system heap content, reset the system heap content.
 * 
 * @param heap Initialized system heap
 * @param count Filled with the number of remaining allocated blocks. Can be @c NULL.
 * @return
 * This function returns @c NULL.
 * 
 * @see xss_init()
 */

void* xss_end(xss_heap_t *heap, size_t *count);

/**
 * @brief Allocate memory on the provided system heap.
 * 
 * The function performs the following checks:
 * - The provided heap @c this must be a valid system heap previously initialized with xss_init().
 * 
 * The function wraps on malloc() system call but allocate a node information block.
 * 
 * @param heap Initialized system heap
 * @param size Size to allocate
 * @param fn [__DEBUG] File name where the call to xss_alloc() takes place
 * @param line [__DEBUG] File line where the call to xss_alloc() takes place.
 * @return
 * The function returns the memory address of an allocated memory block as a @c void* pointer
 * or a @c NULL pointer if the system heap experienced an error.
 * The returned memory block pointer must be freed with xss_free() when the memory area is no more used.
 * It cannot be freed directly with system free().
 * 
 * @see xss_init(), xss_free()
 */

#ifndef __DEBUG

void* xss_alloc(xss_heap_t *heap, size_t size);

#else // __DEBUG

void* xss_alloc_dbg(xss_heap_t *heap, size_t size, const char *fn, int line);
#define xss_alloc(heap, size)     xss_alloc_dbg(heap, size, __FILE__, __LINE__)

#endif  // __DEBUG

/**
 * @brief Free a previously allocated pointer related to the provided system heap.
 * 
 * The function performs the following checks:
 * - The provided heap @c this must be a system heap.
 * - The provided pointer @c ptr must be a valid system heap pointer allocated address.
 * 
 * The function wraps on free() system call but with the allocated information block.
 * 
 * @param heap Initialized system heap.
 * @param ptr Pointer allocated on the system heap.
 * 
 * @see xss_init(), xss_alloc()
 */

void xss_free(xss_heap_t *heap, void *ptr);

/**
 * @brief Get the count of allocated blocks.
 * 
 * @param heap Initialized system heap
 * @return
 * The number of allocated blocks is returned.
 * 
 * @see xss_init(), xss_total_size()
 */

size_t xss_count(xss_heap_t *heap);

/**
 * @brief Get the total size of allocated memory with this allocator.
 * 
 * @param heap Initialized system heap
 * @return
 * The total size of allocated memory is returned.
 * 
 * @see xss_init(), xss_total_size()
 */

size_t xss_total_size(xss_heap_t *heap);

/**
 * @brief Dump the allocated blocks of a system heap.
 * 
 * This function dumps all allocated blocks of a valid system heap
 * with the information where allocation takes place.
 * 
 * @param heap Initialized system heap
 * 
 * @see xss_init()
 */

#ifdef __DEBUG
void xss_dump(xss_heap_t *heap);
#endif  // __DEBUG

#ifdef __cplusplus
}
#endif

#endif // _XTC_SYSHP_H_
