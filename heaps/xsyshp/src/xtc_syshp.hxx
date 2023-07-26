#ifndef _XTC_SYSHP_HXX_
#define _XTC_SYSHP_HXX_

#include "xtc_syshp.h"
#include "xtc_gnrhp.hxx"

namespace xtc {
  namespace xss {

  /**
   * @brief Allocator for system heap
   * 
   * This class wraps a C system heap @c xss_heap_t.
   * It has little interest apart providing the same interface as other allocator.
   * => new & delete operator are already operating on hte system heap unless redirected.
   */
  class Allocator : public xtc::Allocator {
  protected:
    xss_heap_t _heap;       /**< C system heap wrapped in the allocator definition */

  public:

    /**
     * @brief Construct a new Allocator object
     * 
     * The constructor initializes the wrapped C system heap.
     * 
     * @param protect Optional protection interface
     * 
     * @see xss_init()
     */
    Allocator(xtc_protect_t *protect = NULL) {
      xss_init(&_heap, protect);
    }

    /**
     * @brief Destroy the Allocator object
     * 
     * The destructor wraps the C system heap end method.
     * 
     * @see xss_end()
     */
    virtual ~Allocator() {
      xss_end(&_heap, NULL);
    }

    /**
     * @brief Allocate a memory block fitting the requested size.
     * 
     * This method is compatible with a class new operator
     * e.g taking a @c size as parameter and returning and @c void* result.
     * 
     * This method directly wraps the C system heap allocation function.
     * 
     * @param size Size to allocate.
     * @return
     * This method returns the memory address of an allocated memory block as a
     * generic @c void*. It retusn @c NULL if there is no more free memory space.
     * 
     * @see xss_alloc()
     */
    virtual void* allocate(size_t size) {
      return xss_alloc(&_heap, size);
    }

    /**
     * @brief Free a memory block previously allocated.
     * 
     * This method directly wraps the C system heap free function.
     * 
     * @param ptr Pointer to a memory block to be freed.
     *            The block must have been previously allocated with the Allocator.
     * 
     * @see xss_free()
     */
    virtual void free(void *ptr) {
      xss_free(&_heap, ptr);
    }

    /**
     * @brief Get the count of allocated blocks.
     * 
     * This method directly wraps to the C system heap count function.
     * 
     * @return
     * This method returns the count of allocated memory blocks as @c size_t.
     * 
     * @see xss_count()
     */
    virtual size_t count() {
      return xss_count(&_heap);
    }

    /**
     * @brief Get the total allocated memory size with this allocator.
     * 
     * This method directly wraps to the C local heap total size function.
     * 
     * @return
     * This method returns the total size of allocated memory as @c size_t.
     * 
     * @see xss_total_size()
     */
    size_t totalSize() {
      return xss_total_size(&_heap);
    }

  };

}}

#endif
