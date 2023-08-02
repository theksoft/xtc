#ifndef _XTC_LCLHP_HXX_
#define _XTC_LCLHP_HXX_

#include "xtc_lclhp.h"
#include "xtc_gnrhp.hxx"
#include <cstring>

namespace xtc {
  namespace xlh {

  /**
   * @brief Allocator template for local heap
   * 
   * This template wraps a C local heap @c xlh_heap_t.
   * The underlying memory pool has the same allocation class than the Allocator itself
   * e.g. it can be allocated statically, on the heap or on the stack.
   * The latter has a limited interest but is still possible.
   * 
   * @tparam Length Length in bytes of the mmeory area dedicated to the local heap.
   */
  template <size_t Length>
  class Allocator : public xtc::Allocator {
  protected:
    char _memory[Length];   /**< Memory pool */
    xlh_heap_t _heap;       /**< C local heap wrapped in the allocator definition */

  public:

    /**
     * @brief Construct a new Allocator object
     * 
     * The constructor fills in the memory pool with 0
     * and initializes the wrapped C local heap.
     * 
     * @param protect Optional protection interface
     * 
     * @see xlh_init()
     */
    Allocator(xtc_protect_t *protect = NULL) {
      memset(_memory, 0, Length);
      xlh_init(&_heap, _memory, Length, protect);
    }

    /**
     * @brief Destroy the Allocator object
     * 
     * The destructor fills in the memory pool with 0
     * and wraps C local heap end method (reset).
     * 
     * @see xlh_end()
     */
    virtual ~Allocator() {
      xlh_end(&_heap, NULL);
      memset(_memory, 0, Length);
    }

    /**
     * @brief Allocate a memory block fitting the requested size.
     * 
     * This method is compatible with a class new operator
     * e.g taking a @c size as parameter and returning and @c void* result.
     * 
     * This method directly wraps the C local heap allocation function.
     * 
     * @param size Size to allocate.
     * @return
     * This method returns the memory address of an allocated memory block as a
     * generic @c void*. It retusn @c NULL if there is no more free memory space.
     * 
     * @see xlh_alloc()
     */
    virtual void* allocate(size_t size) {
      return xlh_alloc(&_heap, size);
    }

    /**
     * @brief Free a memory block previously allocated.
     * 
     * This method directly wraps the C local heap free function.
     * 
     * @param ptr Pointer to a memory block to be freed.
     *            The block must have been previously allocated with the Allocator.
     * 
     * @see xlh_free()
     */
    virtual void free(void *ptr) {
      xlh_free(&_heap, ptr);
    }

    /**
     * @brief Get the count of allocated blocks.
     * 
     * This method directly wraps to the C local heap count function.
     * 
     * @return
     * This method returns the count of allocated memory blocks as @c size_t.
     * 
     * @see xlh_count()
     */
    virtual size_t count() {
      return xlh_count(&_heap);
    }

    /**
     * @brief Get the largest free memory block available to allocation.
     * 
     * This method directly wraps to the C local heap max free block size function.
     * 
     * @return
     * This method returns the size of the largest free memory block as @c size_t.
     * 
     * @see xlh_max_free_blk()
     */
    size_t maxFreeBlk() {
      return xlh_max_free_blk(&_heap);
    }

    /**
     * @brief Get the free memory block stats.
     * 
     * This method directly wraps to the C local heap free block stats function.
     * 
     * @param stats Structure receiving computed free stats
     * 
     * @see xlh_free_stats()
     */
    void freeStats(xlh_stats_t& stats) {
      xlh_free_stats(&_heap, &stats);
    }

    /**
     * @brief Get the allocated memory block stats.
     * 
     * This method directly wraps to the C local heap allocated block stats function.
     * 
     * @param stats Structure receiving computed allocated stats
     * 
     * @see xlh_allocated_stats()
     */
    void allocatedStats(xlh_stats_t& stats) {
      xlh_allocated_stats(&_heap, &stats);
    }

  };

}}

#endif
