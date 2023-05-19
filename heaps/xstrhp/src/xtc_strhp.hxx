#ifndef _XTC_STRHP_HXX_
#define _XTC_STRHP_HXX_

#include "xtc_strhp.h"
#include "xtc_gnrhp.hxx"
#include <cstring>

namespace xtc {
  namespace xsh {

  /**
   * @brief Allocator template for class or structure
   * 
   * This template wraps a C structure heap @c xsh_heap_t.
   * The underlying memory pool has the same allocation class than the Allocator itself
   * e.g. it can be allocated statically, on the heap or on the stack.
   * The latter has a limited interest but is still possible.
   * 
   * @tparam T Class or structure associated to the allocator
   * @tparam Count Reserve count of memory blocks fitting the associated class or structure.
   */
  template<class T, int Count>
  class Allocator : public xtc::Allocator {
  protected:
    char _memory[XSH_HEAP_LENGTH(T, Count)];  /**< Memory pool for class or structure allocation */
    xsh_heap_t _heap;                         /**< C Structure heap wrapped in the allocator definition */

  public:

    /**
     * @brief Construct a new Allocator object
     * 
     * The constructor fills in the memory pool with 0
     * and initializes the wrapped C structure heap.
     * 
     * @param protect Optional protection interface
     */
    Allocator(xtc_protect_t *protect = NULL) {
      memset(_memory, 0, XSH_HEAP_LENGTH(T, Count));
      xsh_init(&_heap, _memory, XSH_HEAP_LENGTH(T, Count), sizeof(T), protect);
    }

    /**
     * @brief Destroy the Allocator object
     * 
     * The destructor fills in the memory pool with 0
     * as well as the wrapped C structure heap (reset).
     */
    virtual ~Allocator() {
      memset(_memory, 0, XSH_HEAP_LENGTH(T, Count));
      memset(&_heap, 0, sizeof(xsh_heap_t));
    }

    /**
     * @brief Allocate a memory block fitting the associated class or structure.
     * 
     * This method is compatible with a class new operator
     * e.g taking a @c size as parameter and returning and @c void* result.
     * The @c size parameter must be the size of class to allocate.
     * 
     * This method directly wraps the C structure heap allocation function.
     * 
     * @param size Size to allocate -- class or structure size.
     * @return
     * This method returns the memory address of an allocated structure as a
     * generic @c void*. It retusn @c NULL if there is no more free memory space.
     */
    virtual void* allocate(size_t size) {
      return (T*)xsh_alloc(&_heap, size);
    }

    /**
     * @brief Free a memory block previously allocated.
     * 
     * This method directly wraps the C structure heap free function.
     * 
     * @param ptr Pointer to a memory block to be freed.
     *            The block must have been previously allocated with the Allocator.
     */
    virtual void free(void *ptr) {
      xsh_free(&_heap, ptr);
    }

    /**
     * @brief Allocate a memory block fitting the associated class or structure.
     * 
     * This method is a simpler allocation method as the class or structure size
     * is already known and returns directly a pointer to the right class or structure.
     * It should not be used in a class new operator but still can be.
     * 
     * @return
     * This method returns a pointer to the associated class or structure as @c T*.
     */
    T* allocate() {
      return (T*)allocate(sizeof(T));
    }

    /**
     * @brief Free a memory block previously allocated.
     * 
     * This method takes directly a pointer to the associated class or structure.
     * 
     * @param ptr Pointer to a memory block to be freed.
     *            The block must have been previously allocated with the Allocator.
     */
    void free(T* ptr) {
      free((void*)ptr);
    }

    /**
     * @brief Get the count of remaining free memory blocks.
     * 
     * This method directly wraps to the C structure heap free count function.
     * 
     * @return
     * This method returns the count of remaining memory blocks as @c size_t.
     */
    size_t freeCount() {
      return xsh_free_count(&_heap);
    }
  };

}}

#endif
